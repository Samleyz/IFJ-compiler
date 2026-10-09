/**
 * @file token.h
 * @author Michal Sobek (xsobekm00@vutbr.cz), Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Token datatype interface
 * @date 2026-10-10
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "stdbool.h"
#include <stdint.h>

typedef enum {
	OPERATOR_AND,
	OPERATOR_PLUS_PLUS,
	OPERATOR_OR,
	OPERATOR_LESS_EQUAL,
	OPERATOR_LEFT_ARROW,
	OPERATOR_GREATER_EQUAL,
	OPERATOR_EQUAL,
	OPERATOR_RIGHT_DOUBLE_ARROW,
	OPERATOR_NOT_EQUAL,
	OPERATOR_ASSIGN,
	OPERATOR_GREATER,
	OPERATOR_LESS,
	OPERATOR_VERTICAL_BAR,
	OPERATOR_PLUS,
	OPERATOR_EX_MARK,
	OPERATOR_COMMA,
	OPERATOR_STAR,
	OPERATOR_MINUS,
	OPERATOR_SEMICOLON,
	OPERATOR_DOUBLE_DOT,
	OPERATOR_SLASH
} op_type_t;

typedef enum {
	KW_DEF,
	KW_DO,
	KW_DOUBLE,
	KW_ELSE,
	KW_IF,
	KW_IMPORT,
	KW_INT,
	KW_RETURN,
	KW_STRING,
	KW_THEN,
	KW_UNIT,
	KW_VAL,
	KW_VAR,
	KW_WHILE,
	KW_BOOLEAN,
	KW_CASE,
	KW_CLASS,
	KW_EXTENSION,
	KW_FOR,
	KW_LIST,
	KW_MATCH,
	KW_TYPE
} kw_type_t;

typedef union {
	op_type_t operator;
	kw_type_t keyword;
	int32_t int_val;
	double double_val;
} token_attr_t;

/**
 * @brief we are differentiating between these token types (for now :D)
 */
typedef enum {
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_DOUBLE,	// number containing exactly one '.'
	TOKEN_STRING,	// " ",""" """
	TOKEN_OPERATOR, // \b(+|-|*|/|=|<|>|<=|>=|==|!=)\b
	TOKEN_KEYWORD,	// \b(def|do|int|...)\b check project spec for all keywords
	TOKEN_UNDERSCORE,
	TOKEN_EOF,
	TOKEN_EOL,
	TOKEN_INDENT,
	TOKEN_DEDENT,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LSQUARE,
	TOKEN_RSQUARE,
	TOKEN_MAIN
} token_type;

/**
 * @brief token struct --- abstract representation of a lexeme [lexeme_type, lexeme]
 */
typedef struct {
	token_type type;
	token_attr_t attribute;
	char *lexeme;
} token;

/**
 * @brief token creation & storage on heap
 * @param type - type of the token (op, id, number, str,...)
 * @param lexeme - lexeme to be analyzed
 * @return a pointer to the token type for later analysation
 */
token *create_token(token_type type, char *lexeme);

/**
 * @brief free the allocated memory of token
 * @param token pointer of token to be freed
 */
void free_token(token *tkn);

#endif
