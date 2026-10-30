/**
 * @file ast.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Implementation of abstract syntax tree operations
 * @date 2026-10-20
 */

#include "ast.h"
#include <stdlib.h>

static ast_node_t *alloc_nodes = NULL; // tracking inflating arr
static int alloc_cnt = 0;			   // tracking inflating arr item count
static int alloc_cap = 0;			   // tracking inflating arr capacity

/**
 * @brief static function to manage pointer tracking for tree destruction
 * @param ptr node to be tracked
 * @return true - all good
 * @return false - realloc failure :P
 */
static bool track_ptr(ast_node_t ptr) {
	if (alloc_cnt >= alloc_cap) {

		int new_cap = (alloc_cap == 0) ? 128 : alloc_cap * 2; // doubling cap (starting with 128)
		ast_node_t *new_arr = realloc(alloc_nodes, new_cap * sizeof(ast_node_t));
		if (!new_arr) {
			return false; // buy more RAM
		}

		alloc_nodes = new_arr;
		alloc_cap = new_cap;
	}

	alloc_nodes[alloc_cnt] = ptr;
	alloc_cnt++;
	return true;
}

ast_node_t create_node(node_type n_type) {
	ast_node_t node = malloc(sizeof(ast_node));

	if (!node) {
		return NULL;
	}

	if (!track_ptr(node)) {
		free(node);
		return NULL;
	}

	node->type = n_type;
	node->child_node_1 = NULL;
	node->child_node_2 = NULL;
	node->child_node_3 = NULL;

	return node;
}

ast_node_t ast_dtor(void) {
	// freeing items one by one
	for (int node_idx = 0; node_idx < alloc_cnt; node_idx++) {

		ast_node_t node = alloc_nodes[node_idx];
		if (node->type == NODE_IDENTIFIER || node->type == NODE_STRING_LITERAL) {
			free(node->data.string_val);
		}

		free(node);
	}

	// cleaning up the tracking array
	free(alloc_nodes);
	alloc_nodes = NULL;
	alloc_cnt = 0;
	alloc_cap = 0;
	return NULL;
}
