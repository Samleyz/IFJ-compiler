/**
 * @file lexer_helpers.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz), Michal Sobek (xsobekm00@vutbr.cz)
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Helper functions for lexer.c file
 * @date 2026-10-09
 */

#include "lexer_private.h"
#include "token.h"

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
	if (space_counter > stack_top(lexer_state->indent_stack)) {		 // If deeper
		if (!stack_push(lexer_state->indent_stack, space_counter)) { // Store
			// FAILED MALLOC
			return -3;
		}
		// INDENT
		return 1;
	} else if (space_counter < stack_top(lexer_state->indent_stack)) { // If shallower
		stack_pop(lexer_state->indent_stack);						   // Remove resolved
		if (space_counter > stack_top(lexer_state->indent_stack)) { // test for missing indent level
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
	// TODO
	return false;
}

bool convert_double(const char *num_string, double *double_number) {
	// TODO
	return false;
}

bool is_keyword(const char *string, kw_type_t *kw_type) {
	// TODO:
	// 1. Changed signature to `const char *str` to keep it universal and decoupled from dstring
	// ADT.
	// 2. Return type is bool - if the string is keyword true otherwise false
	// 3. Iterate through the `keywords` array and return the matching token type.
	// 4. If return is true then return through the parameter kw_type type of the keyword
	(void)string;
	(void)kw_type;
	return true;
}

bool is_bool_literal(const char *string, bool *literal_val) {
	// TODO
	return true;
}
