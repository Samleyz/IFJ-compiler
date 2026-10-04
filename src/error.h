/**
 * @file error.h
 * @author Jan Mejzlík
 * @brief Provides interface for error and warning functions for centralized error handling. This
 * module provides interface for printing formated error messages to stderr. Includes mechanisms for
 * safely terminating the program.
 * @date 2026-10-04
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

typedef enum {
	// TODO
} err_c_t;

void throw_err(err_c_t code, int line_num, const char *fmt, ...);

void throw_warning(int line_num, const char *fmt, ...);

#endif
