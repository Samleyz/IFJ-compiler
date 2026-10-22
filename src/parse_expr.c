/**
 * @file parse_expr.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of precedence syntactic analysis for expressions
 * @date 2026-10-22
 */

#include "parse_expr.h"
#include "parse_expr_private.h"

/*
 * Note for whoever implements this:
 * This is the bottom-up precedence parser for expressions called by recursive descent.
 * Use the stack from stack.h and build the precedence table for operators.
 * Parse until you hit a token that can't continue the expression, reduce everything
 * into AST operator/operand nodes, and return the subtree root. Don't forget to free
 * the stack if you bail out on error.
 */

ast_node_t parse_expression(parser_t *p) {
	// TODO: implement precedence analysis for expressions
	(void)p;
	return nullptr;
}
