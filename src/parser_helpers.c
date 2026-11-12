/**
 * @file parser_helpers.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of helper functions for parser
 * @date 2026-10-20
 */

#include "error.h"
#include "lexer.h"
#include "parser_private.h"
#include "token.h"
#include <string.h>

void parser_advance(parser_t *parser) {
	free_token_attr(&parser->curr_tkn);
	parser->curr_tkn = parser->look_ahead_tkn;
	parser->look_ahead_tkn = (token){0}; // curr_tkn owns the string now, the lexer may throw below
	parser->look_ahead_tkn = lexer_get_token();
}

bool peek_token_type(const parser_t *parser, token_type type) {
	return (bool)(parser->curr_tkn.type == type);
}

bool peek_operator(const parser_t *parser, op_type_t op_type) {
	return (bool)(parser->curr_tkn.type == TOKEN_OPERATOR &&
				  parser->curr_tkn.attribute.operator_type == op_type);
}

bool peek_keyword(const parser_t *parser, kw_type_t kw_type) {
	return (bool)(parser->curr_tkn.type == TOKEN_KEYWORD &&
				  parser->curr_tkn.attribute.keyword == kw_type);
}

bool peek_specific_id(const parser_t *parser, const char *identifier) {
	if (identifier != NULL && parser->curr_tkn.type == TOKEN_IDENTIFIER &&
		parser->curr_tkn.attribute.string_val != NULL) {
		return (bool)(strcmp(parser->curr_tkn.attribute.string_val, identifier) == 0);
	}
	return false;
}

// Tool: Google Gemini
// The following helper functions (peek_*, match_*, expect_*) were ai generated.
// Prompt (paraphrase): Implement parser helper functions in C: peek_* - non-consuming token
// inspection, match_* - advance parser on match, expect_* to log ERR_SYNTACTIC on mismatch. Note:
// Generated functions were verified and adapted to our token model and error handling.
bool peek_data_type(const parser_t *parser) {
	if (parser->curr_tkn.type != TOKEN_KEYWORD) {
		return false;
	}
	kw_type_t kw = parser->curr_tkn.attribute.keyword;
	return (bool)(kw == KW_INT || kw == KW_DOUBLE || kw == KW_STRING || kw == KW_BOOLEAN ||
				  kw == KW_UNIT || kw == KW_LIST);
}

bool peek_is_literal(const parser_t *parser) {
	token_type type = parser->curr_tkn.type;
	return (bool)(type == TOKEN_INTEGER || type == TOKEN_DOUBLE || type == TOKEN_STRING ||
				  type == TOKEN_BOOLEAN);
}

bool peek_is_rel_op(const parser_t *parser) {
	if (parser->curr_tkn.type != TOKEN_OPERATOR) {
		return false;
	}
	op_type_t op = parser->curr_tkn.attribute.operator_type;
	return (bool)(op == OPERATOR_LESS || op == OPERATOR_LESS_EQUAL || op == OPERATOR_GREATER ||
				  op == OPERATOR_GREATER_EQUAL || op == OPERATOR_EQUAL || op == OPERATOR_NOT_EQUAL);
}

bool peek_is_expr_beg(const parser_t *parser) {
	if (peek_is_literal(parser) || peek_keyword(parser, KW_IF)) {
		return true;
	}
	if (parser->curr_tkn.type == TOKEN_IDENTIFIER || parser->curr_tkn.type == TOKEN_LPAREN ||
		parser->curr_tkn.type == TOKEN_LSQUARE) {
		return true;
	}
	if (parser->curr_tkn.type == TOKEN_OPERATOR) {
		op_type_t op = parser->curr_tkn.attribute.operator_type;
		return (bool)(op == OPERATOR_PLUS || op == OPERATOR_MINUS || op == OPERATOR_EX_MARK);
	}
	return false;
}

bool peek_line_beg(const parser_t *parser) {
	if (peek_keyword(parser, KW_VAL) || peek_keyword(parser, KW_VAR) ||
		peek_keyword(parser, KW_RETURN) || peek_keyword(parser, KW_WHILE) ||
		peek_keyword(parser, KW_IF) || peek_keyword(parser, KW_FOR) ||
		parser->curr_tkn.type == TOKEN_IDENTIFIER || peek_is_expr_beg(parser)) {
		return true;
	}
	return false;
}

bool peek_next_token_type(const parser_t *parser, token_type type) {
	return (bool)(parser->look_ahead_tkn.type == type);
}

bool peek_next_operator(const parser_t *parser, op_type_t op_type) {
	return (bool)(parser->look_ahead_tkn.type == TOKEN_OPERATOR &&
				  parser->look_ahead_tkn.attribute.operator_type == op_type);
}

bool peek_next_keyword(const parser_t *parser, kw_type_t kw_type) {
	return (bool)(parser->look_ahead_tkn.type == TOKEN_KEYWORD &&
				  parser->look_ahead_tkn.attribute.keyword == kw_type);
}

bool match_token_type(parser_t *parser, token_type type) {
	if (peek_token_type(parser, type)) {
		parser_advance(parser);
		return true;
	}
	return false;
}

bool match_operator(parser_t *parser, op_type_t op_type) {
	if (peek_operator(parser, op_type)) {
		parser_advance(parser);
		return true;
	}
	return false;
}

bool match_keyword(parser_t *parser, kw_type_t kw_type) {
	if (peek_keyword(parser, kw_type)) {
		parser_advance(parser);
		return true;
	}
	return false;
}

bool match_specific_id(parser_t *parser, const char *identifier) {
	if (peek_specific_id(parser, identifier)) {
		parser_advance(parser);
		return true;
	}
	return false;
}

bool expect_token_type(parser_t *parser, token_type type) {
	if (match_token_type(parser, type)) {
		return true;
	}
	print_error(ERR_SYNTACTIC, parser->curr_tkn.line_num, "Unexpected token");
	return false;
}

bool expect_operator(parser_t *parser, op_type_t op_type) {
	if (match_operator(parser, op_type)) {
		return true;
	}
	print_error(ERR_SYNTACTIC, parser->curr_tkn.line_num, "Unexpected operator");
	return false;
}

bool expect_keyword(parser_t *parser, kw_type_t kw_type) {
	if (match_keyword(parser, kw_type)) {
		return true;
	}
	print_error(ERR_SYNTACTIC, parser->curr_tkn.line_num, "Unexpected keyword");
	return false;
}

bool expect_specific_id(parser_t *parser, const char *identifier) {
	if (match_specific_id(parser, identifier)) {
		return true;
	}
	print_error(ERR_SYNTACTIC, parser->curr_tkn.line_num, "Expected identifier '%s'",
				identifier != NULL ? identifier : "");
	return false;
}
