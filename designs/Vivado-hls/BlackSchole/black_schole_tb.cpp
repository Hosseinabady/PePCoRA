
#include "black_schole.h"
#include <stdlib.h>     /* srand, rand */
#include <stdio.h>
#include <math.h>

float randFloat(float low, float high){
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}

int main(int argc, char **argv) {
	float	*h_CallCPU,
	        *h_PutCPU,
	        *h_CallGPU,
	        *h_PutGPU,
	        *h_S,
	        *h_X,
	        *h_T;


	const unsigned int   optionCount = OPT_N;
	const float                    R = 0.02f;
	const float                    V = 0.30f;



	float	*memory = (float *)malloc(5*optionCount * sizeof(float));
	h_CallCPU = memory + 0 * optionCount;
	h_PutCPU  = memory + 1 * optionCount;
	h_S       = memory + 2 * optionCount;
	h_X       = memory + 3 * optionCount;
	h_T       = memory + 4 * optionCount;

	srand(2009);
	for(unsigned int i = 0; i < optionCount; i++){
		h_CallCPU[i] = -1.0f;
	    h_PutCPU[i]  = -1.0f;
	    h_S[i]       = randFloat(5.0f, 30.0f);
	    h_X[i]       = randFloat(1.0f, 100.0f);
	    h_T[i]       = randFloat(0.25f, 10.0f);
	}

	for (int i = 0; i < (optionCount/GROUP_SIZE)-2; i++)
		black_schole_level3 (memory, 0 * optionCount+i*GROUP_SIZE, 1 * optionCount+i*GROUP_SIZE, 2 * optionCount+i*GROUP_SIZE, 3 * optionCount+i*GROUP_SIZE, 4 * optionCount+i*GROUP_SIZE, R, V);

	return 0;
}
