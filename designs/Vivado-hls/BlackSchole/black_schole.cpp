#include "black_schole.h"

#include <math.h>
#include <string.h>

#if(0)
    #define EXP(a) native_exp(a)
    #define LOG(a) native_log(a)
    #define SQRT(a) native_sqrt(a)
#else
    #define EXP(a) exp(a)
    #define LOG(a) log(a)
    #define SQRT(a) sqrt(a)
#endif



float CND(float d);

void BlackScholesBody(
    float *call, //Call option price
    float *put,  //Put option price
    float S,              //Current stock price
    float X,              //Option strike price
    float T,              //Option years
    float R,              //Riskless rate of return
    float V               //Stock volatility
);




void black_schole_level3(volatile float* memory, volatile u32 call_offset, volatile u32 put_offset, volatile u32 s_offset, volatile u32 x_offset, volatile u32 t_offset, volatile float r, volatile float v){


#pragma HLS INTERFACE ap_bus port=memory
#pragma HLS RESOURCE core=AXI4M variable=memory


#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"
#pragma HLS INTERFACE ap_none register     port=call_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=call_offset metadata="-bus_bundle LITE"


#pragma HLS INTERFACE ap_none register     port=put_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=put_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=s_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=s_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=x_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=x_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=t_offset
#pragma HLS RESOURCE core=AXI4LiteS        variable=t_offset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=r
#pragma HLS RESOURCE core=AXI4LiteS        variable=r metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=v
#pragma HLS RESOURCE core=AXI4LiteS        variable=v metadata="-bus_bundle LITE"

	float d_Call[GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=d_Call complete dim=1
	float d_Put[GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=d_Put complete dim=1
	float d_S[GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=d_S complete dim=1
	float d_X[GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=d_X complete dim=1
	float d_T[GROUP_SIZE];
//#pragma HLS ARRAY_PARTITION variable=d_T complete dim=1

	memcpy(d_S,(float *)(memory+s_offset),(GROUP_SIZE)*sizeof(float));
	memcpy(d_X,(float *)(memory+x_offset),(GROUP_SIZE)*sizeof(float));
	memcpy(d_T,(float *)(memory+t_offset),(GROUP_SIZE)*sizeof(float));

//	for (int j = 0; j < BLOCK_SIZE; j++) {
//#pragma HLS PIPELINE II=2
//#pragma HLS UNROLL
		for (int i = 0; i < GROUP_SIZE; i++) {
//#pragma HLS UNROLL
#pragma HLS PIPELINE II=2
			BlackScholesBody( &d_Call[i], &d_Put[i], d_S[i], d_X[i], d_T[i], r, v);
		}
//	}

	memcpy((float *)(memory+call_offset), d_Call,(GROUP_SIZE)*sizeof(float));
	memcpy((float *)(memory+put_offset), d_Put,(GROUP_SIZE)*sizeof(float));
}


void BlackScholesBody(
    float *call, //Call option price
    float *put,  //Put option price
    float S,              //Current stock price
    float X,              //Option strike price
    float T,              //Option years
    float R,              //Riskless rate of return
    float V               //Stock volatility
){
    float sqrtT = SQRT(T);
    float    d1 = (LOG(S / X) + (R + 0.5f * V * V) * T) / (V * sqrtT);
    float    d2 = d1 - V * sqrtT;
    float CNDD1 = CND(d1);
    float CNDD2 = CND(d2);

    //Calculate Call and Put simultaneously
    float expRT = EXP(- R * T);
    *call = (S * CNDD1 - X * expRT * CNDD2);
    *put  = (X * expRT * (1.0f - CNDD2) - S * (1.0f - CNDD1));
}


float CND(float d){
    const float       A1 = 0.31938153f;
    const float       A2 = -0.356563782f;
    const float       A3 = 1.781477937f;
    const float       A4 = -1.821255978f;
    const float       A5 = 1.330274429f;
    const float RSQRT2PI = 0.39894228040143267793994605993438f;

    float
        K = 1.0f / (1.0f + 0.2316419f * fabs(d));

    float
        cnd = RSQRT2PI * EXP(- 0.5f * d * d) *
        (K * (A1 + K * (A2 + K * (A3 + K * (A4 + K * A5)))));

    if(d > 0)
        cnd = 1.0f - cnd;

    return cnd;
}
