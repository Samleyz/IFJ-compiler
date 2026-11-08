/**
 * @file code_gen_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Private declarations for code generation
 * @date 2026-11-08
 */

#ifndef CODE_GEN_PRIVATE_H
#define CODE_GEN_PRIVATE_H

#include "ast.h"
#include <stdio.h>

typedef struct code_generator {
	FILE *output_file;
	int label_counter;
} code_gen_t;

bool generate_expression(ast_node_t current_node);

bool generate_statement(ast_node_t current_node);

/**
 * @brief Creates a dynamically allocated string prefixed with given prefix
 *
 * @param[in] prefix Prefix string (e.g. "$$")
 * @param[in] name Base identifier name
 * @return Allocated string (caller is responsible for free()), or nullptr on error
 */
char *get_prefixed_name(const char *prefix, const char *name);

bool get_unique_label();

#endif
