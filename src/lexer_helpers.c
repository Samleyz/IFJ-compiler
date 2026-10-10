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
