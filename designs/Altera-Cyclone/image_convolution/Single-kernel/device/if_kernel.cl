
// TODO: Add OpenCL kernel code here.

//image-1(240x135)

#define IMAGE_HEIGHT  1080
#define IMAGE_WIDTH   1920


#define MASK_HEIGHT 7
#define MASK_WIDTH  7

__kernel void if_kernel(__global float* restrict imageIn,  __global float* restrict imageOut ) {

	
	float sum = 0;

	float Mask[49] =
	{ 0, 0, 0, 0, 0, 0.0145, 0,
	0, 0, 0, 0, 0.0376, 0.1283, 0.0145,
	0, 0, 0, 0.0376, 0.1283, 0.0376, 0,
	0, 0, 0.0376, 0.1283, 0.0376, 0, 0,
	0, 0.0376, 0.1283, 0.0376, 0, 0, 0,
	0.0145, 0.1283, 0.0376, 0, 0, 0, 0,
	0, 0.0145, 0, 0, 0, 0, 0 };


	for (int y = 0; y < IMAGE_HEIGHT; y++) {
		#pragma unroll
		for (int x = 0; x < IMAGE_WIDTH; x++) {
		#pragma unroll 
			for (int l = 0; l < MASK_WIDTH; l++) {
			#pragma unroll 
				for (int k = 0; k < MASK_HEIGHT; k++) {
					int xIn = x + l ;  int yIn = y + k ;
					sum += Mask[l*MASK_WIDTH + k] * imageIn[yIn*(IMAGE_WIDTH + MASK_WIDTH) + xIn];
				}
			}
			imageOut[y*(IMAGE_WIDTH + MASK_WIDTH) + x] = sum;
		}
	}

}