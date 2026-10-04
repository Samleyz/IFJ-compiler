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
	ERR_OK = 0,
	ERR_LEXICAL = 1,
	ERR_SYNTACTIC = 2,
	ERR_SEM_UNDEFINED = 3,
	ERR_SEM_REDEFINITION = 4,
	ERR_SEM_PARAMS = 5,
	ERR_SEM_TYPE_COMPAT = 6,
	ERR_SEM_IMMUTABLE = 7,
	ERR_SEM_OTHER = 10,
	ERR_INTERNAL = 99
} err_c_t;

void throw_err(err_c_t code, int line_num, const char *fmt, ...);

void throw_warning(int line_num, const char *fmt, ...);

#endif
