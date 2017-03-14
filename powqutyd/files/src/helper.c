/*
 * helper.c
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#include "helper.h"
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include "storage.h"

#define MAX_LINE_LENGTH 140
#define TS_POS 2

struct file_cfg *fcfg;

void init_log_file(struct powquty_conf *config) {
	fcfg = malloc(sizeof(struct file_cfg));
	/* init log file parameters */
	set_max_logsize(fcfg, config->max_log_size_kb);
	set_log_path(fcfg, config->powquty_path);
	set_line_length(fcfg, MAX_LINE_LENGTH);
	set_timestamp_position(fcfg, TS_POS);
}

void print_received_buffer(unsigned char* buf, int len) {
	if(len>0) {
		int i=0;
		char c;
		printf("Received[%d] ",len);
		for (i=0;i<len;i++) {
			c= buf[i];
			printf("%x ", (unsigned char) c);
		}
		printf("\n");
	}
}

float get_float_val(unsigned char* buf) {
	float res=0.0;
	//* ((unsigned char *)&x+0 )= buf[3];
	unsigned int bin = buf[3]<<24|buf[2]<<16|buf[1]<<8|buf[0];
	// printf("Uint: \t");
	// print_received_buffer( (unsigned char *)&bin, 4);
	// unsigned char reversed[] = {buf[3], buf[2], buf[1], buf[0]};
	// printf("Reversed: \t");
	// print_received_buffer(reversed,4);
	memcpy(&res, &bin,sizeof(float));
	// printf("Result: \t");
	// print_received_buffer( (unsigned char *)&res, 4);
	return res;
}

unsigned short get_unsigned_short_val(unsigned char* buf) {
	unsigned char c0= buf[0], c1= buf[1];
	return (unsigned short) (c1<<8 | c0);
}

short get_short_val(unsigned char* buf) {
	unsigned char c0= buf[0], c1= buf[1];
	return (short) (c1<<8 | c0);
}

long long get_curr_time_in_milliseconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (long long) ( (tv.tv_sec * 1000) + (int)tv.tv_usec/1000 );
}

long get_curr_time_in_seconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

void print_PQ_Error(PQ_ERROR err) {
	switch (err) {
		case PQ_MEM_ERROR:
			printf("Memory allocation failed.\n");
			break;
		case PQ_INVALED_CONFIG_ERROR:
			printf("Error during creation / configuration of library\n");
			break;
		case PQ_HANDLE_ERROR:
			printf("Invalid handle given to functions. \n");
			break;
		case PQ_PROCESSING_ERROR:
			printf("Error during processing.\n");
			break;
		default:
			printf("Unknown Error: %d\n",err);
			break;
	}
}

void store_to_file(PQResult pq_result, struct powquty_conf *config) {
	char *line = malloc(sizeof(char) * 135);

	long long ts = get_curr_time_in_milliseconds();
	long ts_sec = get_curr_time_in_seconds();

	sprintf(line,
		"%s,%lld,%ld,3,%010.6f,%09.6f,%09.6f,%09.6f,%09.6f,%09.6f,"
		"%09.6f,%09.6f,%09.6f",
		"DEV_UUID",
		ts,
		ts_sec,
		pq_result.PowerVoltageEff_5060T,
		pq_result.PowerFrequency5060T,
		pq_result.Harmonics[0],
		pq_result.Harmonics[1],
		pq_result.Harmonics[2],
		pq_result.Harmonics[3],
		pq_result.Harmonics[4],
		pq_result.Harmonics[5],
		pq_result.Harmonics[6] );
	write_line_to_file(fcfg, line);
}

