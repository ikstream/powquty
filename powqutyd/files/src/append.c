#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "append.h"

#define TERM_CHAR 1
#define DELIM_CHAR 1
#define STATUS_APPEND 2

//TODO: return or exit on ret < 0

char * append_int(char *line, int entry) {
	size_t ret, needed, entry_len;
	char *new_line = malloc((strlen(line) * sizeof(char)) + TERM_CHAR);
	if (new_line == NULL)
		exit(ENOMEM);

	strncpy(new_line, line,strlen(line) * sizeof(char));
	char entry_buffer[snprintf(NULL, 0, "%d", entry) + TERM_CHAR];
	entry_len = snprintf(NULL, 0, "%d", entry) + TERM_CHAR;

	ret = snprintf(entry_buffer, entry_len, "%d", entry);
	if( ret < 0)
		printf("error in write\n");

	needed = (strlen(new_line) * sizeof(char)) + TERM_CHAR +
		 (strlen(entry_buffer) * sizeof(char)) + TERM_CHAR +
		  DELIM_CHAR;

	new_line = realloc(new_line, needed);
	if (new_line == NULL)
		exit(ENOMEM);

	strncat(new_line,entry_buffer, entry_len);
	strncat(new_line, ",", DELIM_CHAR);

	return new_line;
}

char * append_float(char *line, float entry) {
	size_t ret, needed, entry_len;
	char *new_line = malloc((strlen(line) * sizeof(char)) + TERM_CHAR);
	if (new_line == NULL)
		exit(ENOMEM);

	strncpy(new_line, line, strlen(line) * sizeof(char));
	char entry_buffer[snprintf(NULL, 0, "%f", entry) + TERM_CHAR];
	entry_len = snprintf(NULL, 0, "%f", entry) + TERM_CHAR;
	ret = snprintf(entry_buffer, entry_len, "%f", entry);
	if (ret < 0)
		printf("error on write\n"); //TODO: return or exit

	needed = (strlen(new_line) * sizeof(char)) + TERM_CHAR +
			(strlen(entry_buffer) * sizeof(char)) + TERM_CHAR +
			 DELIM_CHAR;
	new_line = realloc(new_line, needed);
	if (new_line == NULL)
		exit(ENOMEM);

	strncat(new_line, entry_buffer, entry_len);
	strncat(new_line, ",", DELIM_CHAR);

	return new_line;
}

char * append_string(char *line, char *entry) {
	size_t ret, needed, entry_len;
	char *new_line = malloc((strlen(line) * sizeof(char)) + TERM_CHAR);
	if (new_line == NULL)
		exit(ENOMEM);

	strncpy(new_line, line, strlen(line) * sizeof(char));
	char entry_buffer[snprintf(NULL, 0, "%s", entry)];
	entry_len = snprintf(NULL, 0, "%s", entry);
	ret = snprintf(entry_buffer, entry_len, "%s", entry);
	if (ret < 0)
		printf("error on write\n"); //TODO: return or exit

	needed = (strlen(new_line) * sizeof(char)) + TERM_CHAR +
			(strlen(entry_buffer) * sizeof(char)) + TERM_CHAR +
			 DELIM_CHAR;
	new_line = realloc(new_line, needed);
	if (new_line == NULL)
		exit(ENOMEM);

	strncat(new_line, entry_buffer, entry_len);
	strncat(new_line, ",", DELIM_CHAR);

	return new_line;
}

char * end_line(char *line) {
	size_t ret, line_length;
	line_length = snprintf(NULL, "0", "%s", line);
	if (line[line_length - 2] == ',')
		line_length[line_length - 2] = '\0';
}

char * truncate_line(char *line, ssize_t max_length) {
	printf("truncated line to max length of: %ld\n", (long)max_length);
}

int append_status(char *line, int status) {
	size_t ret;
	char *new_line = malloc((strlen(line) + TERM_CHAR + DELIM_CHAR +
				STATUS_APPEND + TERM_CHAR) * sizeof(char));
	if (status >= 1)
		ret = strncat(line, ",PA", DELIM_CHAR + STATUS_APPEND);
	else if (status == 0)
		ret = strncat(line, ",OK", DELIM_CHAR + STATUS_APPEND);
	else
		ret = strncat(line, ",TR", DELIM_CHAR + STATUS_APPEND);

	if (ret < 0)
		return 1;

	return 0;
}

