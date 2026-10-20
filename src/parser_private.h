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

void parser_advance(parser_t *parser);

// Predicates checking a specific token type without advancing - peek
bool peek_token_type(const parser_t *p, token_type type);
bool peek_operator(const parser_t *p, op_type_t op_type);
bool peek_keyword(const parser_t *p, kw_type_t kw_type);
bool peek_specific_id(const parser_t *p, const char *identifier);

bool peek_data_type(const parser_t *p);
bool peek_is_literal(const parser_t *p);
bool peek_is_rel_op(const parser_t *p);
bool peek_is_expr_beg(const parser_t *p);

// Predicates for inspecting the next token - lookahead 2
bool peek_next_token_type(const parser_t *p, token_type type);
bool peek_next_operator(const parser_t *p, op_type_t op_type);
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
