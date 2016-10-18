/*
 * measurement.h
 *
 *  Created on: 15 Jan 2015
 *      Author: csxmh
 */

#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_
#include <sys/time.h>
#include <pthread.h>




struct voltage_sample {
	int dev_id;
	struct timeval start_time;
	struct timeval end_time;
	char value_buffer[100];
	struct voltage_sample *next;
};




int open_device(char *lane);
int read_voltage(int dev_id, char* value_buffer);

void *read_voltage_pthread( );
void print_voltage_pthread(struct voltage_sample *head);
void clear_voltage_pthread(struct voltage_sample *head);

void *read_voltage_samples(void *tail);
void read_voltage_start( );
void read_voltage_stop( );
#define LENGTH 20



#endif /* MEASUREMENT_H_ */
