#include "ap_axi_sdata.h"


#define IMAGE_HEIGHT_MAX  1080
#define IMAGE_WIDTH_MAX   1920

#define LOCAL_IMAGE_HEIGHT  4
#define LOCAL_IMAGE_WIDTH   4

#define MASK_HEIGHT 7
#define MASK_WIDTH  7

typedef unsigned long	u32;

void image_filtering_new (volatile float* memory,  volatile u32 input_image_offset, volatile u32 output_image_offset, volatile u32 image_width, volatile u32 image_height);
