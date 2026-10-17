/**
 * @file lexer_helpers.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Helper functions for lexer.c file
 * @date 2026-10-09
 */

#include "lexer_private.h"
#include "token.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

const char *keywords[] = {"def", "do",	   "Double",  "else", "if",	   "import",
						  "Int", "return", "String",  "then", "Unit",  "val",
						  "var", "while",  "Boolean", "case", "class", "extension",
						  "for", "List",   "match",	  "type"};

#define KEYWORDS_LEN (int)(sizeof(keywords) / sizeof(keywords[0]))

int handle_indent(lexer_state_t *lexer_state) {
	// Count leading spaces
	int c;
	static size_t space_counter = 0;
	while (' ' == (c = fgetc(lexer_state->source_file))) {
		space_counter++;
	}
	ungetc(c, lexer_state->source_file);
	// Choose outcome
	if (space_counter > (size_t)stack_top(lexer_state->indent_stack)) {		 // If deeper
		if (!stack_push(lexer_state->indent_stack, (void *)space_counter)) { // Store
			// FAILED MALLOC
			return -3;
		}
		// INDENT
		return 1;
	} else if (space_counter < (size_t)stack_top(lexer_state->indent_stack)) { // If shallower
		stack_pop(lexer_state->indent_stack);								   // Remove resolved
		// test for missing indent level
		if (space_counter > (size_t)stack_top(lexer_state->indent_stack)) {
			// INDENTATION ERROR - invalid token (no such level of indent used before)
			return -2;
		}
		// DEDENT
		return -1;
	} else {			   // Same level
		space_counter = 0; // Reset for later use
		// DONE - can leave FSM state
		return 0;
	}
}

bool convert_int(const char *num_string, int32_t *int_number) {

	if (!num_string || !int_number || num_string[0] == '\0') {
		throw_err(ERR_INTERNAL, -1, "num_string or int_number is NULL");
	}

	errno = 0;
	char *endptr = NULL;
	long result = strtol(num_string, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0' || result < INT32_MIN || result > INT32_MAX) {
		return false;
	}

	*int_number = (int32_t)result;
	return true;
}

bool convert_double(const char *num_string, double *double_number) {

	if (!num_string || !double_number || num_string[0] == '\0') {
		throw_err(ERR_INTERNAL, -1, "num_string or double_number is NULL");
	}

	errno = 0;
	char *endptr = NULL;
	double result = strtod(num_string, &endptr);

	if (errno == ERANGE || *endptr != '\0') {
		return false;
	}

	*double_number = result;
	return true;
}

bool is_keyword(const char *string, kw_type_t *kw_type) {

	if (!string || !kw_type) {
		return false;
	}

	for (int kw_idx = 0; kw_idx < KEYWORDS_LEN; kw_idx++) {
		if (strcmp(string, keywords[kw_idx]) == 0) {
			*kw_type = kw_idx;
			return true; // match -> operator is valid
		}
	}

	return false;
}

bool is_bool_literal(const char *string, bool *literal_val) {

	if (!string || !literal_val) {
		throw_err(ERR_INTERNAL, -1, "string or literal_val is NULL");
	}

	if (strcmp(string, "true") == 0) {
		*literal_val = true;
		return true;
	}

	if (strcmp(string, "false") == 0) {
		*literal_val = false;
		return true;
	}

	return false;
}
