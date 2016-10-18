

#include "image_filtering_new.h"



float mask[49] = {  0, 0, 0, 0, 0, 0.0145, 0 ,
		 0, 0, 0, 0, 0.0376, 0.1283, 0.0145 ,
		 0, 0, 0, 0.0376, 0.1283, 0.0376, 0 ,
		 0, 0, 0.0376, 0.1283, 0.0376, 0, 0 ,
		 0, 0.0376, 0.1283, 0.0376, 0, 0, 0 ,
		 0.0145, 0.1283, 0.0376, 0, 0, 0, 0 ,
		 0, 0.0145, 0, 0, 0, 0, 0  };




float lineBuffer[MASK_HEIGHT][IMAGE_WIDTH_MAX+MASK_WIDTH];
float outputBuffer[IMAGE_WIDTH_MAX+MASK_WIDTH];

void image_filtering_new (volatile float* memory,  volatile u32 input_image_offset, volatile u32 output_image_offset, volatile u32 image_width, volatile u32 image_height, volatile u32 index){



	#pragma HLS INTERFACE ap_bus port=memory
	#pragma HLS RESOURCE core=AXI4M variable=memory


	#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"
	#pragma HLS INTERFACE ap_none register     port=input_image_offset
	#pragma HLS RESOURCE core=AXI4LiteS        variable=input_image_offset metadata="-bus_bundle LITE"


	#pragma HLS INTERFACE ap_none register     port=output_image_offset
	#pragma HLS RESOURCE core=AXI4LiteS        variable=output_image_offset metadata="-bus_bundle LITE"

	#pragma HLS INTERFACE ap_none register     port=image_width
	#pragma HLS RESOURCE core=AXI4LiteS        variable=image_width metadata="-bus_bundle LITE"

	#pragma HLS INTERFACE ap_none register     port=image_height
	#pragma HLS RESOURCE core=AXI4LiteS        variable=image_height metadata="-bus_bundle LITE"

	#pragma HLS INTERFACE ap_none register     port=index
	#pragma HLS RESOURCE core=AXI4LiteS        variable=index metadata="-bus_bundle LITE"




	//float sum = 0;
	for (int row = 0; row < MASK_HEIGHT-1; row++)
		image_filtering_new_label2:for (int col = 0; col < IMAGE_WIDTH_MAX/*image_width*/+MASK_WIDTH; col++) {
				lineBuffer[row][col] = lineBuffer[row+1][col];
	}
	memcpy(lineBuffer[MASK_HEIGHT-1],(float *)(memory+input_image_offset),(IMAGE_WIDTH_MAX/*image_width*/+MASK_WIDTH)*sizeof(float));

	image_filtering_new_label0:for (int col = 0; col < IMAGE_WIDTH_MAX/*image_width*/; col++) { //for generate comparison IMAGE_WIDTH_MAX can be removed
		float sum = 0;
		for (int msk_col = 0; msk_col < MASK_WIDTH; msk_col++)
			image_filtering_new_label3:for (int msk_row = 0; msk_row < MASK_HEIGHT; msk_row++) {
//				if (msk_col == 0 && msk_row == 0)
//					sum = 0;
        		float msk = mask[msk_row*MASK_WIDTH+msk_col];
        		float in_img = lineBuffer[msk_row][col+msk_col];
        		sum += msk * in_img;

 //       		if(msk_col-1 == 0 && msk_row-1 == 0)
 //       			outputBuffer[col]= sum;
        	}

		outputBuffer[col]= sum;

	}



	memcpy((float *)(memory+output_image_offset),outputBuffer,(IMAGE_WIDTH_MAX/*image_width*/+MASK_WIDTH)*sizeof(float));
}





