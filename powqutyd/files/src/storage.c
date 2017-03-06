/*
 * storage.c
 *
 * Created on: Feb 14, 2017
 * 	Author: ikstream
 */

#include <sys/stat.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage.h"

#define KB_TO_BYTE 1024
#define TIME_STAMP_POSITION 2
#define TERM_CHAR 1
#define DELIM_CHAR 1
#define APPEND 2

//TODO: move TIME_STAMP_POSITION to struct
//TODO: provide function to set timestamp position
//TODO: remove store_to_file code when rest is implemented
//TODO: check documentation and comments if still correct

int file_is_unchecked = 1;
long cur_offset;

void set_max_logsize(struct file_cfg *fcfg, long max_logsize) {
	fcfg->max_logsize = (off_t)(max_logsize * KB_TO_BYTE);
}

int set_log_path(struct file_cfg *fcfg, char *path) {
	size_t len = strlen(path) + TERM_CHAR;

	if (len > MAX_PATH_LENGTH)
		return EXIT_FAILURE;

	if(strncpy(fcfg->path, path, len) == NULL);
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

void set_line_length(struct file_cfg *fcfg, signed long line_length) {
	fcfg->line_length = (ssize_t)line_length;
}

char * get_entry_from_line_position(char* line, int entry, char *delim) {
	char* token;
	for (token = strtok(line, delim); token && *token;
	     token = strtok(NULL, ",\n")) {
		if (!--entry)
			return token;
	}
	return NULL;
}


/*
 * get the last line of a file
 * @file: file to get line from
 * @char_count: offset(number of characters) to get the start of line
 * return: complete last line of file
 */
char * get_last_line(FILE *file, ssize_t char_count) {
	char *line = NULL;
	ssize_t read;
	size_t len = 0;

	fseek(file, -char_count,SEEK_END);
	read = getline(&line, &len, file);
	if (read == -1) {
		printf("Could not get last line\n");
		exit(EXIT_FAILURE);
	}
	return line;
}

/*
 * get the number of characters in a regular line in file
 * this assumes, that all lines have the same character count
 * @file: file to check for line length
 * return: returns character count of first line
 */
ssize_t get_character_count_per_line(FILE *file) {
	char *line = NULL;
	size_t len = 0;
	ssize_t char_count;

	fseek(file, 0, SEEK_SET);
	char_count = getline(&line, &len, file);
	if (char_count == -1) {
		printf("Error in line read: Could not get number of characters"
			" in first line\n");
		exit(EXIT_FAILURE);
	}
	fseek(file, 0, SEEK_SET);

	return char_count;
}

/*
 * checks if the log file can be rewritten from start, or has to be resumed
 * @file: file to check
 * @char_count: number of chars in a line
 * return 0 if file write has to be resumed, 1 if the first entry is the oldest
 */
int is_outdated(FILE *file, ssize_t char_count) {
	long first_time, last_time;
	char *line =NULL;
	char *last_line = malloc((sizeof(char) * char_count) + 1);
	size_t len = 0;

	fseek(file, 0, SEEK_SET);
	getline(&line, &len, file);
	first_time = atol(get_entry_from_line_position(line,
						       TIME_STAMP_POSITION,
						       ","));

	memcpy(last_line,get_last_line(file,char_count), char_count);
	last_line[char_count] = '\0';
	last_time = atol(get_entry_from_line_position(last_line,
						      TIME_STAMP_POSITION,
						      ","));

	if (last_time > first_time)
		return 1;

	return 0;
}

/*
 * check if a file is above a given limit
 * @file: file to check
 * @max_size: maximal size of file in kB
 * return: returns 1 if file is above the limit, else 0
 */
int has_max_size(char *powquty_path, off_t max_size) {
	struct stat st;
	off_t filesize;

	max_size *= KB_TO_BYTE;

	if (access(powquty_path, F_OK ))
		return 0;

	if (stat(powquty_path, &st) == 0) {
		filesize = st.st_size;
	} else {
		printf("Could not get filesize\n");
		exit(2);
	}

	if (filesize > max_size)
		return 1;

	return 0;
}

/*
 * calculate real number of a line(starting with 1)
 * @offset: offset of line in file
 * @char_count number of characters in a line
 * return the line number
 */
long get_line_number(long offset, ssize_t char_count) {
	long line_number = (offset / char_count) + 1;
	return line_number;
}

/*
 * get an Entry(timestamp) from a line
 * the position of the line has to be set before calling this function
 * @file file to read from
 * return timestam as integer
 */
int get_line_entry(FILE *file) {
	char *line = NULL;
	size_t len = 0;
	long val;

	getline(&line, &len, file);
	val = atol(get_entry_from_line_position(line, TIME_STAMP_POSITION,","));
	return val;
}

/*
 * set the position to resume write operations after powqutyd stopped
 * uses logarithmic search to get the latest timestamp
 * @file: file to write to
 * @u_bound upper boundary for interval
 * @l_bound lower boundary for latest timestamp search
 * @char_count: length of line to calculate offset
 */
void set_position(FILE *file, long u_bound, long l_bound, ssize_t char_count) {
	long u_offset, m_offset, l_offset;
	long u_line_number, m_line_number, l_line_number;
	int u_val, m_val, l_val;

	fseek(file, u_bound, SEEK_SET);
	u_offset = ftell(file);
	u_val = get_line_entry(file);
	u_line_number = get_line_number(u_offset, char_count);

	fseek(file, l_bound, SEEK_SET);
	l_offset = ftell(file);
	l_val = get_line_entry(file);
	l_line_number = get_line_number(l_offset, char_count);

	m_line_number = ((l_line_number + u_line_number) / 2);
	m_offset = ((m_line_number * char_count) - char_count);
	fseek(file, m_offset, SEEK_SET);
	m_val = get_line_entry(file);

	if ((m_val > l_val) && (m_val > u_val))
		set_position(file, m_offset, l_offset, char_count);
	if ((m_val < l_val) && (m_val < u_val))
		set_position(file, u_offset, m_offset, char_count);
	if ((m_val == u_val) || (m_val == l_val))
		fseek(file, m_offset + char_count, SEEK_SET);
}

/*
 * check if length of line is greater than line_length
 * if it is longer, truncate and append ,TR
 * if it is shorter pad and append ,PA
 * else append ,OK
 * and print to file
 * @file: write to this file
 * @line: write this line to file
 */
int check_and_print(FILE *file, struct file_cfg *fcfg, char * line) {
	size_t char_count = strlen(line) * sizeof(char);
	size_t final_len = fcfg->line_length + DELIM_CHAR + APPEND + TERM_CHAR;
	int i, chars_written;
	char final_line[final_len];

	strncpy(final_line, line, fcfg->line_length);
	if (char_count > fcfg->line_length) {
		strncat(final_line, ",TR", final_len);
	} else if (char_count < fcfg->line_length) {
		for (i = char_count; i < fcfg->line_length; i++)
			if (final_line[i] == '\0')
				final_line[i] = ' ';
		strncat(final_line, ",PA", final_len);
	} else {
		strncat(final_line, ",OK", final_len);
	}

	return fprintf(file, "%s", final_line);
}

//TODO: check return values of functions called
/*
 * Seek position to resume log write
 * This will be used if no fix line length is given
 * @file: look at this file
 * return: 0 if a position is found, on error return >=1
 */
int seek_position(FILE *file) {
	size_t len = 0;
	off_t eof, cur_offset = 0;
	char *line = NULL;
	ssize_t line_length;
	long timestamp, cur_timestamp;

	/* iterate over file and compare timestamps
	 * if never timestamp is found set file position to line start
	 */
	fseek(file, 0, SEEK_END);
	eof = ftell(file);

	while (cur_offset < eof) {
		fseek(file, cur_offset, SEEK_SET);
		line_length = getline(&line, &len, file);
		if (line_length < 0)
			return 1;
		cur_offset += (off_t)line_length;
		timestamp = atol(get_entry_from_line_position(line,
			    TIME_STAMP_POSITION, ","));

		len = 0;
		line = NULL;
		line_length = getline(&line, &len, file);
		if (line_length < 0)
			return 1;
		cur_timestamp = atol(get_entry_from_line_position(line,
			    TIME_STAMP_POSITION, ","));

		if (cur_timestamp < timestamp) {
			fseek(file, (long)line_length, SEEK_CUR);
			return 0;
		}
	}
	return 1;
}

//TODO: check return values
int write_line_to_file(struct file_cfg *fcfg, char *line) {
	FILE *file;
	ssize_t char_count;
	long lower_bound, upper_bound;
	int no_fix_line_length = 0;
	int ret;

	if (!has_max_size(fcfg->path, fcfg->max_logsize)) {
		file = fopen(fcfg->path, "a");
		if (file == NULL)
			exit(EXIT_FAILURE);
	} else {
		file = fopen(fcfg->path, "r+");

		if (fcfg->line_length > 0)
			char_count = fcfg->line_length;
		else if (fcfg->line_length == 0)
			char_count = get_character_count_per_line(file);
		else
			no_fix_line_length = 1;

		if (no_fix_line_length) {
			ret = seek_position(file);
			if (!ret) {
				fprintf(file, "%s", line);
				return ret;
			} else {
				return ret;
			}
		}

		fseek(file, -char_count, SEEK_END);
		lower_bound = ftell(file);
		if (file_is_unchecked) {
			file_is_unchecked = 0;

			if (is_outdated(file,char_count)) {
				fseek(file, 0, SEEK_SET);
				cur_offset = 0;
			} else {
				fseek(file, 0, SEEK_SET);
				upper_bound = ftell(file);
				set_position(file,upper_bound,lower_bound,
					     char_count);
				cur_offset = ftell(file);
				if (cur_offset == lower_bound)
					file_is_unchecked = 1;
			}
		} else {
			cur_offset += (long)get_character_count_per_line(file);
			fseek(file, cur_offset, SEEK_SET);
			if (cur_offset == lower_bound)
				file_is_unchecked = 1;
		}
	}

	/* fcfg == 0 should not be possible at this position */
	if (fcfg->line_length >= 0)
		check_and_print(file, fcfg, line);
	else
		fprintf(file, "%s", line);

	fclose(file);

	return 0;

}

/*
 * For a correct file write all line must have the same number of characters.
 */
/*
void store_to_file(PQResult pq_result, struct powquty_conf *config) {
	FILE* pf;
	ssize_t char_count;
	long lower_bound, upper_bound;

	if (!has_max_size(config->powquty_path, (off_t)config->max_log_size_kb)) {
		pf = fopen(config->powquty_path,"a");
		if (pf == NULL)
			exit(EXIT_FAILURE);
	} else {
		pf = fopen(config->powquty_path, "r+");
		char_count = get_character_count_per_line(pf);
		fseek(pf, -char_count, SEEK_END);
		lower_bound = ftell(pf);
		if (file_is_unchecked) {
			file_is_unchecked = 0;

			if (is_outdated(pf,char_count)) {
				fseek(pf, 0, SEEK_SET);
				cur_offset = 0;
			} else {
				fseek(pf, 0, SEEK_SET);
				upper_bound = ftell(pf);
				set_position(pf,upper_bound,lower_bound,
					     char_count);
				cur_offset = ftell(pf);
				if (cur_offset == lower_bound)
					file_is_unchecked = 1;
			}
		} else {
			cur_offset += (long)get_character_count_per_line(pf);
			fseek(pf, cur_offset, SEEK_SET);
			if (cur_offset == lower_bound)
				file_is_unchecked = 1;
		}
	}
	long long ts = get_curr_time_in_milliseconds();
	long ts_sec = get_curr_time_in_seconds();
	fprintf(pf,
			"%s,%ld,%lld,3,%010.6f,%09.6f,%09.6f,%09.6f,%09.6f,%09.6f,"
			"%09.6f,%09.6f,%09.6f\n",
			"DEV_UUID",
			ts_sec,
			ts,
			pq_result.PowerVoltageEff_5060T,
			pq_result.PowerFrequency5060T,
			pq_result.Harmonics[0],
			pq_result.Harmonics[1],
			pq_result.Harmonics[2],
			pq_result.Harmonics[3],
			pq_result.Harmonics[4],
			pq_result.Harmonics[5],
			pq_result.Harmonics[6] );
	fclose(pf);
}
*/

