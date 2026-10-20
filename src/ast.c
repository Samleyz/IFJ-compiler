/**
 * @file ast.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of abstract syntax tree operations
 * @date 2026-10-20
 */

#include "ast.h"
#include "ast_private.h"

ast_node_t create_node(void) {
	// TODO: implement node creation
	return nullptr;
}

void free_node(ast_node_t node) {
	// TODO: implement single node cleanup
	(void)node;
}

ast_node_t ast_dtor(ast_node_t root) {
	// TODO: implement AST destruction
	(void)root;
	return nullptr;
}
