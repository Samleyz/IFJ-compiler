/**
 * Implementation of a compiler for the imperative language IFJ26
 *
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

/**
 * @brief Generates the function label and creates and pushes its frame
 *
 * @param[in] label_name Name of the function label
 */
void generate_function_prologue(const char *label_name);

/**
 * @brief Generates the frame pop and the return from the function
 */
void generate_function_epilogue(void);

/**
 * @brief Generates definitions of all built-in functions
 */
void generate_builtins_definitions(void);

/**
 * @brief Generates code for an expression
 *
 * After the generated code is run, exactly one value (the value of the expression) is on the top
 * of the stack.
 *
 * @param[in] current_node Expression node (a literal, an identifier, an operation, a call, a
 * member access, an interpolated string, an if expression or a type cast)
 * @return true on success, false otherwise
 */
bool generate_expression(ast_node_t current_node);

/**
 * @brief Generates code for an if statement
 *
 * @param[in] current_node NODE_IF node
 * @return true on success, false otherwise
 */
bool generate_if(ast_node_t current_node);

/**
 * @brief Generates the body of a function
 *
 * @param[in] current_node Function body (NODE_BLOCK or NODE_RETURN)
 * @return true on success, false otherwise
 */
bool generate_function_body(ast_node_t current_node);

/**
 * @brief Emits DEFVAR for all local variables in a function body
 *
 * @param[in] current_node Function body, may be nullptr
 * @return true on success, false otherwise
 */
bool generate_locals_decl(ast_node_t current_node);

/**
 * @brief Returns the output file the code generator writes to
 *
 * @return Output file set by code_gen_setup (owned by the caller of code_gen_setup, do not close
 * it)
 */
FILE *gen_get_output_file(void);

/**
 * @brief Creates a dynamically allocated string made of the prefix, the name and the suffix
 *
 * @param[in] prefix Prefix string (e.g. "$$")
 * @param[in] name Base identifier name
 * @param[in] suffix Suffix string
 * @return Allocated string (caller is responsible for free()), or nullptr on error
 */
char *get_modified_name(const char *prefix, const char *name, const char *suffix);

/**
 * @brief Creates a dynamically allocated unique label and increments the label counter
 *
 * The label has the form prefix + name + "$" + counter + suffix.
 *
 * @param[in] prefix Prefix string
 * @param[in] name Base label name
 * @param[in] suffix Suffix string
 * @return Allocated string (must be freed), or nullptr on error
 */
char *get_unique_label(const char *prefix, const char *name, const char *suffix);

#endif
