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

static err_c_t error_code;

/**
 * @brief Prints a formatted error message using va_list and saves the first error code
 *
 * @param[in] code Error code to be stored for exit
 * @param[in] line_num Number of the line where the error is located, 0 will not print line num
 * @param[in] fmt Formatted error string to be printed to stderr
 * @param[in] args Variable argument list
 */
void vprint_error(err_c_t code, unsigned line_num, const char *fmt, va_list args);

void error_exit(void) {
	exit(error_code != ERR_OK ? error_code
							  : ERR_INTERNAL); // ternary is saftey fuse, if somebody called
											   // error_exit without sending error message
}

void throw_err(err_c_t code, unsigned line_num, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprint_error(code, line_num, fmt, args);
	va_end(args);

	error_exit();
}

void throw_warning(unsigned line_num, const char *fmt, ...) {
	fprintf(stderr, "Warning");

	if (line_num > 0) {
		fprintf(stderr, " on line %u", line_num);
	}

	fprintf(stderr, ": ");

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args); // NOLINT(clang-analyzer-security.VAList)
	va_end(args);

	fputc('\n', stderr);
}

void vprint_error(err_c_t code, unsigned line_num, const char *fmt, va_list args) {
	if (error_code == ERR_OK) {
		error_code = code;
	}

	fprintf(stderr, "Error %d", code);

	if (line_num > 0) {
		fprintf(stderr, " on line %u", line_num);
	}

	fprintf(stderr, ": ");

	vfprintf(stderr, fmt, args); // NOLINT(clang-analyzer-security.VAList)
	va_end(args);

	fputc('\n', stderr);
}

void print_error(err_c_t code, unsigned line_num, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprint_error(code, line_num, fmt, args);
	va_end(args);
}
