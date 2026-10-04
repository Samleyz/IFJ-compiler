/**
 * @file error.c
 * @author Jan Mejzlík
 * @brief Contains implementation of error and warning functions for central error handling.
 * Includes mechanisms for safely terminating the program.
 * @date 2026-10-04
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "error.h"

void throw_err(err_c_t code, int line_num, const char *fmt, ...) {
	//TODO
}

void throw_warning(int line_num, const char *fmt, ...) {
	//TODO
}
