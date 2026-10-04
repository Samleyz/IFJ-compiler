/**
 * @file lexer.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief
 * @date 2026-10-04
 */

#include "lexer.h"
#include "error.h"
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
	if (source == NULL) {
		throw_err(ERR_INTERNAL, -1, "Pointer to input file can not be NULL");
	}

	fsm_state = START;
	bracket_cnt = 0;
	comment_depth_cnt = 0;
	line_num = 1;
	source_file = source;

	// TODO stack init
}

void lexer_cleanup(void) {
	// TODO free stack
}

token *lexer_get_token(void) {
	// TODO
}
