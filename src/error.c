/**
 * @file error.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Contains implementation of error and warning functions for central error handling.
 * Includes mechanisms for safely terminating the program.
 * @date 2026-10-04
 */

#include "error.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void throw_err(err_c_t code, int line_num, const char *fmt, ...) {
	fprintf(stderr, "Error %d", code);

	if (line_num > 0) {
		fprintf(stderr, " on line %d", line_num);
	}

	fprintf(stderr, ": ");

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args); // NOLINT(clang-analyzer-security.VAList)
	va_end(args);

	fputc('\n', stderr);
	exit(code);
}

void throw_warning(int line_num, const char *fmt, ...) {
	fprintf(stderr, "Warning");

	if (line_num > 0) {
		fprintf(stderr, " on line %d", line_num);
	}

	fprintf(stderr, ": ");

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args); // NOLINT(clang-analyzer-security.VAList)
	va_end(args);

	fputc('\n', stderr);
}
