/**
 * @file parser_grammar.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the IFJ26 grammar (LL1*) rules
 * @date 2026-10-22
 */

#include "parser_grammar.h"

/*
 * Note for whoever implements this:
 * Grab token helpers from parser_private.h (peek, match, expect - remember expect blows up on
 * error). Expressions aren't handled here, just call parse_expression() from parse_expr.h.
 * Every rule returns an ast_node_t subtree. Create the AST node right at the start of the
 * function - all nodes will be tracked and cleaned up centrally on error anyway, so no need to
 * create temporary locals.
 */

ast_node_t parse_header(parser_t *p) {
	// TODO: parse header rule implementation
	(void)p;
	return nullptr;
}

ast_node_t parse_body(parser_t *p) {
	// TODO: parse body rule implementation
	(void)p;
	return nullptr;
}

ast_node_t parse_block(parser_t *p) {
	// TODO: parse block rule implementation
	(void)p;
	return nullptr;
}

ast_node_t parse_if_statement(parser_t *p) {
	// TODO: parse if statement rule implementation
	(void)p;
	return nullptr;
}
