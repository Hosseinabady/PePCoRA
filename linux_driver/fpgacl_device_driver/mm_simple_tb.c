/*
 * image_convolution_app.c
 *
 *  Created on: 11 Nov 2014
 *      Author: csxmh
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>		/* ioctl */
#include <sys/time.h>
#include "fpgacl_device_driver.h"
#include "mm_simple_tb.h"

#include "measurement.h"

#define DEVICE_NAME_0    "/dev/fpgacl_acp"

#define ITERATION  100000L




struct read_write_command_struct {
    void*    user_data_address;
    u32     value;
    u32     argument_index;
    u32     read_write;
};
typedef struct read_write_command_struct read_write_command_type;


double getTimestamp();

void main() {

#ifdef POWER_MONITORING
	struct fpgacl_sample *head;
	char *lane_current = "/sys/devices/amba.0/e0004000.ps7-i2c/i2c-0/i2c-8/8-0034/hwmon/hwmon1/curr3_input";
#endif //POWER_MONITORING

    FILE *file;
    char *buffer;
    unsigned long fileLen;
    int file_desc;
    int i, j, k;
    u32 isIdle;

	float ABO[3*SIZE*SIZE];
	float *A = ABO + 0*SIZE*SIZE;
	float *B = ABO + 1*SIZE*SIZE;
	float *C = ABO + 2*SIZE*SIZE;

#ifdef POWER_MONITORING
	head = (struct fpgacl_sample *)fpgacl_read_sample_pthread(lane_current);
	fpgacl_read_sample_start();
#endif //POWER_MONITORING

	for (i = 0; i < SIZE*SIZE; i++) {
		A[i] = 2;
		B[i] = 5;
		C[i] = 1.1;
	}


    file_desc = open(DEVICE_NAME_0, O_RDWR);
    if (file_desc < 0) {
        printf("Can't open device file :%s\n", DEVICE_NAME_0);
        exit(-1);
    }





	argument_parameters_type *first_arg_param = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param->size = SIZE*SIZE*sizeof(float);
	first_arg_param->type_size = sizeof(float);
	first_arg_param->fpga_reg_offset_address =  FPGACL_DEVICE_ADDR_A_OFFSET_DATA;
	first_arg_param->index = 0;
	ioctl ( file_desc, FPGACL_ARGUMEN_POINTER, (u32*)first_arg_param);


	argument_parameters_type *second_arg_param = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param->size = SIZE*SIZE*sizeof(float);
	second_arg_param->type_size = sizeof(float);
	second_arg_param->fpga_reg_offset_address =  FPGACL_DEVICE_ADDR_B_OFFSET_DATA;
	second_arg_param->index = 1;
	ioctl ( file_desc, FPGACL_ARGUMEN_POINTER, (u32*)second_arg_param);

	argument_parameters_type *third_arg_param = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param->size = SIZE*SIZE*sizeof(float);
	third_arg_param->type_size = sizeof(float);
	third_arg_param->fpga_reg_offset_address =  FPGACL_DEVICE_ADDR_C_OFFSET_DATA;
	third_arg_param->index = 2;
	ioctl ( file_desc, FPGACL_ARGUMEN_POINTER, (u32*)third_arg_param);



	read_write_command_type *rd_command = (read_write_command_type*)malloc(sizeof(read_write_command_type));

	printf("Hardware implementation starts\n");


	unsigned long int iter;
	for (iter = 0; iter < ITERATION; iter++) {

		rd_command->user_data_address = (A);
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(file_desc, rd_command, SIZE*SIZE*sizeof(float));


		rd_command->user_data_address = (B);
		rd_command->value = 0;
		rd_command->argument_index = 1;
		rd_command->read_write = 1;

		write(file_desc, rd_command, SIZE*SIZE*sizeof(float));

		ioctl ( file_desc, FPGACL_START_ACP, FPGACL_DEVICE_ADDR_AP_CTRL);

		isIdle =  ioctl ( file_desc, FPGACL_CTRL_ACP, FPGACL_DEVICE_ADDR_AP_CTRL);

		while (!isIdle) {
			isIdle =  ioctl ( file_desc, FPGACL_CTRL_ACP, FPGACL_DEVICE_ADDR_AP_CTRL);
		}

		rd_command->user_data_address = (C);
		rd_command->value = 0;
		rd_command->argument_index = 2;
		rd_command->read_write = 0;

		read(file_desc, rd_command, SIZE*SIZE*sizeof(float));

	}




	float *goldenModel = (float* )malloc(sizeof(float)*SIZE*SIZE);


	float sum;
	for (i = 0; i < SIZE; i++)
	    for (j = 0; j < SIZE; j++) {
	    	sum = 0;
	    	for (k = 0; k < SIZE; k++) {
	    		sum += A[i*SIZE+k]*B[k*SIZE+j];
	    	}
	    	goldenModel[i*SIZE+j] = sum;
	    }


	for (i = 0; i < SIZE*SIZE; i++) {
		if (goldenModel[i] != C[i]) {
	    	printf("golden[%d] = %f    C[%d] = %f  \n", i, goldenModel[i],i, C[i]);
	    	printf("Result error!\n");
	    	exit(-1);
	    }
	}

#ifdef POWER_MONITORING
	fpgacl_read_sample_stop();
	fpgacl_save_sample_pthread(head, "log.txt");
	fpgacl_clear_sample_pthread(head);
#endif	//POWER_MONITORING
	close(file_desc);
	printf("Bye mm_simple\n\r");

}


double getTimestamp()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}



