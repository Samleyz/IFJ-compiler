/*
 * token.c
 * Implementation IFJ-26, 30.9.2026
 * Autor: Michal Sobek, xsobekm00, VUT FIT
 * Translated: gcc 13.3.0
 * helper functions for basic work with tokens
 */

#include "token.h"
#include <stdlib.h>
#include <string.h>

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
