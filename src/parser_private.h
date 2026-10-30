/**
 * @file parser_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Internal declarations for parser
 * @date 2026-10-20
 */

#ifndef PARSER_PRIVATE_H
#define PARSER_PRIVATE_H

#include "ast.h"
#include "token.h"

typedef struct parser {
	token curr_tkn;
	token look_ahead_tkn;
	ast_node_t root;
} parser_t;

/**
 * @brief Advances parser to the next token, shifting lookahead token to current token
 *
 * @param[in,out] parser Pointer to parser structure
 */
void parser_advance(parser_t *parser);

/**
 * @brief Checks if current token matches specified token type without advancing
 *
 * @param[in] p Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if current token matches type, otherwise false
 */
bool peek_token_type(const parser_t *p, token_type type);

/**
 * @brief Checks if current token is an operator of specified type without advancing
 *
 * @param[in] p Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if current token is operator of specified type, otherwise false
 */
bool peek_operator(const parser_t *p, op_type_t op_type);

/**
 * @brief Checks if current token is a keyword of specified type without advancing
 *
 * @param[in] p Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if current token is keyword of specified type, otherwise false
 */
bool peek_keyword(const parser_t *p, kw_type_t kw_type);

/**
 * @brief Checks if current token is an identifier with specified string value without advancing
 *
 * @param[in] p Pointer to parser structure
 * @param[in] identifier String identifier to compare against
 * @return true if current token is identifier matching identifier string, otherwise false
 */
bool peek_specific_id(const parser_t *p, const char *identifier);

/**
 * @brief Checks if current token is a data type keyword without advancing
 *
 * @param[in] p Pointer to parser structure
 * @return true if current token is data type keyword, otherwise false
 */
bool peek_data_type(const parser_t *p);

/**
 * @brief Checks if current token is a literal (integer, double, string, boolean) without advancing
 *
 * @param[in] p Pointer to parser structure
 * @return true if current token is literal, otherwise false
 */
bool peek_is_literal(const parser_t *p);

/**
 * @brief Checks if current token is a relational comparison operator without advancing
 *
 * @param[in] p Pointer to parser structure
 * @return true if current token is relational operator, otherwise false
 */
bool peek_is_rel_op(const parser_t *p);

/**
 * @brief Checks if current token can begin an expression without advancing
 *
 * @param[in] p Pointer to parser structure
 * @return true if current token can begin an expression, otherwise false
 */
bool peek_is_expr_beg(const parser_t *p);

/**
 * @brief Checks if next lookahead token matches specified token type
 *
 * @param[in] p Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if next token matches type, otherwise false
 */
bool peek_next_token_type(const parser_t *p, token_type type);

/**
 * @brief Checks if next lookahead token is an operator of specified type
 *
 * @param[in] p Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if next token is operator of specified type, otherwise false
 */
bool peek_next_operator(const parser_t *p, op_type_t op_type);

/**
 * @brief Checks if next lookahead token is a keyword of specified type
 *
 * @param[in] p Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if next token is keyword of specified type, otherwise false
 */
bool peek_next_keyword(const parser_t *p, kw_type_t kw_type);

// Consumes the token if present and returns true, does not throw an error - match
bool match_token_type(parser_t *p, token_type type);
bool match_operator(parser_t *p, op_type_t op_type);
bool match_keyword(parser_t *p, kw_type_t kw_type);
bool match_specific_id(parser_t *p, const char *identifier);

// Consumes the token if present, otherwise throws a syntax error - expect
void expect_token_type(parser_t *p, token_type type);
void expect_operator(parser_t *p, op_type_t op_type);
void expect_keyword(parser_t *p, kw_type_t kw_type);
void expect_specific_id(parser_t *p, const char *identifier);

#endif
