/**
 * @file code_gen.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions resposible for skeleton, function and extension generation
 * @date 2026-11-08
 */

#include "code_gen.h"
#include "ast.h"
#include "code_gen_private.h"
#include "instruction_set.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static code_gen_t code_generator;

/**
 * @brief Emits the IFJcode26 header and the program bootstrap sequence
 *
 * Finds the entry function annotated with @main
 * calls the main function, and terminates the program with EXIT 0.
 *
 * @param[in] root Root of the AST program
 * @return true on success, false otherwise
 */
static bool generate_prologue(ast_node_t root);

/**
 * @brief Generates code for extension methods
 *
 * @param[in] current_node AST node containing extension definition
 * @return true on success, false otherwise
 */
static bool generate_extension(ast_node_t current_node);

/**
 * @brief Generates code for a function definition
 *
 * @param[in] current_node AST node of the function definition
 * @return true on success, false otherwise
 */
static bool generate_function_def(ast_node_t current_node);

/**
 * @brief Generates definitions of IFJcode26 built-in functions
 *
 * @return true on success, false otherwise
 */
bool generate_builtins_definitions(void);

void code_gen_setup(FILE *output_file) {
	instruction_set_init(output_file);
	code_generator.label_counter = 0;
	code_generator.output_file = output_file;
}

bool generate_builtins_definitions(void) {
	// TODO: implement built-in functions
	return true;
}

bool generate_program(ast_node_t root) {
	if (!generate_prologue(root)) {
		return false;
	}
	if (!generate_builtins_definitions()) {
		return false;
	}

	ast_node_t current_node = root->as.program.items.head;
	while (current_node) { // cycle over all items in the program
		if (current_node->type == NODE_FUNC_DEF) {
			if (!generate_function_def(current_node)) {
				return false;
			}
		} else if (current_node->type == NODE_EXTENSION) {
			if (!generate_extension(current_node)) {
				return false;
			}
		}

		current_node = current_node->next;
	}

	return true;
}

bool generate_extension(ast_node_t current_node) {
	ast_node_t curr_node = current_node->as.extension.methods.head;
	while (curr_node) { // cycle over all extension methods
		if (!generate_function_def(curr_node)) {
			return false;
		}
		curr_node = curr_node->next;
	}

	return true;
}

bool generate_function_def(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

bool generate_prologue(ast_node_t root) {
	fprintf(code_generator.output_file, ".IFJcode26\n\n");

	// find the @main entry function
	char *main_label = nullptr;
	ast_node_t current_node = root->as.program.items.head;
	while (current_node) {
		if (current_node->type == NODE_FUNC_DEF && current_node->as.func_def.is_main) {
			main_label = current_node->as.func_def.name;
			break;
		}
		current_node = current_node->next;
	}

	if (!main_label) {
		return false;
	}

	// prepare the prefixed label for main function
	char *pref_main_label = get_prefixed_name("$$", main_label);

	if (!pref_main_label) {
		return false;
	}

	operand_t call_operand = {.operand_type = OP_TYPE_LABEL,
							  .frame = FT_GLOBAL,
							  .value = {.label_name = pref_main_label}};

	// call main function
	if (!print_inst_1(INST_CALL, call_operand)) {
		free(pref_main_label);
		return false;
	}

	operand_t exit_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_GLOBAL, .value = {.int_val = 0}};

	// exit program with return code 0
	if (!print_inst_1(INST_EXIT, exit_operand)) {
		free(pref_main_label);
		return false;
	}

	free(pref_main_label);

	fputc('\n', code_generator.output_file);

	return true;
}

char *get_prefixed_name(const char *prefix, const char *name) {
	if (!prefix || !name) {
		return nullptr;
	}

	// allocate space for prefix + name + null terminator
	size_t dest_size = strlen(prefix) + strlen(name) + 1;
	char *buffer = malloc(dest_size * sizeof(char));

	if (!buffer) {
		return nullptr;
	}

	snprintf(buffer, dest_size, "%s%s", prefix, name);

	return buffer;
}
