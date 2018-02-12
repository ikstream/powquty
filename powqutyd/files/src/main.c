/*
 * main.c
 *
 *  Created on: Jun 16, 2016
 *      Author: neez
 */
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PQ_App.h"
#include "calculation.h"
#include "file_handling.h"
#include "retrieval.h"
#include "config.h"
#include "uci_config.h"

static volatile int stop_main = 0;

void handle_signal()
{
	stop_calculation();
	printf("DEBUG:\tThreads should have stopped\n");
	stop_main = 1;
}

void stop_powqutyd() {
	handle_signal();
}

void stop_powqutyd_file_read() {
	stop_file_read();
	printf("DEBUG:\tThreads should have stopped\n");
	stop_main = 1;
}

void handle_args (int argc, char **argv) {
	int c;
	while ((c = getopt (argc, argv, "rdf:")) != -1) {
		switch (c) {
			case 'r':
				set_raw_print(1);
				break;
			case 'd':
				set_debug(1);
				break;
			case 'f':
				set_file_read(optarg);
				break;
			default:
				break;
		}
	}
}

int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	// char* config_file = "/etc/powqutyd/powqutyd.cfg";

	/*if(load_config(config_file)){
		printf("Error: could not load some config from %s\n", config_file);
		// return -1;
	}*/

	struct powquty_conf conf;
	uci_config_powquty(&conf);
	//printf("UCI CONFIG FTW!!!");

	// PQ_ERROR err = PQ_NO_ERROR;

	printf("Starting powqutyd ...\n");
	handle_args(argc, argv);

	printf("arguments handled\n");
	if (get_input_file_state()) {
		printf("use input file\n");
		while (!stop_main) {
			join_file_read();
		}
		return 0;
	}
	printf("not input_file\n");
	if(!calculation_init(&conf)) {
		printf("Calculation Thread started\n");
	} else {
		exit(EXIT_FAILURE);
	}


	while (!stop_main){
		join_calculation();

	}


	return 0;
}

