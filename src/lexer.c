/**
 * @file lexer.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief
 * @date 2026-10-04
 */

#include "lexer.h"
#include <stdio.h>

typedef enum {
	START,
	IDENTIFIER,
	UNDERSCORE,
	NUMBER,
	UND_FOR_NUMS,
	ZERO,
	DOUBLE,
	EXPONENT,
	EX_NUM,
	DOT_STATE,
	DOT_STATE_2
} state_t;

static FILE *source_file = NULL;
static state_t fsm_state = START;
static int bracket_cnt = 0;
static int comment_depth_cnt = 0;
static unsigned line_num = 0;

void lexer_init(FILE *source) {
	// TODO
}

void lexer_cleanup(void) {
	// TODO
}

token *lexer_get_token(void) {
	// TODO
}
