/**
 * @file token.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of functions for work with tokens
 * @date 2026-10-30
 */

#include "token.h"
#include <stdlib.h>

void free_token_attr(token *tkn) {
	if (tkn != NULL && (tkn->type == TOKEN_IDENTIFIER || tkn->type == TOKEN_STRING)) {
		free(tkn->attribute.string_val);
		tkn->attribute.string_val = nullptr;
	}
}
