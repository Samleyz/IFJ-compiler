/**
 * @file ast_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Internal declaration for ast
 * @date 2026-10-20
 */

#ifndef AST_PRIVATE_H
#define AST_PRIVATE_H

#include "ast.h"

typedef enum { NONE } node_type;

struct ast_node {
	// TODO abstract syntax tree node implementation
};

/**
 * @brief Frees a single AST node and its allocated resources
 *
 * @param[in] node AST node to be freed
 */
void free_node(ast_node_t node);

#endif
