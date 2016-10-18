
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CL/opencl.h"
#include "AOCL_Utils.h"
#include <string.h>
#include <math.h>
#include <cmath>
#define ALTERA_FPGA

using namespace aocl_utils;


#define M_PI 3.14159265358979323846
// OpenCL runtime configuration
cl_platform_id platform = NULL;
unsigned num_devices = 0;
scoped_array<cl_device_id> device; // num_devices elements
cl_context context = NULL;
scoped_array<cl_command_queue> queue; // num_devices elements
cl_program program = NULL;
scoped_array<cl_kernel> kernel; // num_devices elements
scoped_array<cl_mem> positionsIn_buf; // num_devices elements
scoped_array<cl_mem> positionsOut_buf; // num_devices elements
scoped_array<cl_mem> velocities_buf; // num_devices elements

// Problem data.
const unsigned N = 2048; // problem size
scoped_array<scoped_aligned_ptr<cl_float4> > positionsIn, positionsOut; // num_devices elements
scoped_array<scoped_aligned_ptr<cl_float4> > velocities; // num_devices elements

scoped_array<scoped_array<cl_float4> > ref_output; // num_devices elements
scoped_array<unsigned> n_per_device; // num_devices elements

// Function prototypes
float rand_float();
bool init_opencl();
void init_problem();
void run();
void cleanup();


float *h_initialPositions;
float *h_initialVelocities;
float *h_positions;
float *h_distances;

cl_uint  deviceIndex = 1;
cl_uint  numBodies = N;
cl_float delta = 0.005f;
cl_float softening = 0.001f;
cl_uint  iterations = 1;//16;
unsigned distFrequency = 8;
float    sphereRadius = 128;
float    tolerance = 0.001f;

void runReference(const float *initialPositions,
	const float *initialVelocities,
	float *finalPositions);

size_t dataSize;
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
  std::string binary_file = getBoardBinaryFile("nbody", device[0]);
  printf("Using AOCX: %s\n", binary_file.c_str());
  program = createProgramFromBinary(context, binary_file.c_str(), device, num_devices);

  // Build the program that was just created.
  status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
  checkError(status, "Failed to build program");

