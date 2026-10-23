/**
 * @file error.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Provides interface for error and warning functions for centralized error handling. This
 * module provides interface for printing formated error messages to stderr. Includes mechanisms for
 * safely terminating the program.
 * @date 2026-10-04
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

typedef enum {
	ERR_OK = 0,				  // without an error, successful compilation
	ERR_LEXICAL = 1,		  // error during lexical analysis
	ERR_SYNTACTIC = 2,		  // error during syntactical analysis
	ERR_SEM_UNDEFINED = 3,	  // error during semantical analysis caused by an undefined function,
							  // variable, type or by missing @main annotation
	ERR_SEM_REDEFINITION = 4, // error during semantical analysis caused by unauthorized
							  // redefinition of a function, variable or type
	ERR_SEM_PARAMS =
		5, // error during semantical analysis caused by wrong number or wrong type of arguments
		   // during function call, unauthorized parameters of a function with @main annotation
	ERR_SEM_TYPE_COMPAT = 6, // error during semantical analysis caused by type incompatibility
	ERR_SEM_IMMUTABLE = 7,	 // error during semantical analysis caused by attempt to write into
							 // non-modifiable variable or formal argument of a function
	ERR_SEM_OTHER = 10,		 // error caused by other semantical problems
	ERR_INTERNAL = 99		 // internal compiler error, not caused by the compiled program
} err_c_t;

/**
 * @brief Throws an error message to stderr and terminates the program
 *
 * @param[in] code Error code with which the program will be terminated
 * @param[in] line_num Number of the line where the error is located, 0 will not print line num
 * @param[in] fmt Formatted error string to be printed to stderr
 */
__attribute__((noreturn)) __attribute__((format(printf, 3, 4))) void
throw_err(err_c_t code, unsigned line_num, const char *fmt, ...);

/**
 * @brief Throws a warning message to stderr and continues running
 *
 * @param[in] line_num Number of the line where the warning is located, 0 will not print line num
 * @param[in] fmt Formatted warning string to be printed to stderr
 */
__attribute__((format(printf, 2, 3))) void throw_warning(unsigned line_num, const char *fmt, ...);

/**
 * @brief Prints an error message to stderr without terminating the program and saves the first error code
 *
 * @param[in] code Error code to be stored for exit
 * @param[in] line_num Number of the line where the error is located, 0 will not print line num
 * @param[in] fmt Formatted error string to be printed to stderr
 */
__attribute__((format(printf, 3, 4))) void
print_error(err_c_t code, unsigned line_num, const char *fmt, ...);

/**
 * @brief Terminates the program with the first recorded error code
 */
__attribute__((noreturn)) void error_exit(void);

#endif
