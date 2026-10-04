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
	EXP_SIGN,
	DOT_STATE,
	DOT_STATE_2,
	D_RETURN,
	UND_FOR_DOUBLES,
	UND_FOR_EXP,
	EOL_STATE,
	EOF_STATE,
	HANDLE_INDENT,
	TAB_STATE,
	MAIN_AT,
	MAIN_M,
	MAIN_A,
	MAIN_I,
	MAIN_N,
	O_BRACKET,
	C_BRACKET,
	B_EOL_STATE,
	O_SQUARE_BRACKET,
	C_SQUARE_BRACKET,
	STRING,
	STRING_END,
	BLOCK_STRING,
	BLOCK_STRING_END,
	BLOCK_STRING_EX_1,
	BLOCK_STRING_EX_2,
	ALERT_ESC,
	OP_SEMICOLON,
	OP_MINUS,
	OP_DOUBLE_DOT,
	OP_STAR,
	OP_COMMA,
	OP_EX_MARK,
	OP_NOT_EQUAL,
	OP_AND,
	OP_AND_ENTRY,
	OP_PLUS,
	OP_PLUS_PLUS,
	OP_VERTICAL_BAR,
	OP_OR,
	OP_SLASH,
	OP_LESS,
	OP_LESS_EQUAL,
	OP_LEFT_ARROW,
	OP_GREATER,
	OP_GREATER_EQUAL,
	OP_ASSIGN,
	OP_RIGHT_DOUBLE_ARROW,
	OP_EQUAL,
	ONE_LINE_COM,
	BLOCK_COM,
	NEWLINE_BLOCK_COM,
	ALERT_COM_IN_1,
	ALERT_COM_IN_2,
	ALERT_COM_OUT_1,
	ALERT_COM_OUT2,
	COMMENT_ENTRY
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