#else
  // Read in the program from file
  char* source = readSource("nbody.cl");

  // Create the program
  cl_program program;

  // Create and compile the program
  program = clCreateProgramWithSource(context, 1,
	  (const char**)&source, NULL, NULL);
  cl_int build_status;
  build_status = clBuildProgram(program, 1, device, NULL, NULL, NULL);
  if (build_status == CL_BUILD_PROGRAM_FAILURE) {
	  // Determine the size of the log
	  size_t log_size;
	  clGetProgramBuildInfo(program, device[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

	  // Allocate memory for the log
	  char *log = (char *)malloc(log_size);

	  // Get the log
	  clGetProgramBuildInfo(program, device[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

	  // Print the log
	  printf("%s\n", log);
  }

  checkError(build_status, "Failed to build program");
#endif
  // Create per-device objects.
  queue.reset(num_devices);
  kernel.reset(num_devices);
  n_per_device.reset(num_devices);
  positionsIn_buf.reset(num_devices);
  positionsOut_buf.reset(num_devices);
  velocities_buf.reset(num_devices);

  for(unsigned i = 0; i < num_devices; ++i) {
    // Command queue.
    queue[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    // Kernel.
    const char *kernel_name = "nbody";
    kernel[i] = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");

    // Determine the number of elements processed by this device.
    n_per_device[i] = N / num_devices; // number of elements handled by this device

    // Spread out the remainder of the elements over the first
    // N % num_devices.
    if(i < (N % num_devices)) {
      n_per_device[i]++;
    }

    // Input buffers.
	positionsIn_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, n_per_device[i] * sizeof(cl_float4), NULL, &status);
    checkError(status, "Failed to create buffer for input A");

	positionsOut_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, n_per_device[i] * sizeof(cl_float4), NULL, &status);
    checkError(status, "Failed to create buffer for input B");

    // Output buffer.
	velocities_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, n_per_device[i] * sizeof(cl_float4), NULL, &status);
    checkError(status, "Failed to create buffer for output");
  }

  return true;
}

// Initialize the data for the problem. Requires num_devices to be known.
void init_problem() {
  if(num_devices == 0) {
    checkError(-1, "No devices");
  }

  positionsIn.reset(num_devices);
  positionsOut.reset(num_devices);
  velocities.reset(num_devices);
  ref_output.reset(num_devices);

  // Generate input vectors A and B and the reference output consisting
  // of a total of N elements.
  // We create separate arrays for each device so that each device has an
  // aligned buffer. 
  for(unsigned i = 0; i < num_devices; ++i) {
	positionsIn[i].reset(n_per_device[i]);
	positionsOut[i].reset(n_per_device[i]);
	velocities[i].reset(n_per_device[i]);
	ref_output[i].reset(n_per_device[i]);

	size_t dataSize = numBodies*sizeof(cl_float4);
	h_initialPositions = (float *)malloc(dataSize);
	h_initialVelocities = (float *)malloc(dataSize);
	h_positions = (float *)malloc(dataSize);
	h_distances = (float *)malloc(numBodies*numBodies*sizeof(float));
	for (int i = 0; i < numBodies; i++)
	{
		// Generate a random point on the edge of a sphere
		float longitude = 2.f * M_PI * (rand() / (float)RAND_MAX);
		float latitude = acos((2.f * (rand() / (float)RAND_MAX)) - 1);
		h_initialPositions[i * 4 + 0] = sphereRadius * sin(latitude) * cos(longitude);
		h_initialPositions[i * 4 + 1] = sphereRadius * sin(latitude) * sin(longitude);
		h_initialPositions[i * 4 + 2] = sphereRadius * cos(latitude);
		h_initialPositions[i * 4 + 3] = 1;
	}
	memset(h_initialVelocities, 0, dataSize);
	

	
  }
}

void run() {
  cl_int status;
  double start_time_kernel, end_time_kernel, total_kernel = 0;

  const double start_time = getCurrentTimestamp();

  // Launch the problem for each device.
  scoped_array<cl_event> kernel_event(num_devices);
  scoped_array<cl_event> finish_event(num_devices);

  for(unsigned i = 0; i < num_devices; ++i) {

    // Transfer inputs to each device. Each of the host buffers supplied to
    // clEnqueueWriteBuffer here is already aligned to ensure that DMA is used
    // for the host-to-device transfer.
    cl_event write_event[2];
	status = clEnqueueWriteBuffer(queue[i], positionsIn_buf[i], CL_FALSE,
		0, n_per_device[i] * sizeof(cl_float4), h_initialPositions, 0, NULL, &write_event[0]);
    checkError(status, "Failed to transfer input A");

	status = clEnqueueWriteBuffer(queue[i], positionsOut_buf[i], CL_FALSE,
		0, n_per_device[i] * sizeof(cl_float4), h_initialPositions, 0, NULL, &write_event[1]);
    checkError(status, "Failed to transfer input B");

	status = clEnqueueWriteBuffer(queue[i], velocities_buf[i], CL_FALSE,
		0, n_per_device[i] * sizeof(cl_float4), h_initialVelocities, 0, NULL, &write_event[1]);
	checkError(status, "Failed to transfer input B");


	for (int j = 0; j < iterations; j++)
	{
		// Set kernel arguments.
		unsigned argi = 0;

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &positionsIn_buf[i]);
		checkError(status, "Failed to set argument %d", argi - 1);

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &positionsOut_buf[i]);
		checkError(status, "Failed to set argument %d", argi - 1);

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &velocities_buf[i]);
		checkError(status, "Failed to set argument %d", argi - 1);

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_uint), &numBodies);
		checkError(status, "Failed to set argument %d", argi - 1);

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_float), &delta);
		checkError(status, "Failed to set argument %d", argi - 1);

		status = clSetKernelArg(kernel[i], argi++, sizeof(cl_float), &softening);
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
		const size_t global_work_size = n_per_device[i];
		//printf("Launching for device %d (%d elements)\n", i, global_work_size);
		start_time_kernel = getCurrentTimestamp();

		status = clEnqueueNDRangeKernel(queue[i], kernel[i], 1, NULL, &global_work_size, NULL, 2, write_event, &kernel_event[i]);
		checkError(status, "Failed to launch kernel");
		status = clFinish(queue[i]);

		end_time_kernel = getCurrentTimestamp();
		total_kernel += (end_time_kernel - start_time_kernel) * 1e3;
		// Read the result. This the final operation.
		status = clEnqueueReadBuffer(queue[i], positionsOut_buf[i], CL_FALSE, 0, n_per_device[i] * sizeof(cl_float4), positionsOut[i], 1, &kernel_event[i], &finish_event[i]);
		// Swap position buffers
		cl_mem temp = positionsIn_buf[i];
		positionsIn_buf[i] = positionsOut_buf[i];
		positionsOut_buf[i] = temp;
	}
    // Release local events.
    clReleaseEvent(write_event[0]);
    clReleaseEvent(write_event[1]);
	status = clEnqueueReadBuffer(queue[i], positionsIn_buf[i], CL_FALSE, 0, n_per_device[i] * sizeof(cl_float4), h_positions, 1, &kernel_event[i], &finish_event[i]);
  }

  // Wait for all devices to finish.
  clWaitForEvents(num_devices, finish_event);

  const double end_time = getCurrentTimestamp();

  // Wall-clock time taken.
  printf("\nTime OpenCL: %0.3f ms\n", (end_time - start_time) * 1e3);
  printf("\nTime kernel: %0.3f ms\n", total_kernel);
  // Get kernel times using the OpenCL event profiling API.
  for(unsigned i = 0; i < num_devices; ++i) {
    cl_ulong time_ns = getStartEndTime(kernel_event[i]);
 //   printf("Kernel time (device %d): %0.3f ms\n", i, double(time_ns) * 1e-6);
  }

  // Release all events.
  for(unsigned i = 0; i < num_devices; ++i) {
    clReleaseEvent(kernel_event[i]);
    clReleaseEvent(finish_event[i]);
  }

  // Verify results.
  // Verify final positions
  float *h_reference = (float *)malloc(dataSize);
  runReference(h_initialPositions, h_initialVelocities, h_reference);
  unsigned errors = 0;
  for (int i = 0; i < numBodies; i++)
  {
	  float ix = h_positions[i * 4 + 0];
	  float iy = h_positions[i * 4 + 1];
	  float iz = h_positions[i * 4 + 2];

	  float rx = h_reference[i * 4 + 0];
	  float ry = h_reference[i * 4 + 1];
	  float rz = h_reference[i * 4 + 2];

	  float dx = (rx - ix);
	  float dy = (ry - iy);
	  float dz = (rz - iz);
	  float dist = sqrt(dx*dx + dy*dy + dz*dz);

	  if (dist > tolerance)
	  {
		  if (!errors)
		  {
			  printf("Verification failed:\n");
		  }

		  // Only show the first 8 errors
		  if (errors++ < 8)
		  {
			  printf("-> Position error at %d: %.8f\n", i, dist);
		  }
	  }
  }
  if (errors)
  {
	  printf("Total errors: %d\n", errors);
  }
  else
  {
	  printf("Verification passed.\n");
  }
  printf("\n");
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
	if (positionsIn_buf && positionsIn_buf[i]) {
		clReleaseMemObject(positionsIn_buf[i]);
    }
	if (positionsOut_buf && positionsOut_buf[i]) {
		clReleaseMemObject(positionsOut_buf[i]);
    }
	if (velocities_buf && velocities_buf[i]) {
		clReleaseMemObject(velocities_buf[i]);
    }
  }

  if(program) {
    clReleaseProgram(program);
  }
  if(context) {
    clReleaseContext(context);
  }
}

