/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// standard utilities and systems includes

#include "oclBlackScholes_common.h"
#include <stdlib.h>     /* srand, rand */
#include <stdio.h>
#include <math.h>
#include "AOCL_Utils.h"
#define __ALTERA_FPGA_OPENCL__
using namespace aocl_utils;
void cleanup();

#ifdef __ALTERA_FPGA_OPENCL__
	cl_device_id device;
	cl_program program;
	cl_kernel kernel;
	cl_context context;
#endif

#define GPU_PROFILING
////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////
double executionTime(cl_event &event){
    cl_ulong start, end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);

    return (double)1.0e-9 * (end - start); // convert nanoseconds to seconds on return
}

////////////////////////////////////////////////////////////////////////////////
// Random float helper
////////////////////////////////////////////////////////////////////////////////
float randFloat(float low, float high){
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}

////////////////////////////////////////////////////////////////////////////////
// Main program
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{



    cl_mem                             //OpenCL memory buffer objects
        d_Call,
        d_Put,
        d_S,
        d_X,
        d_T;

    cl_int ciErrNum;

    float
        *h_CallCPU,
        *h_PutCPU,
        *h_CallGPU,
        *h_PutGPU,
        *h_S,
        *h_X,
        *h_T;

    const unsigned int   optionCount = 4000000;
    const float                    R = 0.02f;
    const float                    V = 0.30f;

	cl_int err;

	// Discovery platform
	//cl_platform_id platform;
#ifdef __ALTERA_FPGA_OPENCL__
	cl_platform_id platform = NULL;
	platform = findPlatform("Altera");
	if(platform == NULL) {
		printf("ERROR: Unable to find Altera OpenCL platform.\n");
		return false;
	}
	
	
#else	
	cl_platform_id platform_id[100];
	err = clGetPlatformIDs(100, platform_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Unable to get Platform ID. Error Code=%d\n", err);
		exit(1);
	}
#endif
	// Discover device
#ifdef __ALTERA_FPGA_OPENCL__
	

	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

	printf("  %s\n", getDeviceName(device).c_str());

#else		
	cl_device_id device;

	err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_CPU, 1, &device, NULL);

	if (err != CL_SUCCESS) {
		printf("Unable to get Device ID. Error Code=%d\n", err);
		exit(1);
	}
#endif

    //Get all the devices
    cl_uint uiNumDevices = 0;           // Number of devices available
    cl_uint uiTargetDevice = 0;	        // Default Device to compute on
    cl_uint uiNumComputeUnits;          // Number of compute units (SM's on NV GPU)

    
    
    // set logfile name and start logs


	h_CallCPU = (float *)malloc(optionCount * sizeof(float));
    h_PutCPU  = (float *)malloc(optionCount * sizeof(float));
    h_CallGPU = (float *)malloc(optionCount * sizeof(float));
    h_PutGPU  = (float *)malloc(optionCount * sizeof(float));
    h_S       = (float *)malloc(optionCount * sizeof(float));
    h_X       = (float *)malloc(optionCount * sizeof(float));
    h_T       = (float *)malloc(optionCount * sizeof(float));

    srand(2009);
    for(unsigned int i = 0; i < optionCount; i++){
		h_CallCPU[i] = -1.0f;
        h_PutCPU[i]  = -1.0f;
        h_S[i]       = randFloat(5.0f, 30.0f);
        h_X[i]       = randFloat(1.0f, 100.0f);
        h_T[i]       = randFloat(0.25f, 10.0f);
    }


	// Create context
#ifdef __ALTERA_FPGA_OPENCL__		
	cl_int status;
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
	checkError(status, "Failed to create context");
#else
	cl_context_properties properties[3];
	properties[0] = CL_CONTEXT_PLATFORM;
	properties[1] = (cl_context_properties)platform_id[0];
	properties[2] = 0;

	// create context
	cl_context context;
	context = clCreateContext(properties, 1, &device, NULL, NULL, &err);
	if (err != CL_SUCCESS)
	{
		printf("Unable to create context. Error Code=%d\n", err);
		exit(1);
	}
