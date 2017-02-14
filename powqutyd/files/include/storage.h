/*
 * storage.h
 *
 * Create on: Feb 14, 2017
 * 	Author: ikstream
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "uci_config.h"
#include "PQ_App.h"

void store_to_file(PQResult pq_result, struct powquty_conf *config);

#endif