void runReference(const float *initialPositions,
	const float *initialVelocities,
	float *finalPositions)
{
	size_t dataSize = numBodies * 4 * sizeof(float);
	float *positionsIn = (float *)malloc(dataSize);
	float *positionsOut = (float *)malloc(dataSize);
	float *velocities = (float *)malloc(dataSize);

	memcpy(positionsIn, initialPositions, dataSize);
	memcpy(velocities, initialVelocities, dataSize);

	for (int itr = 0; itr < iterations; itr++)
	{
		for (int i = 0; i < numBodies; i++)
		{
			float ix = positionsIn[i * 4 + 0];
			float iy = positionsIn[i * 4 + 1];
			float iz = positionsIn[i * 4 + 2];
			float iw = positionsIn[i * 4 + 3];

			float fx = 0.f;
			float fy = 0.f;
			float fz = 0.f;

			for (int j = 0; j < numBodies; j++)
			{
				float jx = positionsIn[j * 4 + 0];
				float jy = positionsIn[j * 4 + 1];
				float jz = positionsIn[j * 4 + 2];
				float jw = positionsIn[j * 4 + 3];

				// Compute distance between bodies
				float dx = (jx - ix);
				float dy = (jy - iy);
				float dz = (jz - iz);
				float dist = sqrt(dx*dx + dy*dy + dz*dz + softening*softening);

				// Compute interaction force
				float coeff = jw / (dist*dist*dist);
				fx += coeff * dx;
				fy += coeff * dy;
				fz += coeff * dz;
			}

			// Update velocity
			float vx = velocities[i * 4 + 0] + fx * delta;
			float vy = velocities[i * 4 + 1] + fy * delta;
			float vz = velocities[i * 4 + 2] + fz * delta;
			velocities[i * 4 + 0] = vx;
			velocities[i * 4 + 1] = vy;
			velocities[i * 4 + 2] = vz;

			// Update position
			positionsOut[i * 4 + 0] = ix + vx * delta;
			positionsOut[i * 4 + 1] = iy + vy * delta;
			positionsOut[i * 4 + 2] = iz + vz * delta;
			positionsOut[i * 4 + 3] = iw;
		}

		// Swap buffers
		float *temp = positionsIn;
		positionsIn = positionsOut;
		positionsOut = temp;
	}

	memcpy(finalPositions, positionsIn, dataSize);

	free(positionsIn);
	free(positionsOut);
	free(velocities);


}