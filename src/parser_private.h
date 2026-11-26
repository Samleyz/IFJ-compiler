/**
 * Implementation of a compiler for the imperative language IFJ26
 *
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
	token_type prev_tkn_type; // type of the token consumed last, a block ends with a dedent
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
 * @param[in] parser Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if current token matches type, otherwise false
 */
bool peek_token_type(const parser_t *parser, token_type type);

/**
 * @brief Checks if current token is an operator of specified type without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if current token is operator of specified type, otherwise false
 */
bool peek_operator(const parser_t *parser, op_type_t op_type);

/**
 * @brief Checks if current token is a keyword of specified type without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if current token is keyword of specified type, otherwise false
 */
bool peek_keyword(const parser_t *parser, kw_type_t kw_type);

/**
 * @brief Checks if current token is an identifier with specified string value without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] identifier String identifier to compare against
 * @return true if current token is identifier matching identifier string, otherwise false
 */
bool peek_specific_id(const parser_t *parser, const char *identifier);

/**
 * @brief Checks if current token is a data type keyword without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @return true if current token is data type keyword, otherwise false
 */
bool peek_data_type(const parser_t *parser);

/**
 * @brief Checks if current token is a literal (integer, double, string, boolean) without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @return true if current token is literal, otherwise false
 */
bool peek_is_literal(const parser_t *parser);

/**
 * @brief Checks if current token is a relational comparison operator without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @return true if current token is relational operator, otherwise false
 */
bool peek_is_rel_op(const parser_t *parser);

/**
 * @brief Checks if current token can begin an expression without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @return true if current token can begin an expression, otherwise false
 */
bool peek_is_expr_beg(const parser_t *parser);

/**
 * @brief Checks if current token can begin a statement line in a block without advancing
 *
 * @param[in] parser Pointer to parser structure
 * @return true if current token can start a line, otherwise false
 */
bool peek_line_beg(const parser_t *parser);

/**
 * @brief Checks if next lookahead token matches specified token type
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if next token matches type, otherwise false
 */
bool peek_next_token_type(const parser_t *parser, token_type type);

/**
 * @brief Checks if next lookahead token is an operator of specified type
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if next token is operator of specified type, otherwise false
 */
bool peek_next_operator(const parser_t *parser, op_type_t op_type);

/**
 * @brief Checks if next lookahead token is a keyword of specified type
 *
 * @param[in] parser Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if next token is keyword of specified type, otherwise false
 */
bool peek_next_keyword(const parser_t *parser, kw_type_t kw_type);

/**
 * @brief Consumes current token if it matches specified token type
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if token matched and was consumed, otherwise false
 */
bool match_token_type(parser_t *parser, token_type type);

/**
 * @brief Consumes current token if it is an operator of specified type
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if token matched and was consumed, otherwise false
 */
bool match_operator(parser_t *parser, op_type_t op_type);

/**
 * @brief Consumes current token if it is a keyword of specified type
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if token matched and was consumed, otherwise false
 */
bool match_keyword(parser_t *parser, kw_type_t kw_type);

/**
 * @brief Consumes current token if it is an identifier with specified string value
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] identifier String identifier to compare against
 * @return true if token matched and was consumed, otherwise false
 */
bool match_specific_id(parser_t *parser, const char *identifier);

/**
 * @brief Consumes current token if it matches specified token type, otherwise records syntax error
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] type Expected token type
 * @return true if token matched and was consumed, otherwise false
 */
bool expect_token_type(parser_t *parser, token_type type);

/**
 * @brief Consumes current token if it is an operator of specified type, otherwise records syntax
 * error
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] op_type Expected operator type
 * @return true if token matched and was consumed, otherwise false
 */
bool expect_operator(parser_t *parser, op_type_t op_type);

/**
 * @brief Consumes current token if it is a keyword of specified type, otherwise records syntax
 * error
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] kw_type Expected keyword type
 * @return true if token matched and was consumed, otherwise false
 */
bool expect_keyword(parser_t *parser, kw_type_t kw_type);

/**
 * @brief Consumes current token if it is an identifier with specified string value, otherwise
 * records syntax error
 *
 * @param[in,out] parser Pointer to parser structure
 * @param[in] identifier String identifier to compare against
 * @return true if token matched and was consumed, otherwise false
 */
bool expect_specific_id(parser_t *parser, const char *identifier);

#endif
