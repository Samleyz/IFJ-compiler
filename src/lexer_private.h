/**
 * @file lexer_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Internal private declarations for lexer
 * @date 2026-10-09
 */

#ifndef LEXER_PRIVATE_H
#define LEXER_PRIVATE_H

#include "dynamic_string.h"
#include "lexer.h"
#include "stack.h"
#include "token.h"
#include <stdint.h>
#include <stdio.h>

/**
 * @brief state of the fsm
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
	OP_COLON,
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
	AFTER_MULTILINE_COMMENT,
	COMMENT_ENTRY,
	STATE_CNT
} state_t;

typedef struct {
	FILE *source_file; /**< Input file */
	state_t fsm_state; /**< Current state a the fsm */
	unsigned line_num; /**< The number of the line currently beeíng read */

	int bracket_cnt;	   /**< Brackent depth counter */
	int comment_depth_cnt; /**< Current depth of the comment */
	bool line_has_code; /**< Whether the current logical line contains a code token */
	unsigned comment_start_line; /**< Line where the outermost block comment started */

	int string_start_line; /**< The line number where the string started */
	dstring_t dynamic_string; /**< Dynamic string used to store lexemes */
	stack_t indent_stack;	  /**< Stack used to store indentation levels */
} lexer_state_t;

/**
 * @brief Determines what tokens regarding indentations should be sent (if any)
 *
 * @param[in] lexer_state Current state of the lexer
 * @return 0 Indentation is resolved, move to next state
 * @return 1 Indentation is deeper, send INDENT
 * @return -1 Indentation is shallower, send DEDENT
 * @return -2 Indentation error
 * @return -3 Internal error (failed malloc)
 */
int handle_indent(lexer_state_t *lexer_state);

/**
 * @brief Converts an integer from a string in the Scala int format to a 32-bit C variable
 *
 * @param[in] num_string String number in the Scala format to be converted
 * @param[out] int_number Pointer to the variable in which the result will be stored
 * @return true if the conversion was successful, false if the conversion was unsuccessful
 */
bool convert_int(const char *num_string, int32_t *int_number);

/**
 * @brief Converts a double from a string in the Scala double format to a 64-bit C variable
 *
 * @param[in] num_string String number in the Scala format to be converted
 * @param[out] double_number Pointer to the variable in which the result will be stored
 * @return true if the conversion was successful, false if the conversion was unsuccessful
 */
bool convert_double(const char *num_string, double *double_number);

/**
 * @brief Function to determine if the string is an identifier or a keyword
 * @param[in] string String to be checked
 * @param[out] kw_type Pointer to a variable in which the type of the analyzed keyword will be
 * stored
 * @return true if the string is a keyword, false if the string is not a keyword
 */
bool is_keyword(const char *string, kw_type_t *kw_type);

/**
 * @brief Function to dertermine if the string is a boolean literal
 *
 * @param[in] string String to be checked
 * @param[out] literal_val Value of the boolean literal
 * @return true if the string is a boolean literal, otherwise false
 */
bool is_bool_literal(const char *string, bool *literal_val);

#endif
