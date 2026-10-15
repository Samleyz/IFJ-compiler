/**
 * @file lexer_helpers.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz), Michal Sobek (xsobekm00@vutbr.cz)
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
	// TODO
	return 0;
}

bool convert_int(const char *num_string, int32_t *int_number) {

	if (!num_string || !int_number || num_string[0] == '\0') {
		throw_err(ERR_INTERNAL, -1, "num_string or int_number is NULL");
	}

	int64_t result = 0; // 64b for error detection later on

	for (int c_idx = 0; num_string[c_idx] != '\0'; c_idx++) {
		char c_tmp = num_string[c_idx];

		if (c_tmp < '0' || c_tmp > '9') {
			return false; // not digit, how did you do this???
		}

		result = result * 10 + (c_tmp - '0'); // '0'-'0'= 0,...,'9'-'0'= 9

		if (result > INT32_MAX) {
			return false; // int too big
		}
	}

	*int_number = (int32_t)result;
	return true;
}

bool convert_double(const char *num_string, double *double_number) {

	if (!num_string || !double_number) {
		throw_err(ERR_INTERNAL, -1, "num_string or double_number is NULL");
	}

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

	if (!string || !literal_val) {
		throw_err(ERR_INTERNAL, -1, "string or literal_val is NULL");
	}

	if (strcmp(string, "true") == true) {
		literal_val = true;
		return true;
	}

	if (strcmp(string, "false") == false) {
		literal_val = false;
		return true;
	}

	return false;
}
