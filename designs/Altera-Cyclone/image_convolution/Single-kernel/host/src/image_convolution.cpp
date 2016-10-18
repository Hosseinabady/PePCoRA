#include "stdio.h"
#include <stdlib.h>
#include "bmpfuncs.h"
#include <CL/cl.h> 
#include <AOCL_Utils.h>

#define __1920x1080__


#define __ALTERA_FPGA_OPENCL__

#define IMAGE_HEIGHT_MAX  1080
#define IMAGE_WIDTH_MAX   1920


#define MASK_HEIGHT 7
#define MASK_WIDTH  7

typedef unsigned long	u32;

using namespace aocl_utils;
void cleanup();
#ifdef __1920x1080__
#define INPUT_FILE_NAME   "image-1(1920x1080).bmp"
#define OUTPUT_FILE_NAME  "outputImage-1(1920x1080).bmp"
#define IMAGE_WIDTH   1920
#define IMAGE_HEIGHT  1080
#endif



#ifdef __ALTERA_FPGA_OPENCL__
	cl_device_id device;
	cl_program program;
	cl_kernel kernel;
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



int main() {


	int imageHeight;
	int imageWidth;


	long long start, end;
	float total;
	cl_event  event;
	const char* inputFile = INPUT_FILE_NAME;
	const char* outputFile = OUTPUT_FILE_NAME;
	cl_int err;
	// Homegrown function to read a BMP from file
	float* inputImage = readImage(inputFile, &imageWidth, &imageHeight);

	// Size of the input and output images on the host
	int dataSize = imageHeight*imageWidth*sizeof(float);


	// Output image on the host
	float* outputImage = NULL;
	outputImage = (float*)malloc(dataSize);
	int i, j;
	for (i = 0; i < imageHeight; i++) {
		for (j = 0; j < imageWidth; j++) {
			outputImage[i*imageWidth + j] = 0;

		}
	}


	float* image_padding = (float *)malloc(2 * sizeof(float)*(IMAGE_HEIGHT + MASK_HEIGHT)*(IMAGE_WIDTH + MASK_WIDTH));
	float* inputImage_padding = image_padding;
	float* outputImage_padding = image_padding + (IMAGE_HEIGHT + MASK_HEIGHT)*(IMAGE_WIDTH + MASK_WIDTH);

	for (int i = 0; i < IMAGE_HEIGHT + MASK_HEIGHT; i++)
	 for (int j = 0; j < IMAGE_WIDTH + MASK_WIDTH; j++) {
		int row = i - MASK_HEIGHT / 2;
		int col = j - MASK_WIDTH / 2;

		if (row < 0) row = 0;	if (row > IMAGE_HEIGHT - 1) row = IMAGE_HEIGHT - 1;
		if (col < 0) col = 0;   if (col > IMAGE_WIDTH - 1) col = IMAGE_WIDTH - 1;
		image_padding[i*(IMAGE_WIDTH + MASK_WIDTH) + j] = inputImage[row*IMAGE_WIDTH + col];
	}

	int dataSize_padding = (IMAGE_HEIGHT + MASK_HEIGHT)*(IMAGE_WIDTH + MASK_WIDTH)*sizeof(float);

	// 45 degree motion blur
	float filter[49] =
	{ 0, 0, 0, 0, 0, 0.0145, 0,
	0,      0,      0,      0, 0.0376, 0.1283, 0.0145,
	0,      0,      0, 0.0376, 0.1283, 0.0376,      0,
	0, 0, 0.0376, 0.1283, 0.0376, 0, 0,
	0, 0.0376, 0.1283, 0.0376, 0, 0, 0,
	0.0145, 0.1283, 0.0376, 0, 0, 0, 0,
	0, 0.0145, 0, 0, 0, 0, 0 };

	int filterWidth = 7;
	int paddingPixels = (int)(filterWidth / 2) * 2;



	// Discovery platform
	
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




	// Create context
#ifdef __ALTERA_FPGA_OPENCL__	
	cl_int status;
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
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

	// Create memory buffers
	cl_mem d_inputImage;
	cl_mem d_outputImage;

	d_inputImage = clCreateBuffer(context, CL_MEM_READ_ONLY, dataSize_padding, NULL, NULL);
	d_outputImage = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dataSize_padding, NULL, NULL);

	// Write input data to the device

	clEnqueueWriteBuffer(queue, d_inputImage, CL_TRUE, 0, dataSize_padding, inputImage_padding, 0, NULL, NULL);

	// Write the filter to the device
	
#ifdef __ALTERA_FPGA_OPENCL__

	// Create the program for all device. Use the first device as the
	// representative device (assuming all device are of the same type).
	std::string binary_file = getBoardBinaryFile("if_kernel", device);
	printf("Using AOCX: %s\n", binary_file.c_str());
	program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);

	// Build the program that was just created.
	status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
	checkError(status, "Failed to build program");
	
	const char *kernel_name = "if_kernel";
    kernel = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");
	
#else
	// Read in the program from file
	char* source = readSource("if_kernel.cl");

	// Create the program
	cl_program program;

	// Create and compile the program
	program = clCreateProgramWithSource(context, 1, 
		(const char**)&source, NULL, NULL);
	cl_int build_status;
	build_status = clBuildProgram(program, 1, &device, NULL, NULL,
		NULL);

	// Create the kernel
	cl_kernel kernel;
	kernel = clCreateKernel(program, "if_kernel", NULL);
	
#endif
	double start_time, end_time;
	start_time = getCurrentTimestamp();


	// Size of the NDRange
	size_t globalSize[2] = { 1, 1 };

	// Set the kernel arguments
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_inputImage);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_outputImage);


	// Execute the kernel
	clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, 0, 0, NULL, &event);

	// Wait for kernel to complete
	clFinish(queue);

	clEnqueueReadBuffer(queue, d_outputImage, CL_TRUE, 0, dataSize_padding, outputImage_padding, 0, NULL, NULL);

	end_time = getCurrentTimestamp();
	const double total_time = end_time - start_time;

	// Wall-clock time taken.
	printf("\nOpenCl took Time: %0.3f ms\n", total_time * 1e3);

	
	err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
	if (err != CL_SUCCESS) {
		printf("Error start time. Error Code=%d\n", err);
		exit(1);
	}

	err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);
	if (err != CL_SUCCESS) {
		printf("Error start time. Error Code=%d\n", err);
		exit(1);
	}

	total = (double)(end - start) / 1e6;
	printf(" time = %f msec\n", total);
	
	// Read back the output image

	for (int i = 0; i < IMAGE_HEIGHT; i++)
	for (int j = 0; j < IMAGE_WIDTH; j++) {
		outputImage[i*(IMAGE_WIDTH)+j] = outputImage_padding[(i + MASK_HEIGHT / 2)*(IMAGE_WIDTH + MASK_WIDTH) + (j + MASK_WIDTH / 2)];
	}


	// Homegrown function to write the image to file
	storeImage(outputImage, outputFile, imageHeight,
		imageWidth, inputFile);

	// Free OpenCL objects
	clReleaseMemObject(d_inputImage);
	clReleaseMemObject(d_outputImage);
	
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);



	getchar();
	return 0;

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

