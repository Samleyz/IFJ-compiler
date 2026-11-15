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

#define ERR_RUNTIME_SEMANTICS 40

typedef struct code_generator {
	FILE *output_file;
	int label_counter;
} code_gen_t;

void generate_function_prologue(const char *label_name);

void generate_function_epilogue(void);

void generate_builtins_definitions(void);

bool generate_expression(ast_node_t current_node);

bool generate_function_body(ast_node_t current_node);

/**
 * @brief Returns the output file the code generator writes to
 *
 * @return Output file set by code_gen_setup (owned by the caller of code_gen_setup, do not close
 * it)
 */
FILE *gen_get_output_file(void);

/**
 * @brief Creates a dynamically allocated string prefixed with given prefix
 *
 * @param[in] prefix Prefix string (e.g. "$$")
 * @param[in] name Base identifier name
 * @return Allocated string (caller is responsible for free()), or nullptr on error
 */
char *get_modified_name(const char *prefix, const char *name, const char *suffix);

bool get_unique_label();

#endif
