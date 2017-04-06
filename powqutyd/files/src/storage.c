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
#include <errno.h>
#include "storage.h"

#define KB_TO_BYTE 1024
#define TERM_CHAR 1
#define DELIM_CHAR 1
#define APPEND 2

int file_is_unchecked = 1;
long cur_offset;

void set_max_logsize(struct file_cfg *fcfg, long max_logsize) {
	fcfg->max_logsize = (off_t)(max_logsize * KB_TO_BYTE);
	if (fcfg->max_logsize <= 0) {
		printf("max log size has to be > 0 but is %ld\n", max_logsize);
		exit(EXIT_FAILURE);
	}
}

int set_log_path(struct file_cfg *fcfg, char *path) {
	size_t len = strlen(path) + TERM_CHAR;

	if (len > MAX_PATH_LENGTH)
		return EXIT_FAILURE;

	if(strncpy(fcfg->path, path, len) == NULL);
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

void set_line_length(struct file_cfg *fcfg, ssize_t line_length) {
	fcfg->line_length = line_length;
}

void set_timestamp_position(struct file_cfg *fcfg, int ts_pos) {
	fcfg->ts_pos = ts_pos;
	if (ts_pos < 1) {
		printf("position of timestamp has to be >=1 but is %d\n",ts_pos);
		exit(EXIT_FAILURE);
	}
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

	if (fseek(file, - char_count, SEEK_END)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	read = getline(&line, &len, file);
	if (read == -1) {
		printf("getline failed in %s: %s\n", __func__, strerror(errno));
		free(line);
		exit(errno);
	}

	return line;
}

/*
 * check if all lines in a file are of the same length and set line_length
 * if there are two different line length set line_length to -1 else
 * set it accordingly
 * @fcfg: file config struct
 * @file: file to check
 * return -1 on different line length, else number of chars without
 * 	teminating char
 */
ssize_t check_lines_for_length(struct file_cfg *fcfg, FILE *file) {
	char *line = NULL;
	size_t len = 0;
	off_t eof, cur_pos;
	ssize_t char_count, cur_char_count;

	if (fseek(file, 0, SEEK_END)) {
		printf("fseek failed in %s with %s\n", __func__, strerror(errno));
		exit(errno);
	}
	eof = ftell(file);

	if (fseek(file, 0, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	char_count = getline(&line, &len, file);
	if (char_count == -1) {
		free(line);
		printf("getline failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	/* check every line in log file */
	cur_pos = ftell(file);
	while(cur_pos != eof) {
		free(line);
		len = 0;
		cur_char_count = getline(&line, &len, file);
		if (cur_char_count != char_count) {
			fcfg->line_length = -1;
			printf("Found two different line length\n");
			free(line);
			return -1;
		}
		cur_pos = ftell(file);
	}
	fcfg->line_length = char_count;
	return char_count;
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

	if (fseek(file, 0, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char_count = getline(&line, &len, file);
	if (char_count == -1) {
		printf("getline failed in %s: %s\n", __func__, strerror(errno));
		free(line);
		exit(EXIT_FAILURE);
	}

	if (fseek(file, 0, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return char_count;
}

/*
 * checks if the log file can be rewritten from start, or has to be resumed
 * @fcfg: log file information struct
 * @file: file to check
 * @char_count: number of chars in a line
 * return 0 if file write has to be resumed, 1 if the first entry is the oldest
 */
int is_outdated(struct file_cfg *fcfg, FILE *file, ssize_t char_count) {
	long first_time, last_time;
	char *line =NULL;
	char *last_line = malloc((sizeof(char) * char_count) + 1);
	size_t len = 0;

	if (last_line == NULL) {
		printf("Couldn't allocate memory in %s: %s\n", __func__,
							      strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fseek(file, 0, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (getline(&line, &len, file) == -1) {
		printf("getline failed in %s: %s\n", __func__, strerror(errno));
		free(line);
		exit(EXIT_FAILURE);
	}

	first_time = atol(get_entry_from_line_position(line,
						       fcfg->ts_pos,
						       ","));

	memcpy(last_line,get_last_line(file,char_count), char_count);
	last_line[char_count] = '\0';
	last_time = atol(get_entry_from_line_position(last_line,
						      fcfg->ts_pos,
						      ","));

	free(last_line);
	if (last_time > first_time)
		return 1;

	return 0;
}

/*
 * check if a file is above a given limit
 * @fcfg: log file configuration struct
 * return: returns 1 if file is above the limit, else 0
 */
int has_max_size(struct file_cfg *fcfg) {
	struct stat st;
	off_t filesize;

	if (access(fcfg->path, F_OK ))
		return 0;

	if (stat(fcfg->path, &st) == 0) {
		filesize = st.st_size;
	} else {
		printf("stat failed in %s: %s\n", __func__ ,strerror(errno));
		exit(errno);
	}

	if (filesize > fcfg->max_logsize)
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
 * @fcfg: log file inforamtion struct
 * @file file to read from
 * return timestam as integer
 */
int get_line_entry(struct file_cfg *fcfg, FILE *file) {
	char *line = NULL;
	size_t len = 0;
	long val;

	if (getline(&line, &len, file) == -1) {
		printf("getline failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	val = atol(get_entry_from_line_position(line, fcfg->ts_pos, ","));
	return val;
}

/*
 * set the position to resume write operations after powqutyd stopped
 * uses logarithmic search to get the latest timestamp
 * @fcfg: log file information struct
 * @file: file to write to
 * @u_bound upper boundary for interval
 * @l_bound lower boundary for latest timestamp search
 * @char_count: length of line to calculate offset
 */
void set_position(struct file_cfg *fcfg, FILE *file, long u_bound, long l_bound,
		  ssize_t char_count) {

	long u_offset, m_offset, l_offset;
	long u_line_number, m_line_number, l_line_number;
	int u_val, m_val, l_val;

	if (fseek(file, u_bound, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	u_offset = ftell(file);
	if (u_offset == -1) {
		printf("ftell failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	u_val = get_line_entry(fcfg, file);
	u_line_number = get_line_number(u_offset, char_count);

	if (fseek(file, l_bound, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	l_offset = ftell(file);
	if (l_offset == -1) {
		printf("ftell failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	l_val = get_line_entry(fcfg, file);
	l_line_number = get_line_number(l_offset, char_count);

	m_line_number = ((l_line_number + u_line_number) / 2);
	m_offset = ((m_line_number * char_count) - char_count);

	if (fseek(file, m_offset, SEEK_SET)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	m_val = get_line_entry(fcfg, file);

	/* set boundaries for recursive function call */
	if ((m_val > l_val) && (m_val > u_val))
		set_position(fcfg, file, m_offset, l_offset, char_count);
	if ((m_val < l_val) && (m_val < u_val))
		set_position(fcfg, file, u_offset, m_offset, char_count);
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
 * @fcfg: log file information struct
 * @line: write this line to file
 * return number of characters printed, 0 on error
 */
int check_and_print(FILE *file, struct file_cfg *fcfg, char * line) {
	size_t char_count = strlen(line) * sizeof(char);
	size_t final_len = fcfg->line_length + (sizeof(char) * (
				DELIM_CHAR + APPEND + TERM_CHAR));
	int i;
	char final_line[final_len];

	if (strncpy(final_line, line, fcfg->line_length) == NULL) {
		printf("strncpy failed in %s\n", __func__);
		return 0;
	}

	if (final_line[final_len - 1] == '\n')
		final_line[final_len - 1] = '\0';

	/* Append status to line */
	if (char_count > fcfg->line_length) {
		final_line[fcfg->line_length] = '\0';
		if (strncat(final_line, ",TR\n", final_len) == NULL) {
			printf("strncat failed(TR) in %s\n", __func__);
			return 0;
		}
	} else if (char_count < fcfg->line_length) {
		for (i = char_count - 1; i < fcfg->line_length; i++) {
			if ((final_line[i] == '\0') || (final_line[i] == '\n')){
				final_line[i] = ' ';
			}
			final_line[fcfg->line_length] = '\0';
		}
		if (strncat(final_line, ",PA\n", final_len) == NULL) {
			printf("strncat failed(PA) in %s\n", __func__);
			return 0;
		}
	} else {
		final_line[fcfg->line_length] = '\0';
		if (strncat(final_line, ",OK\n", final_len) == NULL) {
			printf("strncat failed(OK) in %s\n", __func__);
			return 0;
		}
	}

	return fprintf(file, "%s", final_line);
}

/*
 * Seek position to resume log write
 * This will be used if no fix line length is given
 * @fcfg: log file information struct
 * @file: look at this file
 * return: 0 if a position is found, on error return 1
 */
int seek_position(struct file_cfg *fcfg, FILE *file) {
	size_t len = 0;
	off_t eof, cur_offset = 0;
	char *line = NULL;
	ssize_t line_length;
	long timestamp, cur_timestamp;

	if (fseek(file, 0, SEEK_END)) {
		printf("fseek failed in %s: %s\n", __func__, strerror(errno));
		return errno;
	}

	eof = ftell(file);
	if (eof == -1) {
		printf("ftell failed in %s: %s\n", __func__, strerror(errno));
		return errno;
	}

	/* iterate over file and compare timestamps
	 * if never timestamp is found set file position to line start */
	while (cur_offset < eof) {
		if (fseek(file, cur_offset, SEEK_SET)) {
			printf("fseek failed in %s: %s\n", __func__,
							  strerror(errno));
			return errno;
		}

		line_length = getline(&line, &len, file);
		if (line_length < 0) {
			printf("getline failed in %s: %s\n", __func__,
							    strerror(errno));
			free(line);
			return errno;
		}
		cur_offset += (off_t)line_length;
		timestamp = atol(get_entry_from_line_position(line,
			    fcfg->ts_pos, ","));

		len = 0;
		free(line);
		line = NULL;
		line_length = getline(&line, &len, file);
		if (line_length < 0) {
			printf("getline failed in %s: %s\n", __func__,
							    strerror(errno));
			free(line);
			return errno;
		}
		cur_timestamp = atol(get_entry_from_line_position(line,
			    fcfg->ts_pos, ","));

		if (cur_timestamp < timestamp) {
			if (fseek(file, (long)line_length, SEEK_CUR)) {
				printf("fseek failed in %s: %s\n", __func__,
							strerror(errno));
				return errno;
			}
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

int write_line_to_file(struct file_cfg *fcfg, char *line) {
	FILE *file;
	ssize_t char_count;
	long lower_bound, upper_bound;
	int no_fix_line_length = 0;
	int ret;

	/* check for max size */
	if (!has_max_size(fcfg)) {
		file = fopen(fcfg->path, "a");
		if (file == NULL) {
			printf("fopen failed in %s: %s\n", __func__,
							  strerror(errno));
			return errno;
		}
	} else {
		file = fopen(fcfg->path, "r+");
		if (file == NULL) {
			printf("fopen failed in %s: %s\n", __func__,
							  strerror(errno));
			return errno;
		}

		/* check line length */
		if (fcfg->line_length > 0) {
			char_count = fcfg->line_length + (sizeof(char) *
					(DELIM_CHAR + APPEND +
					 TERM_CHAR));
		} else if (fcfg->line_length == 0) {
			char_count = check_lines_for_length(fcfg, file);
			fcfg->line_length = char_count;
			if (char_count < 0)
				no_fix_line_length = 1;
		} else {
			no_fix_line_length = 1;
		}

		if (no_fix_line_length) {
			ret = seek_position(fcfg, file);
			if (!ret) {
				fprintf(file, "%s", line);
				fclose(file);
				return ret;
			} else {
				fclose(file);
				return ret;
			}
		}

		if (fseek(file, -char_count, SEEK_END)) {
			printf("fseek failed in %s: %s\n", __func__,
							  strerror(errno));
			fclose(file);
			return errno;
		}

		/* first time file is checked for write position */
		lower_bound = ftell(file);
		if (file_is_unchecked) {
			file_is_unchecked = 0;

			/* check if first time stamp is oldest */
			if (is_outdated(fcfg, file,char_count)) {
				if (fseek(file, 0, SEEK_SET)) {
					printf("fseek failed in %s: %s\n",
						__func__, strerror(errno));
					return errno;
				}
				cur_offset = 0;
			} else {
				if (fseek(file, 0, SEEK_SET)) {
					printf("fseek failed in %s: %s\n",
						__func__, strerror(errno));
					if (fclose(file)) {
						printf("fclose failed: %s: %s\n",
							__func__, strerror(errno));
						exit(errno);
					}
					return errno;
				}
				upper_bound = ftell(file);
				if (upper_bound == -1) {
					printf("ftell failed in %s: %s\n",
						__func__, strerror(errno));
					if (fclose(file)) {
						printf("fclose failed: %s :%s\n",
							__func__, strerror(errno));
						exit(errno);
					}
					return errno;
				}
				set_position(fcfg, file,upper_bound,lower_bound,
					     char_count);
				cur_offset = ftell(file);
				if (cur_offset == -1) {
					printf("ftell failed in %s: %s\n",
						__func__, strerror(errno));
					if (fclose(file)) {
						printf("fclose failed: %s:%s\n",
							__func__,strerror(errno));
						exit(errno);
					}
					return errno;
				}
				if (cur_offset == lower_bound)
					file_is_unchecked = 1;
			}
		} else {
			cur_offset += (long)get_character_count_per_line(file);
			if (fseek(file, cur_offset, SEEK_SET)) {
				printf("fseek failed in %s: %s\n", __func__,
							strerror(errno));
				if (fclose(file)) {
					printf("fclose failed: %s: %s\n",
						__func__, strerror(errno));
					exit(errno);
				}
				return errno;
			}
			if (cur_offset == lower_bound)
				file_is_unchecked = 1;
		}
	}

	/* process line if we know its length, else just print it to file */
	if (fcfg->line_length > 0) {
		if (check_and_print(file, fcfg, line))
			if (fclose(file)) {
				printf("fclose failed: %s: %s\n", __func__,
						strerror(errno));
				exit(errno);
			}
			return EXIT_FAILURE;
	} else {
		fprintf(file, "%s\n", line);
	}

	if (fclose(file)) {
		printf("fclose failed: %s: %s\n", __func__, strerror(errno));
		exit(errno);
	}

	return EXIT_SUCCESS;
}

