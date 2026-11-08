/**
 * @file code_gen_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Private declarations for code generation
 * @date 2026-11-08
 */

#ifndef CODE_GEN_PRIVATE_H
#define CODE_GEN_PRIVATE_H

#include "ast.h"

void generate_prologue(void);

bool generate_extension(ast_node_t current_node);

bool generate_function(ast_node_t current_node);

bool generate_expression(ast_node_t current_node);

bool generate_statement(ast_node_t current_node);

// TODO all other generate functions for expample for block, var_def, assign etc.

bool get_unique_label();

bool generate_builtin_function(ast_node_t current_node);

#endif
