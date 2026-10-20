/**
 * @file parser_helpers.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of helper functions for parser
 * @date 2026-10-20
 */

#include "parser_private.h"

void parser_advance(parser_t *parser) {
	// TODO: implementation
}

bool peek_token_type(const parser_t *p, token_type type) {
	// TODO: implementation
	(void)p;
	(void)type;
	return false;
}

bool peek_operator(const parser_t *p, op_type_t op_type) {
	// TODO: implementation
	(void)p;
	(void)op_type;
	return false;
}

bool peek_keyword(const parser_t *p, kw_type_t kw_type) {
	// TODO: implementation
	(void)p;
	(void)kw_type;
	return false;
}

bool peek_specific_id(const parser_t *p, const char *identifier) {
	// TODO: implementation
	(void)p;
	(void)identifier;
	return false;
}

bool peek_data_type(const parser_t *p) {
	// TODO: implementation
	(void)p;
	return false;
}

bool peek_is_literal(const parser_t *p) {
	// TODO: implementation
	(void)p;
	return false;
}

bool peek_is_rel_op(const parser_t *p) {
	// TODO: implementation
	(void)p;
	return false;
}

bool peek_is_expr_beg(const parser_t *p) {
	// TODO: implementation
	(void)p;
	return false;
}

bool peek_next_token_type(const parser_t *p, token_type type) {
	// TODO: implementation
	(void)p;
	(void)type;
	return false;
}

bool peek_next_operator(const parser_t *p, op_type_t op_type) {
	// TODO: implementation
	(void)p;
	(void)op_type;
	return false;
}

bool peek_next_keyword(const parser_t *p, kw_type_t kw_type) {
	// TODO: implementation
	(void)p;
	(void)kw_type;
	return false;
}

bool match_token_type(parser_t *p, token_type type) {
	// TODO: implementation
	(void)p;
	(void)type;
	return false;
}

bool match_operator(parser_t *p, op_type_t op_type) {
	// TODO: implementation
	(void)p;
	(void)op_type;
	return false;
}

bool match_keyword(parser_t *p, kw_type_t kw_type) {
	// TODO: implementation
	(void)p;
	(void)kw_type;
	return false;
}

bool match_specific_id(parser_t *p, const char *identifier) {
	// TODO: implementation
	(void)p;
	(void)identifier;
	return false;
}

void expect_token_type(parser_t *p, token_type type) {
	// TODO: implementation
	(void)p;
	(void)type;
}

void expect_operator(parser_t *p, op_type_t op_type) {
	// TODO: implementation
	(void)p;
	(void)op_type;
}

void expect_keyword(parser_t *p, kw_type_t kw_type) {
	// TODO: implementation
	(void)p;
	(void)kw_type;
}

void expect_specific_id(parser_t *p, const char *identifier) {
	// TODO: implementation
	(void)p;
	(void)identifier;
}
