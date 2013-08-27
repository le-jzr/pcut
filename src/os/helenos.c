/*
 * Copyright (c) 2013 Vojtech Horky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <task.h>
#include <fcntl.h>
#include "../helper.h"


/* String functions. */
/* String functions. */
int pcut_str_equals(const char *a, const char *b) {
	return str_cmp(a, b) == 0;
}


int pcut_str_start_equals(const char *a, const char *b, int len) {
	return str_lcmp(a, b, len) == 0;
}

int pcut_str_size(const char *s) {
	return str_size(s);
}

int pcut_str_to_int(const char *s) {
	int result = strtol(s, NULL, 10);
	return result;
}

char *pcut_str_find_char(const char *haystack, const char needle) {
	return str_chr(haystack, needle);
}


/* Forking-mode related functions. */

#define MAX_TEST_NUMBER_WIDTH 24
#define PCUT_TEMP_FILENAME_BUFFER_SIZE 128
#define MAX_COMMAND_LINE_LENGTH 1024
#define OUTPUT_BUFFER_SIZE 8192

static char error_message_buffer[OUTPUT_BUFFER_SIZE];
static char extra_output_buffer[OUTPUT_BUFFER_SIZE];

static void before_test_start(pcut_item_t *test) {
	pcut_report_test_start(test);

	memset(error_message_buffer, 0, OUTPUT_BUFFER_SIZE);
	memset(extra_output_buffer, 0, OUTPUT_BUFFER_SIZE);
}

void pcut_run_test_forking(const char *self_path, pcut_item_t *test) {
	before_test_start(test);

	char tempfile_name[PCUT_TEMP_FILENAME_BUFFER_SIZE];
	snprintf(tempfile_name, PCUT_TEMP_FILENAME_BUFFER_SIZE - 1, "pcut_%lld.tmp", (unsigned long long) task_get_id());
	int tempfile = open(tempfile_name, O_CREAT | O_RDWR);
	if (tempfile < 0) {
		pcut_report_test_done(test, TEST_OUTCOME_ERROR, "Failed to create temporary file.", NULL, NULL);
		return;
	}

	char test_number_argument[MAX_TEST_NUMBER_WIDTH];
	snprintf(test_number_argument, MAX_TEST_NUMBER_WIDTH, "-t%d", test->id);

	int *files[4];
	int fd_stdin = fileno(stdin);
	files[0] = &fd_stdin;
	files[1] = &tempfile;
	files[2] = &tempfile;
	files[3] = NULL;

	const char *const arguments[3] = {
		self_path,
		test_number_argument,
		NULL
	};

	int status = TEST_OUTCOME_PASS;

	task_id_t task_id;
	int rc = task_spawnvf(&task_id, self_path, arguments, files);
	if (rc != EOK) {
		status = TEST_OUTCOME_ERROR;
		goto leave_close_tempfile;
	}

	task_exit_t task_exit;
	int task_retval;
	rc = task_wait(task_id, &task_exit, &task_retval);
	if (rc != EOK) {
		status = TEST_OUTCOME_ERROR;
		goto leave_close_tempfile;
	}
	if (task_exit == TASK_EXIT_UNEXPECTED) {
		status = TEST_OUTCOME_ERROR;
	} else {
		status = task_retval == 0 ? TEST_OUTCOME_PASS : TEST_OUTCOME_FAIL;
	}

	read_all(tempfile, extra_output_buffer, OUTPUT_BUFFER_SIZE);

leave_close_tempfile:
	close(tempfile);
	unlink(tempfile_name);

	pcut_report_test_done_unparsed(test, status, extra_output_buffer, OUTPUT_BUFFER_SIZE);
}
