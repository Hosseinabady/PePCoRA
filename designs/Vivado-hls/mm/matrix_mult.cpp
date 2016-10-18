#include <string.h>
#include "matrix_mult.h"




typedef unsigned long u32;


void matrix_mult(volatile float* memory, u32 a_offset, u32 b_offset, u32 c_offset, u32 newGroupFlag) {


#pragma HLS INTERFACE ap_bus port=memory
#pragma HLS RESOURCE core=AXI4M variable=memory


#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=a_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=a_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=b_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=b_offset  metadata="-bus_bundle LITE"

#pragma port=c_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=c_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=newGroupFlag
#pragma HLS RESOURCE core=AXI4LiteS        variable=newGroupFlag metadata="-bus_bundle LITE"

//#pragma HLS EXPRESSION_BALANCE

	float a[A_WIDTH];

	static float b[B_HEIGHT][GROUP_SIZE];

	static float c[WORK_ITEM_SIZE];
	float sum;

	memcpy(a,(float *)(memory+a_offset),A_WIDTH*sizeof(float));


	if (newGroupFlag == 1)
		memcpy(b,(float *)(memory+b_offset),B_HEIGHT*GROUP_SIZE*sizeof(float));


	matrix_mult_label0:for (int j = 0; j < WORK_ITEM_SIZE; j++) {

		matrix_mult_label1:for(int k = 0; k < B_HEIGHT; k++) {
#pragma HLS PIPELINE
			if (k == 0)
				sum = 0;
			sum += a[k]*b[k][j];
			if(k == B_HEIGHT-1)
				c[j] = sum;
		}
	}

	memcpy((float *)(memory+c_offset), c, WORK_ITEM_SIZE*sizeof(float));
	return;
}
