

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CL/opencl.h"
#include "AOCL_Utils.h"
#include "mm.h"


#define ALTERA_FPGA

using namespace aocl_utils;



typedef unsigned long	u32;

const unsigned N = MATRIX_WIDTH*MATRIX_HEIGHT;




// OpenCL runtime configuration
cl_platform_id platform = NULL;
unsigned num_devices = 0;
scoped_array<cl_device_id> device; // num_devices elements
cl_context context = NULL;
scoped_array<cl_command_queue> queue; // num_devices elements
cl_program program = NULL;
scoped_array<cl_kernel> kernel; // num_devices elements

scoped_array<cl_mem> arrayA_buffer; // num_devices elements
scoped_array<cl_mem> arrayB_buffer; // num_devices elements
scoped_array<cl_mem> arrayC_buffer; // num_devices elements


// Problem data.

scoped_array<scoped_aligned_ptr<float> > arrayA; 
scoped_array<scoped_aligned_ptr<float> > arrayB;
scoped_array<scoped_aligned_ptr<float> > arrayC;


scoped_array<unsigned> nx_per_device; // num_devices elements
scoped_array<unsigned> ny_per_device; // num_devices elements

// Function prototypes
float rand_float();
bool init_opencl();
void init_problem();
void run();
void cleanup();


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


// Entry point.
int main() {
  // Initialize OpenCL.
  if(!init_opencl()) {
    return -1;
  }

  // Initialize the problem data.
  // Requires the number of devices to be known.
  init_problem();

  // Run the kernel.
  run();

  // Free the resources allocated
  cleanup();

  getchar();
  return 0;
}

/////// HELPER FUNCTIONS ///////

// Randomly generate a floating-point number between -10 and 10.
float rand_float() {
  return float(rand()) / float(RAND_MAX) * 20.0f - 10.0f;
}

