#include "nbody.h"
#include <math.h>
#include <string.h>




void nbody(volatile float* memory, volatile u32 positionsIn_offset, volatile u32 positionsOut_offset, volatile u32 velocitiesIn_offset, volatile u32 velocitiesOut_offset, int index){


#pragma HLS INTERFACE ap_bus port=memory
#pragma HLS RESOURCE core=AXI4M variable=memory


#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"
#pragma HLS INTERFACE ap_none register     port=positionsIn_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=positionsIn_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=positionsOut_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=positionsOut_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=velocitiesIn_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=velocitiesIn_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=velocitiesOut_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=velocitiesOut_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=index
#pragma HLS RESOURCE core=AXI4LiteS        variable=index metadata="-bus_bundle LITE"



	float positionsIn[4*NUM_BODIES];
#pragma HLS ARRAY_PARTITION variable=positionsIn cyclic factor=8 dim=1
//#pragma HLS ARRAY_PARTITION variable=positionsIn complete dim=1
	float velocitiesIn[4*NUM_BODIES];
//#pragma HLS ARRAY_PARTITION variable=velocitiesIn complete dim=1
#pragma HLS ARRAY_PARTITION variable=velocitiesIn cyclic factor=8 dim=1
	float positionsOut[4*GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=positionsOut complete dim=1
#pragma HLS ARRAY_PARTITION variable=positionsOut cyclic factor=8 dim=1
	float velocitiesOut[4*GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=velocitiesOut complete dim=1
#pragma HLS ARRAY_PARTITION variable=velocitiesOut cyclic factor=8 dim=1

	memcpy(positionsIn,(float *)(memory+positionsIn_offset),(NUM_BODIES)*4*sizeof(float));
	memcpy(velocitiesIn,(float *)(memory+velocitiesIn_offset),(NUM_BODIES)*4*sizeof(float));

	float ipos[4];
	float force[4];
	float velocity[4];

	for (int i = 0; i < GROUP_SIZE; i++) {
#define B  8
		for (int j_j = 0; j_j < (NUM_BODIES)/B; j_j++) {
#pragma HLS PIPELINE II=1
			for (int j_i = 0; j_i < B; j_i++) {
				int j = j_j*B+j_i;

				if (j == 0) {
					ipos[0] = positionsIn[(i+index*GROUP_SIZE)*4+0];
					ipos[1] = positionsIn[(i+index*GROUP_SIZE)*4+1];
					ipos[2] = positionsIn[(i+index*GROUP_SIZE)*4+2];
					ipos[3] = positionsIn[(i+index*GROUP_SIZE)*4+3];


					force[0] = 0.0f;
					force[1] = 0.0f;
					force[2] = 0.0f;
					force[3] = 0.0f;
				}

				float d[4];
				float f[4];
				float jpos[4];
				jpos[0] = positionsIn[j*4+0];
				jpos[1] = positionsIn[j*4+1];
				jpos[2] = positionsIn[j*4+2];
				jpos[3] = positionsIn[j*4+3];

				d[0] = jpos[0] - ipos[0];
				d[1] = jpos[1] - ipos[1];
				d[2] = jpos[2] - ipos[2];
				d[3]    = 0;

				float  distSq = d[0]*d[0] + d[1]*d[1] + d[2]*d[2] + SOFTENING*SOFTENING;
				float  dist   = sqrt(distSq);
				float  coeff  = jpos[3] / (dist*dist*dist);
				f[0] = coeff*d[0];
				f[1] = coeff*d[1];
				f[2] = coeff*d[2];
				f[3] = coeff*d[3];



				force[0] += f[0];
				force[1] += f[1];
				force[2] += f[2];
				force[3] += f[3];


				if (j == NUM_BODIES-1) {
					// Update velocity

						velocity[0] = velocitiesIn[(i+index*GROUP_SIZE)*4+0];
						velocity[1] = velocitiesIn[(i+index*GROUP_SIZE)*4+1];
						velocity[2] = velocitiesIn[(i+index*GROUP_SIZE)*4+2];
						velocity[3] = velocitiesIn[(i+index*GROUP_SIZE)*4+3];

						velocity[0]       += force[0] * DELTA;
						velocity[1]       += force[1] * DELTA;
						velocity[2]       += force[2] * DELTA;
						velocity[3]       += force[3] * DELTA;

						velocitiesOut[i*4+0]   = velocity[0];
						velocitiesOut[i*4+1]   = velocity[1];
						velocitiesOut[i*4+2]   = velocity[2];
						velocitiesOut[i*4+3]   = velocity[3];

						// Update position
						positionsOut[i*4+0] = ipos[0] + velocity[0] * DELTA;
						positionsOut[i*4+1] = ipos[1] + velocity[1] * DELTA;
						positionsOut[i*4+2] = ipos[2] + velocity[2] * DELTA;
						positionsOut[i*4+3] = ipos[3] + velocity[3] * DELTA;

				}
			}
		}

	}

	memcpy((float *)(memory+positionsOut_offset),positionsOut,(GROUP_SIZE)*4*sizeof(float));
	memcpy((float *)(memory+velocitiesOut_offset),velocitiesOut,(GROUP_SIZE)*4*sizeof(float));

}
