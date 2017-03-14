/*
 * storage.h
 *
 * Create on: Feb 14, 2017
 * 	Author: ikstream
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <unistd.h>
#include <sys/types.h>

#define MAX_PATH_LENGTH 512

/*
 * log file information
 * max_logsize: maximal size of log file in bytes
 * line_length: length of a individual line in log file
 * ts_pos: position of the timestamp in log file line
 * path: path to log file
 */
struct file_cfg {
	off_t max_logsize;
	ssize_t line_length;
	int ts_pos;
	char path[MAX_PATH_LENGTH];
};

/*
 * set max size for each log file
 * @file_cfg: set max size for this log file
 * @max_logsize: max size in kB has to be >0
 */
void set_max_logsize(struct file_cfg *fcfg, long max_logsize);

/*
 * set path for log file
 * @file_cfg: set path of this log file
 * @path: path to file
 * @return 0 on succes else 1
 */
int set_log_path(struct file_cfg *fcfg, char *path);

/*
 * set length of line in a log file
 * @file_cfg: set line length for this log file
 * @line_length:-1 -> 	no fix line length
 * 		0  -> 	not sure how long. check line length and set it,
 * 			if possible
 * 		>0 ->	set fix line length to this value without null byte
 * 			if a line is above this value truncate it
 * 			if a line is below this value pad it.
 */
void set_line_length(struct file_cfg *fcfg, ssize_t line_length);

/*
 * set timestamp position in a log file line
 * @fcfg: use the position in this log file
 * @ts_pos: entry number in log file line, has to be >= 1
 */
void set_timestamp_position(struct file_cfg *fcfg, int ts_pos);

/*
 * get entry in csv line
 * @line: comma separated line, to parse
 * @entry: position in line
 * @delim: delimiter for entries
 * return: token if found, else NULL
 */
char * get_entry_from_line_position(char* line, int entry, char *delim);

/*
 * write log line to correct position in log file
 * @file_cfg: log file config struct
 * @line: write this line to log file
 * return: 0 on success, else 1
 */
int write_line_to_file(struct file_cfg *fcfg, char *line);

#endif

