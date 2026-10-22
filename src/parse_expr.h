/**
 * @file parse_expr.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Interface for expression parsing via precedence analysis
 * @date 2026-10-22
 */

#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H

#include "ast.h"
#include "parser_private.h"

/**
 * @brief Parses an expression using precedence syntactic analysis
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to expression subtree, or nullptr
 */
ast_node_t parse_expression(parser_t *p);

#endif
