/**
 * @file semantic_analysis.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of semantic analysis functions
 */

#include "error.h"
#include "semantic_analysis_private.h"

bool perform_semantic_analysis(ast_node_t root) {
	// TODO
}

stable_t prefill_symbol_table() {
	// TODO
}

bool first_pass(ast_node_t node, stable_t symtable) {
	assert(node && symtable && "Called first_pass on nullptr.");
	// TODO
}

bool second_pass_node(ast_node_t node, stable_t symtable) {
	assert(node && symtable && "Called second_pass_node on nullptr.");
	// TODO
}
