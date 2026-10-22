/**
 * @file parser_grammar.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Declarations of grammar parsing functions (rules) for IFJ26 parser
 * @date 2026-10-22
 */

#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

#include "ast.h"
#include "parser_private.h"

/**
 * @brief Parses the program header rule
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to subtree, or nullptr
 */
ast_node_t parse_header(parser_t *p);

/**
 * @brief Parses the program body rule
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to subtree, or nullptr
 */
ast_node_t parse_body(parser_t *p);

/**
 * @brief Parses a block rule (indented block of statements)
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to subtree, or nullptr
 */
ast_node_t parse_block(parser_t *p);

/**
 * @brief Parses an if statement rule
 *
 * @param[in,out] p Pointer to parser structure
 * @return AST node pointer to subtree, or nullptr
 */
ast_node_t parse_if_statement(parser_t *p);

#endif
