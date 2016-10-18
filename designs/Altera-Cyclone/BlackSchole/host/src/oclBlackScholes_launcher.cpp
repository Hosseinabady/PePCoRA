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




#include "oclBlackScholes_common.h"
#include "stdio.h"
#include <stdlib.h>
#include "AOCL_Utils.h"

static cl_program cpBlackScholes;   //OpenCL program
static cl_kernel  ckBlackScholes;   //OpenCL kernel
static cl_command_queue cqDefaultCommandQueue;


#define __ALTERA_FPGA_OPENCL__
using namespace aocl_utils;


#ifdef __ALTERA_FPGA_OPENCL__
extern cl_device_id device;
extern cl_program program;
extern cl_kernel kernel;
extern cl_context context;
#endif
// This function reads in a text file and stores it as a char pointer
char* readSource(char* kernelPath) {

	cl_int status;
	FILE *fp;
	char *source;
	long int size;

	printf("Program file is: %s\n", kernelPath);

	fp = fopen(kernelPath, "rb");
	if (!fp) {
		printf("Could not open kernel file\n");
		exit(-1);
	}
	status = fseek(fp, 0, SEEK_END);
	if (status != 0) {
		printf("Error seeking to end of file\n");
		exit(-1);
	}
	size = ftell(fp);
	if (size < 0) {
		printf("Error getting file position\n");
		exit(-1);
	}

	rewind(fp);

	source = (char *)malloc(size + 1);

	int i;
	for (i = 0; i < size + 1; i++) {
		source[i] = '\0';
	}

	if (source == NULL) {
		printf("Error allocating space for the kernel source\n");
		exit(-1);
	}

	fread(source, 1, size, fp);
	source[size] = '\0';

	return source;
}




extern "C" void initBlackScholes(cl_context cxGPUContext, cl_command_queue cqParamCommandQueue, const char **argv){


	
	cl_int ciErrNum;
	size_t kernelLength;


	char *cPathAndName = "BlackScholes.cl";

	#ifdef __ALTERA_FPGA_OPENCL__

	// Create the program for all device. Use the first device as the
	// representative device (assuming all device are of the same type).
	std::string binary_file = getBoardBinaryFile("BlackScholes", device);
	printf("Using AOCX: %s\n", binary_file.c_str());
	program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);

	// Build the program that was just created.
	cl_int status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
	checkError(status, "Failed to build program");
	
	const char *kernel_name = "BlackScholes";
    ckBlackScholes = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");
	
#else
	
	char *cBlackScholes = readSource(cPathAndName);
    

    
    cpBlackScholes = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cBlackScholes, NULL, NULL);

    
    ciErrNum = clBuildProgram(cpBlackScholes, 0, NULL, NULL, NULL, NULL);

    if(ciErrNum != CL_BUILD_SUCCESS){
		printf("*** Compilation failure ***\n");


        size_t deviceNum;
        cl_device_id *cdDevices;
        ciErrNum = clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &deviceNum);

        cdDevices = (cl_device_id *)malloc(deviceNum * sizeof(cl_device_id));
       

        ciErrNum = clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, deviceNum * sizeof(cl_device_id), cdDevices, NULL);
        

        size_t logSize;
        char *logTxt;

        ciErrNum = clGetProgramBuildInfo(cpBlackScholes, cdDevices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        

        logTxt = (char *)malloc(logSize);
        

        ciErrNum = clGetProgramBuildInfo(cpBlackScholes, cdDevices[0], CL_PROGRAM_BUILD_LOG, logSize, logTxt, NULL);
        

        printf("%s\n", logTxt);
        printf("*** Exiting ***\n");
        free(logTxt);
        free(cdDevices);
        exit(666);

    }

    //Save ptx code to separate file
    

    ckBlackScholes = clCreateKernel(cpBlackScholes, "BlackScholes", &ciErrNum);
    free(cBlackScholes);
#endif
    cqDefaultCommandQueue = cqParamCommandQueue;
    
    
}

extern "C" void closeBlackScholes(void){
    cl_int ciErrNum;
    ciErrNum  = clReleaseKernel(ckBlackScholes);
    ciErrNum |= clReleaseProgram(cpBlackScholes);
    
}

////////////////////////////////////////////////////////////////////////////////
// OpenCL Black-Scholes kernel launcher
////////////////////////////////////////////////////////////////////////////////
extern "C" void BlackScholes(
    cl_command_queue cqCommandQueue,
    cl_mem d_Call, //Call option price
    cl_mem d_Put,  //Put option price
    cl_mem d_S,    //Current stock price
    cl_mem d_X,    //Option strike price
    cl_mem d_T,    //Option years
    cl_float R,    //Riskless rate of return
    cl_float V,    //Stock volatility
    cl_uint optionCount
){
    cl_int ciErrNum;

    if(!cqCommandQueue)
        cqCommandQueue = cqDefaultCommandQueue;

    ciErrNum  = clSetKernelArg(ckBlackScholes, 0, sizeof(cl_mem),   (void *)&d_Call);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 1, sizeof(cl_mem),   (void *)&d_Put);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 2, sizeof(cl_mem),   (void *)&d_S);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 3, sizeof(cl_mem),   (void *)&d_X);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 4, sizeof(cl_mem),   (void *)&d_T);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 5, sizeof(cl_float), (void *)&R);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 6, sizeof(cl_float), (void *)&V);
    ciErrNum |= clSetKernelArg(ckBlackScholes, 7, sizeof(cl_uint),  (void *)&optionCount);
    

    //Run the kernel
    size_t globalWorkSize = 60 * 1024;
	size_t localWorkSize = 128;
    ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, ckBlackScholes, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    
}