#endif


	// Create command queue
	cl_command_queue queue;
	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	if (err != CL_SUCCESS)
	{
		printf("Unable to create command queue. Error Code=%d\n", err);
		exit(1);
	}

	d_Call = clCreateBuffer(context, CL_MEM_READ_WRITE, optionCount * sizeof(float), NULL, &ciErrNum);
	d_Put = clCreateBuffer(context, CL_MEM_READ_WRITE, optionCount * sizeof(float), NULL, &ciErrNum);
	d_S = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, optionCount * sizeof(float), h_S, &ciErrNum);
	d_X = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, optionCount * sizeof(float), h_X, &ciErrNum);
	d_T = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, optionCount * sizeof(float), h_T, &ciErrNum);
        

	initBlackScholes(context, queue, NULL);

    //Just a single run or a warmup iteration
    BlackScholes(
            NULL,
            d_Call,
            d_Put,
            d_S,
            d_X,
            d_T,
            R,
            V,
            optionCount
        );

#ifdef GPU_PROFILING
    const int numIterations = 16;
    cl_event startMark, endMark;
    ciErrNum = clEnqueueMarker(queue, &startMark);
    ciErrNum |= clFinish(queue);

	double start_time, end_time;
	start_time = getCurrentTimestamp();

    for(int i = 0; i < numIterations; i++){
        BlackScholes(
            queue,
            d_Call,
            d_Put,
            d_S,
            d_X,
            d_T,
            R,
            V,
            optionCount
        );
    }

    ciErrNum  = clEnqueueMarker(queue, &endMark);
    ciErrNum |= clFinish(queue);

	end_time = getCurrentTimestamp();
	const double total_time = end_time - start_time;

	// Wall-clock time taken.
	printf("\nOpenCl took Time: %0.3f ms\n", (total_time * 1e3) / (double)numIterations);


    //Calculate performance metrics by wallclock time


    //Get profiling info
    cl_ulong startTime = 0, endTime = 0;
    ciErrNum  = clGetEventProfilingInfo(startMark, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &startTime, NULL);
    ciErrNum |= clGetEventProfilingInfo(endMark, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);

    printf("\nOpenCL time: %f s\n\n", 1.0e-9 * ((double)endTime - (double)startTime) / (double)numIterations);
#endif

	clFinish(queue);
    printf("\nReading back OpenCL BlackScholes results...\n");
	ciErrNum = clEnqueueReadBuffer(queue, d_Call, CL_TRUE, 0, optionCount * sizeof(float), h_CallGPU, 0, NULL, NULL);
    
	ciErrNum = clEnqueueReadBuffer(queue, d_Put, CL_TRUE, 0, optionCount * sizeof(float), h_PutGPU, 0, NULL, NULL);
    

    printf("Comparing against Host/C++ computation...\n"); 
    BlackScholesCPU(h_CallCPU, h_PutCPU, h_S, h_X, h_T, R, V, optionCount);
    double deltaCall = 0, deltaPut = 0, sumCall = 0, sumPut = 0;
    double L1call, L1put;
    for(unsigned int i = 0; i < optionCount; i++)
     {
           sumCall += fabs(h_CallCPU[i]);
            sumPut  += fabs(h_PutCPU[i]);
            deltaCall += fabs(h_CallCPU[i] - h_CallGPU[i]);
            deltaPut  += fabs(h_PutCPU[i] - h_PutGPU[i]);
        }
        L1call = deltaCall / sumCall; 
        L1put = deltaPut / sumPut;
        printf("Relative L1 (call, put) = (%.3e, %.3e)\n\n", L1call, L1put);

		printf("Shutting down...\n");
        closeBlackScholes();
        ciErrNum  = clReleaseMemObject(d_T);
        ciErrNum |= clReleaseMemObject(d_X);
        ciErrNum |= clReleaseMemObject(d_S);
        ciErrNum |= clReleaseMemObject(d_Put);
        ciErrNum |= clReleaseMemObject(d_Call);
		ciErrNum |= clReleaseCommandQueue(queue);
        ciErrNum |= clReleaseContext(context);
        

        free(h_T);
        free(h_X);
        free(h_S);
        free(h_PutGPU);
        free(h_CallGPU);
        free(h_PutCPU);
        free(h_CallCPU);

       
		printf("press a key\n");
		getchar();
        
}

// Free the resources allocated during initialization
void cleanup() {
 /*   clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);


    clReleaseMemObject(inputImage_buffer);

    clReleaseMemObject(outputImage_buffer);

  if(program) {
    clReleaseProgram(program);
  }
  if(context) {
    clReleaseContext(context);
  }
  */
}