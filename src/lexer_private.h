/**
 * @file lexer_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Internal private declarations for lexer
 * @date 2026-10-09
 */

#ifndef LEXER_PRIVATE_H
#define LEXER_PRIVATE_H

#include "dynamic_string.h"
#include <stdio.h>

/**
 * @biref state of the fsm
 */
typedef enum state {
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
	COMMENT_ENTRY,
	STATE_CNT
} state_t;

typedef struct {
	FILE *source_file; /**< Input file */
	state_t fsm_state; /**< Current state a the fsm */
	unsigned line_num; /**< The number of the line currently beeíng read */

	int bracket_cnt;	   /**< Brackent depth counter */
	int comment_depth_cnt; /**< Current depth of the comment */

	dstring_t dynamic_string; /**< Dynamic string used to store lexemes */
							  // TODO add stack
} lexer_state_t;

#endif
