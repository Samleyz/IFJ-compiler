/**
 * @file parse_expr.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Samuel Cehlarik (xcehlas00@vutbr.cz)
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
 * Handles operators, groups, calls, member access, interpolated strings and if expressions.
 * Stops at the first token that cannot continue the expression (also an identifier right after
 * an operand, such as "to" in a for loop) and leaves that token unconsumed.
 *
 * @param[in,out] p Pointer to parser structure
 * @return root of the expression subtree (the caller frees it with ast_dtor), or nullptr on an
 * error, which is printed (ERR_SYNTACTIC or ERR_INTERNAL) and leaves nothing allocated
 */
ast_node_t parse_expression(parser_t *p);

#endif
