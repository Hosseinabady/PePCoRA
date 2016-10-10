/*
 * fpga_design.h
 *
 *  Created on: 10 Nov 2014
 *      Author: csxmh
 */

#ifndef FPGA_DESIGN_H_
#define FPGA_DESIGN_H_


#define ENPOWER_OPENCL_DEVICE_BASE_ADDRESS                      0x43C00000


#define ENPOWER_OPENCL_DEVICE_ADDR_AP_CTRL                      0x00
#define ENPOWER_OPENCL_DEVICE_ADDR_INPUT_IMAGE_OFFSET_DATA      0x14
#define ENPOWER_OPENCL_DEVICE_ADDR_OUTPUT_IMAGE_OFFSET_DATA     0x1C
#define ENPOWER_OPENCL_DEVICE_ADDR_IMAGE_WIDTH_DATA             0x24
#define ENPOWER_OPENCL_DEVICE_ADDR_IMAGE_HEIGHT_DATA            0x2C
#define ENPOWER_OPENCL_DEVICE_ADDR_INDEX_DATA                   0x34





#define ENPOWER_OPENCL_ARGUMEN_IMAGE_WIDTH_INDEX               2
#define ENPOWER_OPENCL_ARGUMEN_IMAGE_HEIGHT_INDEX              3
#define ENPOWER_OPENCL_ARGUMEN_INDEX_INDEX                     4

#define NUM_OF_ARGUMENT_NUMBER                                 5



#define IMAGE_WIDTH_MAX                                        1920
#define MASK_WIDTH                                             7
#define MASK_HEIGHT                                            7

#define IMAGE_WIDTH                                        1920
#define IMAGE_HEIGHT                                       1080


#define IN_LINE_SIZE         (IMAGE_WIDTH+MASK_WIDTH)*4
#define OUT_LINE_SIZE        (IMAGE_WIDTH+MASK_WIDTH)*4
#define INOUT_LINE_SIZE      IN_LINE_SIZE+OUT_LINE_SIZE


#endif /* FPGA_DESIGN_H_ */
