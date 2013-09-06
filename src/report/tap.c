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

#include "../internal.h"
#include "report.h"
#ifndef __helenos__
#include <string.h>
#endif

static int test_counter;
static int tests_in_suite;
static int failed_tests_in_suite;

static void tap_init(pcut_item_t *all_items) {
	int tests_total = pcut_count_tests(all_items);
	test_counter = 0;

	printf("1..%d\n", tests_total);
}

static void tap_suite_start(pcut_item_t *suite) {
	tests_in_suite = 0;
	failed_tests_in_suite = 0;

	printf("#> Starting suite %s.\n", suite->suite.name);
}

static void tap_suite_done(pcut_item_t *suite) {
	printf("#> Finished suite %s (failed %d of %d).\n",
			suite->suite.name, failed_tests_in_suite, tests_in_suite);
}

static void tap_test_start(pcut_item_t *test) {
	PCUT_UNUSED(test);

	tests_in_suite++;
	test_counter++;
}

static void print_by_lines(const char *message, const char *prefix) {
	if ((message == NULL) || (message[0] == 0)) {
		return;
	}
	char *next_line_start = pcut_str_find_char(message, '\n');
	while (next_line_start != NULL) {
		next_line_start[0] = 0;
		printf("%s%s\n", prefix, message);
		message = next_line_start + 1;
		next_line_start = pcut_str_find_char(message, '\n');
	}
	if (message[0] != 0) {
		printf("%s%s\n", prefix, message);
	}
}

static void tap_test_done(pcut_item_t *test, int outcome,
		const char *error_message, const char *teardown_error_message,
		const char *extra_output) {
	const char *test_name = test->test.name;

	if (outcome != TEST_OUTCOME_PASS) {
		failed_tests_in_suite++;
	}

	const char *status_str = NULL;
	const char *fail_error_str = NULL;
	switch (outcome) {
	case TEST_OUTCOME_PASS:
		status_str = "ok";
		fail_error_str = "";
		break;
	case TEST_OUTCOME_FAIL:
		status_str = "not ok";
		fail_error_str = " failed";
		break;
	case TEST_OUTCOME_ERROR:
		status_str = "not ok";
		fail_error_str = " aborted";
		break;
	default:
		/* Shall not get here. */
		break;
	}
	printf("%s %d %s%s\n", status_str, test_counter, test_name, fail_error_str);

	print_by_lines(error_message, "# error: ");
	print_by_lines(teardown_error_message, "# error: ");

	print_by_lines(extra_output, "# stdio: ");
}

static void tap_done() {
}


pcut_report_ops_t pcut_report_tap = {
	.init = tap_init,
	.done = tap_done,
	.suite_start = tap_suite_start,
	.suite_done = tap_suite_done,
	.test_start = tap_test_start,
	.test_done = tap_test_done
};