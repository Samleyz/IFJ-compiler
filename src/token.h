/**
 * @file token.h
 * @author Michal Sobek (xsobekm00@vutbr.cz), Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Token datatype interface
 * @date 2026-10-10
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "error.h"
#include "stdbool.h"
#include <stdint.h>

/**
 * @brief types of operators
 */
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
	OPERATOR_COLON,
	OPERATOR_SLASH,
	OPERATOR_DOT
} op_type_t;

/**
 * @breif types of keywords
 */
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

/**
 * @brief attributes of the token
 */
typedef union {
	op_type_t operator;			 /**< operator type */
	kw_type_t keyword;			 /**< keyword type */
	int32_t int_val;			 /**< integer value */
	double double_val;			 /**< double value */
	bool bool_literal;			 /**< bool literal value */
	char *string_val;			 /**< string literal value */
	struct {					 /**< struct representing error */
		err_c_t err_code;		 /**< code of the error */
		const char *err_message; /**< string with the error message */
	} err_info;
} token_attr_t;

/**
 * @brief types of tokens
 */
typedef enum {
	TOKEN_IDENTIFIER, /**< letters A-Z and _	*/
	TOKEN_INTEGER,	  /**< integer number */
	TOKEN_DOUBLE,	  /**< number containing exactly one '.' */
	TOKEN_STRING,	  /**< " ",""" """ */
	TOKEN_OPERATOR,	  /**< \b(+|-|*|/|=|<|>|<=|>=|==|!=|....)\b check project spec for all operators
					   */
	TOKEN_KEYWORD,	  /**< \b(def|do|int|...)\b check project spec for all keywords */
	TOKEN_BOOLEAN,	  /**< true of false literal */
	TOKEN_UNDERSCORE, /**< _ character */
	TOKEN_EOF,		  /**< end of file */
	TOKEN_EOL,		  /**< end of line */
	TOKEN_INDENT,	  /**< indentation token */
	TOKEN_DEDENT,	  /**< dedentation token */
	TOKEN_LPAREN,	  /**< left parenthesis */
	TOKEN_RPAREN,	  /**< right parenthesis */
	TOKEN_LSQUARE,	  /**< left square parenthesis */
	TOKEN_RSQUARE,	  /**< right square parenthesis */
	TOKEN_MAIN,		  /**< @main token */
	TOKEN_INVALID	  /**< token sent if error occured */
} token_type;

/**
 * @brief token struct abstract representation of a lexeme [lexeme_type, lexeme]
 */
typedef struct {
	token_type type;		/**< type of the token */
	token_attr_t attribute; /**< token attribute (string, operator type etc.) */
	unsigned line_num;		/**< number of the line where was the token located */
} token;

/**
 * @brief token creation & storage on heap
 * @param tkn_type - type of the token (op, id, number, str,...)
 * @param attribute - attribute of the token
 * @return a pointer to the token type for later analysation
 */
token *create_token(token_type tkn_type, token_attr_t attribute, unsigned line_num);

/**
 * @brief free the allocated memory of token
 * @param token pointer of token to be freed
 */
void free_token(token *tkn);

#endif
