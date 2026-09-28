/*
 * tokens.c
 * Implementation IFJ-26, 30.9.2026
 * Autor: Michal Sobek, xsobekm00, VUT FIT
 * Translated: gcc 13.3.0
 * helper functions for basic work with tokens
 */

#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEYWORDS_LEN 22
#define OPERATORS_LEN 10

const char *keywords[] = {"def", "do",	   "Double",  "else", "if",	   "import",
						  "Int", "return", "String",  "then", "Unit",  "val",
						  "var", "while",  "Boolean", "case", "class", "extension",
						  "for", "List",   "match",	  "type"};

const char *operators[] = {"+", "-", "*", "/", "<", "<=", ">", ">=", "==", "!="};

token *create_token(token_type tkn_type, dstring_t lexeme) {
	token *tkn = (token *)malloc(sizeof(token));
	if (!tkn) {
		return NULL;
	}
	tkn->type = tkn_type;
	tkn->lexeme = lexeme;
	return tkn;
}

void free_token(token *tkn) {
	if (tkn) {
		dstring_dtor(tkn->lexeme);
		free(tkn);
	}
}

int check_keyword(dstring_t lexeme) {
	char *str = dstring_get_string(lexeme);
	if (!str) {
		return 0;
	}

	// comparing lexeme to keywords of IFJ26
	for (int kw_idx = 0; kw_idx < KEYWORDS_LEN; kw_idx++) {
		if (strcmp(str, keywords[kw_idx]) == 0) {
			return 1; // match -> the id is a keyword
		}
	}

	return 0;
}

int check_operator_validity(dstring_t lexeme) {
	char *str = dstring_get_string(lexeme);
	if (!str) {
		return 0;
	}

	// comparing lexeme to operators of IFJ26
	for (int op_idx = 0; op_idx < OPERATORS_LEN; op_idx++) {
		if (strcmp(str, operators[op_idx]) == 0) {
			return 1; // match -> operator is valid
		}
	}

	return 0;
}