// Initializes the OpenCL objects.
bool init_opencl() {
  cl_int status;

  printf("Initializing OpenCL\n");

  if(!setCwdToExeDir()) {
    return false;
  }

  // Get the OpenCL platform.
#ifdef ALTERA_FPGA
  platform = findPlatform("Altera");
  if(platform == NULL) {
	printf("ERROR: Unable to find Altera OpenCL platform.\n");
    return false;
  }
#else
  cl_platform_id platform_id[100];
  cl_int err = clGetPlatformIDs(100, platform_id, NULL);
  if (err != CL_SUCCESS) {
	  printf("Unable to get Platform ID. Error Code=%d\n", err);
	  exit(1);
  }
#endif

  // Query the available OpenCL device.
#ifdef ALTERA_FPGA
  device.reset(getDevices(platform, CL_DEVICE_TYPE_ALL, &num_devices));
  printf("Platform: %s\n", getPlatformName(platform).c_str());
  printf("Using %d device(s)\n", num_devices);
  for(unsigned i = 0; i < num_devices; ++i) {
    printf("  %s\n", getDeviceName(device[i]).c_str());
  }

#else
  // Discover device
  //cl_device_id device_id;
  cl_device_id *dids = new cl_device_id[1];
  err = clGetDeviceIDs(platform_id[1], CL_DEVICE_TYPE_GPU, 1, dids, NULL);
  
  if (err != CL_SUCCESS) {
	  printf("Unable to get Device ID. Error Code=%d\n", err);
	  exit(1);
  }

  device = dids;
  num_devices = 1;
#endif
  // Create the context.
  context = clCreateContext(NULL, num_devices, device, NULL, NULL, &status);
  checkError(status, "Failed to create context");


#ifdef ALTERA_FPGA
  // Create the program for all device. Use the first device as the
  // representative device (assuming all device are of the same type).
  std::string binary_file = getBoardBinaryFile("mm", device[0]);
  printf("Using AOCX: %s\n", binary_file.c_str());
    program = createProgramFromBinary(context, binary_file.c_str(), device, num_devices);

  // Build the program that was just created.
  status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
  checkError(status, "Failed to build program");

#else
  // Read in the program from file
  char* source = readSource("mm.cl");

  // Create the program
  cl_program program;

  // Create and compile the program
  program = clCreateProgramWithSource(context, 1,
	  (const char**)&source, NULL, NULL);
  cl_int build_status;
  build_status = clBuildProgram(program, 1, device, NULL, NULL, NULL);

#endif
  // Create per-device objects.
  queue.reset(num_devices);
  kernel.reset(num_devices);
  nx_per_device.reset(num_devices);
  ny_per_device.reset(num_devices);
  arrayA_buffer.reset(num_devices);
  arrayB_buffer.reset(num_devices);
  arrayC_buffer.reset(num_devices);
  


  for(unsigned i = 0; i < num_devices; ++i) {
    // Command queue.
    queue[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    // Kernel.
    const char *kernel_name = "matrix_mult";
    kernel[i] = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");

    // Determine the number of elements processed by this device.
	nx_per_device[i] = MATRIX_WIDTH / num_devices; // number of elements handled by this device
	ny_per_device[i] = MATRIX_HEIGHT / num_devices; // number of elements handled by this device

    // Spread out the remainder of the elements over the first
    // N % num_devices.
	if (i < (MATRIX_WIDTH % num_devices)) {
      nx_per_device[i]++;
    }

	if (i < (MATRIX_HEIGHT % num_devices)) {
      ny_per_device[i]++;
    }

    // Input buffers.
    arrayA_buffer[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, nx_per_device[i]*ny_per_device[i] * sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for input arrayA");

	arrayB_buffer[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, nx_per_device[i] * ny_per_device[i] * sizeof(float), NULL, &status);
	checkError(status, "Failed to create buffer for input arrayB");


    // Output buffer.
	arrayC_buffer[i] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, nx_per_device[i] * ny_per_device[i] * sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for output arrayC");
  }

  return true;
}

// Initialize the data for the problem. Requires num_devices to be known.
void init_problem() {
  if(num_devices == 0) {
    checkError(-1, "No devices");
  }

  arrayA.reset(num_devices);
  arrayB.reset(num_devices);
  arrayC.reset(num_devices);
  


  // Generate input vectors A and B and the reference output consisting
  // of a total of N elements.
  // We create separate arrays for each device so that each device has an
  // aligned buffer. 

  int k = 0;
  for(unsigned i = 0; i < num_devices; ++i) {
    arrayA[i].reset(nx_per_device[i]*ny_per_device[i]);
	arrayB[i].reset(nx_per_device[i]*ny_per_device[i]);
    arrayC[i].reset(nx_per_device[i]*ny_per_device[i]);

    

	for (unsigned j = 0; j < nx_per_device[i] * ny_per_device[i]; ++j) {
      arrayA[i][j] = 2.45;
	  arrayB[i][j] = 1.35;
	  arrayC[i][j] = 0;
    }
  }
}

void run() {
  cl_int status;

  const double start_time = getCurrentTimestamp();

  // Launch the problem for each device.

  for(unsigned i = 0; i < num_devices; ++i) {

    // Transfer inputs to each device. Each of the host buffers supplied to
    // clEnqueueWriteBuffer here is already aligned to ensure that DMA is used
    // for the host-to-device transfer.
    cl_event write_event[2];
   // status = clEnqueueWriteBuffer(queue[i], inputImage_buffer[i], CL_TRUE,
   //     0, n_per_device[i] * sizeof(float), inputImage[i], 0, NULL, &write_event[0]);
	status = clEnqueueWriteBuffer(queue[i], arrayA_buffer[i], CL_TRUE, 0, nx_per_device[i]*ny_per_device[i] * sizeof(float), arrayA[i], 0, NULL, NULL);
    checkError(status, "Failed to transfer input A");

	status = clEnqueueWriteBuffer(queue[i], arrayB_buffer[i], CL_TRUE, 0, nx_per_device[i] * ny_per_device[i] * sizeof(float), arrayB[i], 0, NULL, NULL);
	checkError(status, "Failed to transfer input A");
	

    // Set kernel arguments.
    unsigned argi = 0;

    status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &arrayA_buffer[i]);
    checkError(status, "Failed to set argument %d", argi - 1);

	status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &arrayB_buffer[i]);
	checkError(status, "Failed to set argument %d", argi - 1);
    
    status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &arrayC_buffer[i]);
    checkError(status, "Failed to set argument %d", argi - 1);

    // Enqueue kernel.
    // Use a global work size corresponding to the number of elements to add
    // for this device.
    // 
    // We don't specify a local work size and let the runtime choose
    // (it'll choose to use one work-group with the same size as the global
    // work-size).
    //
    // Events are used to ensure that the kernel is not launched until
    // the writes to the input buffers have completed.
	const size_t global_work_size[2] = {nx_per_device[i], ny_per_device[i]};

    printf("Launching for device %d (%d elements)\n", i, global_work_size);

  //  status = clEnqueueNDRangeKernel(queue[i], kernel[i], 1, NULL,
   //     &global_work_size, NULL, 2, write_event, &kernel_event[i]);

	status = clEnqueueNDRangeKernel(queue[i], kernel[i], 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    checkError(status, "Failed to launch kernel");

	

	 status = clFinish(queue[i]);
	
	 checkError(status, "Fail to complete kernel");
    // Read the result. This the final operation.
    //status = clEnqueueReadBuffer(queue[i], outputImage_buffer[i], CL_FALSE,
    //    0, n_per_device[i] * sizeof(float), outputImage[i], 1, &kernel_event[i], &finish_event[i]);

	 status = clEnqueueReadBuffer(queue[i], arrayC_buffer[i], CL_TRUE, 0, nx_per_device[i]*ny_per_device[i] * sizeof(float), arrayC[i], 0, NULL, NULL);


    // Release local events.
   // clReleaseEvent(write_event[0]);
   // clReleaseEvent(write_event[1]);
  }

  // Wait for all devices to finish.
//  clWaitForEvents(num_devices, finish_event);

  const double end_time = getCurrentTimestamp();

  // Wall-clock time taken.
  printf("\nTime: %0.3f ms\n", (end_time - start_time) * 1e3);

  // Get kernel times using the OpenCL event profiling API.
  //for(unsigned i = 0; i < num_devices; ++i) {
  //  cl_ulong time_ns = getStartEndTime(kernel_event[i]);
  //  printf("Kernel time (device %d): %0.3f ms\n", i, double(time_ns) * 1e-6);
  //}

  // Release all events.
  for(unsigned i = 0; i < num_devices; ++i) {
//    clReleaseEvent(kernel_event[i]);
//    clReleaseEvent(finish_event[i]);
  }
  
  
  
  float *goldenModel = (float*)malloc(sizeof(float)*MATRIX_HEIGHT*MATRIX_WIDTH);

  for (int i = 0; i < MATRIX_WIDTH; i++)
	for (int j = 0; j < MATRIX_HEIGHT; j++) {
	  goldenModel[j*MATRIX_WIDTH + i] = 0;
	  for (int k = 0; k < MATRIX_WIDTH; k++) {
		  goldenModel[j*MATRIX_WIDTH + i] += arrayA[0][j*MATRIX_WIDTH + k] * arrayB[0][k*MATRIX_WIDTH + i];
	  }
  }

  printf("\nVerification:\n");

  for (int i = 0; i < MATRIX_WIDTH; i++)
	for (int j = 0; j < MATRIX_HEIGHT; j++) {
		float temp = arrayC[0][j*MATRIX_WIDTH + i];
		if (goldenModel[j*MATRIX_WIDTH + i] != temp) {
			printf("Error at index (%d, %d)\n", j, i);
			printf("\nVerification: FAIL\n");
			exit(1);
		}
	}


  printf("\nVerification: PASS\n");

}

// Free the resources allocated during initialization
void cleanup() {
  for(unsigned i = 0; i < num_devices; ++i) {
    if(kernel && kernel[i]) {
      clReleaseKernel(kernel[i]);
    }
    if(queue && queue[i]) {
      clReleaseCommandQueue(queue[i]);
    }

	if (arrayA_buffer && arrayA_buffer[i]) {
		clReleaseMemObject(arrayA_buffer[i]);
    }
	if (arrayA_buffer && arrayA_buffer[i]) {
		clReleaseMemObject(arrayA_buffer[i]);
    }
  }

  if(program) {
    clReleaseProgram(program);
  }
  if(context) {
    clReleaseContext(context);
  }
}

