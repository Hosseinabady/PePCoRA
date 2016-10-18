
#define MATRIX_WIDTH    1024
#define MATRIX_HEIGHT   1024


__kernel void matrix_mult(__global float *restrict arrayA, __global float *restrict arrayB, __global float *restrict arrayC) {

	int globalCol = get_global_id(0);
	int globalRow = get_global_id(1);

	float sum = 0;
	//#pragma unroll  32
	for (int i = 0; i < MATRIX_WIDTH; i++) {
		sum += arrayA[globalRow*MATRIX_WIDTH + i] * arrayB[i*MATRIX_WIDTH + globalRow];
	}

	arrayC[globalRow*MATRIX_WIDTH + globalCol] = sum;

	return;
}
