/**
 * @file token.c
 * @author Michal Sobek (xsobekm00@vutbr.cz), Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of basic functions to work with tokens
 * @date 2026-10-10
 */

#include "token.h"
#include <stdlib.h>

token *create_token(token_type tkn_type, token_attr_t attribute) {
	token *tkn = (token *)calloc(1, sizeof(token));
	if (!tkn) {
		return NULL;
	}
	tkn->attribute = attribute;
	tkn->type = tkn_type;
	return tkn;
}

void free_token(token *tkn) {
	if (tkn) {
		if (tkn->type == TOKEN_IDENTIFIER || tkn->type == TOKEN_STRING) {
			free(tkn->attribute.string_val);
		}
		free(tkn);
	}
}
