#include "stdio.h"
#include "stdlib.h"
#include "matrix_mult.h"




/*
int main() {



	float *ABC = (float *)malloc(sizeof(float)*(3*SIZE+2*GROUP_SIZE*DIM));
	float *A = ABC;
	float *B = ABC+SIZE;
	float *C = ABC+2*SIZE;
	float* B_tmp = ABC+3*SIZE;
	float* C_tmp = ABC+3*SIZE+GROUP_SIZE*DIM;


	unsigned int b_offset, c_offset;

	if (GROUP_SIZE == DIM) {
		b_offset = SIZE;
		c_offset = 2*SIZE;
	} else {
		b_offset = 3*SIZE;
		c_offset = 3*SIZE+GROUP_SIZE*DIM;
	}




	for (int i = 0; i < SIZE; i++) {
    	A[i] = 4.34;
    	B[i] = 1.78;
    	C[i] = 0;
    }

	for (int k = 0; k < DIM/GROUP_SIZE; k++) {
		if (GROUP_SIZE!=DIM) {
			for (int i_i = 0; i_i < GROUP_SIZE; i_i++)
				for (int j_j = 0; j_j < DIM; j_j++)
					B_tmp[j_j*GROUP_SIZE+i_i] = B[j_j*DIM+i_i+k*GROUP_SIZE];
		}


		for (int i = 0; i < DIM; i++) {
			if (i == 0) {
				matrix_mult(ABC, 0, b_offset, c_offset, 1, GROUP_SIZE, DIM);
			} else {
				matrix_mult(ABC, 0+i*DIM, b_offset, c_offset+i*GROUP_SIZE, 0, GROUP_SIZE, DIM);
			}

		}


		if (GROUP_SIZE!=DIM) {
			for (int i_i = 0; i_i < GROUP_SIZE; i_i++)
				for (int j_j = 0; j_j < DIM; j_j++)
					C[j_j*DIM+(i_i)+k*GROUP_SIZE] = C_tmp[j_j*GROUP_SIZE+i_i];
		}

    }


    float *goldenModel = (float* )malloc(sizeof(float)*SIZE);

    for (int i = 0; i < DIM; i++)
    	for (int j = 0; j < DIM; j++) {
    		goldenModel[j*DIM+i] = 0;
    		for (int k = 0; k < DIM; k++) {
    			goldenModel[j*DIM+i] += A[j*DIM+k]*B[k*DIM+i];
    		}
    	}


    for (int i = 0; i < SIZE; i++) {
    	if (goldenModel[i] != C[i]) {
    		printf("golden[%d] = %f    C[%d] = %f  \n", i, goldenModel[i],i, C[i]);
    		printf("Result error!\n");
    		return -1;
    	}
    }

    printf("Result match!\n");

	return 0;
}
*/


int main() {



	float *ABC = (float *)malloc(sizeof(float)*(A_HEIGHT*A_WIDTH+            //A
			                                    B_HEIGHT*B_WIDTH+            //B
			                                    A_HEIGHT*B_WIDTH+            //C
			                                    B_HEIGHT*GROUP_SIZE+         //B_tmp
			                                    GROUP_SIZE));                //C_tmp
	float *A = ABC;
	float *B = A+A_HEIGHT*A_WIDTH;
	float *C = B+B_HEIGHT*B_WIDTH;
	float* B_tmp = C+A_HEIGHT*B_WIDTH;
	float* C_tmp = B_tmp+B_HEIGHT*GROUP_SIZE;


	unsigned int b_offset, c_offset;

	if (GROUP_SIZE == B_WIDTH) {
		b_offset = A_HEIGHT*A_WIDTH;
		c_offset = A_HEIGHT*A_WIDTH + B_HEIGHT*B_WIDTH;
	} else {
		b_offset = A_HEIGHT*A_WIDTH + B_HEIGHT*B_WIDTH + A_HEIGHT*B_WIDTH;
		c_offset = A_HEIGHT*A_WIDTH + B_HEIGHT*B_WIDTH + A_HEIGHT*B_WIDTH + B_HEIGHT*GROUP_SIZE;
	}




	for (int i = 0; i < A_HEIGHT*A_WIDTH; i++)
    	A[i] = 4.34;
	for (int i = 0; i < B_HEIGHT*B_WIDTH; i++)
    	B[i] = 1.78;
	for (int i = 0; i < A_HEIGHT*B_WIDTH; i++)
    	C[i] = 0;


	for (int k = 0; k < B_WIDTH/GROUP_SIZE; k++) {
		if (GROUP_SIZE!=B_WIDTH) {
			for (int i_i = 0; i_i < GROUP_SIZE; i_i++)
				for (int j_j = 0; j_j < B_HEIGHT; j_j++)
					B_tmp[j_j*GROUP_SIZE+i_i] = B[j_j*B_WIDTH+i_i+k*GROUP_SIZE];
		}


		for (int i = 0; i < A_HEIGHT; i++) {
			for (int p = 0; p < GROUP_SIZE/WORK_ITEM_SIZE; p++) {
				if (i == 0 && p == 0) {
					matrix_mult(ABC, 0, b_offset, c_offset, 1);
				} else {
					if (GROUP_SIZE == B_WIDTH) {
						matrix_mult(ABC, 0+i*A_WIDTH, b_offset, c_offset+i*GROUP_SIZE, 0);
					} else {
						matrix_mult(ABC, 0+i*A_WIDTH, b_offset, c_offset, 0);
					}
				}

				if (GROUP_SIZE != B_WIDTH) {
					for (int i_i = 0; i_i < WORK_ITEM_SIZE; i_i++)
						C[i*B_WIDTH+(i_i)+p*WORK_ITEM_SIZE+k*GROUP_SIZE] = C_tmp[i_i];
				} else if (WORK_ITEM_SIZE != GROUP_SIZE) {
					for (int i_i = 0; i_i < WORK_ITEM_SIZE; i_i++)
						C[i*B_WIDTH+(i_i)+p*WORK_ITEM_SIZE+k*GROUP_SIZE] = C_tmp[i_i];
				}
			}
		}

    }


    float *goldenModel = (float* )malloc(sizeof(float)*A_HEIGHT*B_WIDTH);

    for (int i = 0; i < B_WIDTH; i++)
    	for (int j = 0; j < A_HEIGHT; j++) {
    		goldenModel[j*B_WIDTH+i] = 0;
    		for (int k = 0; k < A_WIDTH; k++) {
    			goldenModel[j*B_WIDTH+i] += A[j*A_WIDTH+k]*B[k*B_WIDTH+i];
    		}
    	}


    for (int i = 0; i < A_HEIGHT*B_WIDTH; i++) {
    	if (goldenModel[i] != C[i]) {
    		printf("golden[%d] = %f    C[%d] = %f  \n", i, goldenModel[i],i, C[i]);
    		printf("Result error!\n");
    		return -1;
    	}
    }

    printf("Result match!\n");

	return 0;
}
