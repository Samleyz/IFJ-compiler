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
 * Starts at the current token of the parser. Supports operands (identifiers and literals),
 * prefix and binary operators with the priority and associativity of the specification,
 * groups, "()" as the Unit literal, function calls with any number of argument lists, and
 * "if" used as an expression (parsed by parse_if_statement).
 *
 * The expression ends at the first token that cannot continue it, for example "then", "do",
 * "else", ",", ";", "=", the end of the line, a ")" that closes something outside of the
 * expression, or an identifier right after an operand (this is how the words "to", "until" and
 * "by" of a for generator end the expression). That token stays the current token of the
 * parser, it is never consumed (the end of the line included), so the caller checks for it.
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to the root of the expression subtree, owned by the caller
 * (free it with ast_dtor), or nullptr
 * @note On a syntax error (also when there is no expression at all) the error is printed with
 * print_error(ERR_SYNTACTIC, ...) and nullptr is returned. The program is not terminated, the
 * caller decides how to continue. Everything built so far is freed, so nothing leaks.
 * @note The eval_type of the returned nodes is TYPE_UNKNOWN, semantic analysis fills it in.
 */
ast_node_t parse_expression(parser_t *p);

#endif
