/**
 * @file parser_grammar.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Implementation of the IFJ26 grammar (LL1*) rules
 * @date 2026-10-22
 */

#include "parser_grammar.h"
#include "parse_expr.h"
#include "stdlib.h"

/*
 * Note for whoever implements this:
 * Grab token helpers from parser_private.h (peek, match, expect - remember expect blows up on
 * error). Expressions aren't handled here, just call parse_expression() from parse_expr.h.
 * Every rule returns an ast_node_t subtree. Create the AST node right at the start of the
 * function - all nodes will be tracked and cleaned up centrally on error anyway, so no need to
 * create temporary locals.
 */

ast_node_t parse_header(parser_t *p) {
	if (!expect_keyword(p, KW_IMPORT)) {
		return NULL;
	}
	if (!expect_specific_id(p, "ifj26")) {
		return NULL;
	}
	if (!expect_operator(p, OPERATOR_DOT)) {
		return NULL;
	}
	if (!expect_operator(p, OPERATOR_STAR)) {
		return nullptr;
	}
	if (!expect_token_type(p, TOKEN_EOL)) {
		return nullptr;
	}
	return p->root != nullptr ? p->root : (ast_node_t)(uintptr_t)true;
}

ast_node_t parse_top_def_seq(parser_t *p) {

	(void)p;
	return nullptr;
}

ast_node_t parse_top_def(parser_t *p) {
	// TODO: Rule 5 & 6
	// <top_def> -> <func_def> | <ext_def>
	(void)p;
	return nullptr;
}

ast_node_t parse_func_def(parser_t *p) {
	// TODO: Rule 24
	// <func_def> -> <opt_main> "def" identifier <param_list_seq> <opt_type> "=" <func_body>
	(void)p;
	return nullptr;
}

ast_node_t parse_ext_def(parser_t *p) {
	// TODO: Rule 54-57
	// <ext_def> -> "extension" "(" identifier ":" <type> ")" "EOL" "indent" <ext_method_seq>
	// "dedent"
	(void)p;
	return nullptr;
}

ast_node_t parse_block(parser_t *p) {
	// TODO: Rule 7
	// <block> -> "indent" <line> <line_seq> <opt_eol> "dedent"
	(void)p;
	return nullptr;
}

ast_node_t parse_cond_stmt(parser_t *p) {
	// TODO: Rule 76-82
	// <cond_stmt> -> "if" expr "then" <branch_body> <opt_else>
	(void)p;
	return nullptr;
}

ast_node_t parse_if_statement(parser_t *p) { return parse_cond_stmt(p); }

ast_node_t parse_while_stmt(parser_t *p) {
	// TODO: Rule 83
	// <while_stmt> -> "while" expr "do" "EOL" <block>
	(void)p;
	return nullptr;
}

ast_node_t parse_for_stmt(parser_t *p) {
	// TODO: Rule 84-95
	// <for_stmt> -> "for" <for_header> "do" <branch_body>
	(void)p;
	return nullptr;
}

ast_node_t parse_var_def(parser_t *p) {
	// TODO: Rule 67-69
	// <var_def> -> <var_kw> identifier <opt_type> "=" expr
	(void)p;
	return nullptr;
}

ast_node_t parse_assign_stmt(parser_t *p) {
	// TODO: Rule 70
	// <assign_stmt> -> identifier "=" expr
	(void)p;
	return nullptr;
}

ast_node_t parse_return_stmt(parser_t *p) {
	// TODO: Rule 51-53
	// <return_stmt> -> "return" <opt_expr>
	(void)p;
	return nullptr;
}

ast_node_t parse_simple_stmt(parser_t *p) {
	// TODO: Rule 17-20
	// <simple_stmt> -> <var_def> | <assign_stmt> | <return_stmt> | expr
	(void)p;
	return nullptr;
}

ast_node_t parse_complex_stmt(parser_t *p) {
	// TODO: Rule 21-23
	// <complex_stmt> -> <while_stmt> | <cond_stmt> | <for_stmt>
	(void)p;
	return nullptr;
}

ast_node_t parse_param_list(parser_t *p) {
	// TODO: Rule 34-41
	// <param_list> -> "(" <param_list_opt> ")"
	(void)p;
	return nullptr;
}

char *parse_type(parser_t *p) {
	// TODO: Rule 71-75
	// <type> -> "Int" | "Double" | "String" | "Unit" | "Boolean"
	(void)p;
	return nullptr;
}
