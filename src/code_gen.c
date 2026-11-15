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
 * Defines the global variable used to discard values, finds the entry function annotated with
 * @main, calls the main function, and terminates the program with EXIT 0.
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
 * @brief Recursively generates parameter definitions and pops in reverse order
 *
 * @param[in] param Current formal parameter node
 * @return true on success, false otherwise
 */
static bool generate_reverse_params(ast_node_t param);

/**
 * @brief Recursively generates parameter lists in reverse order
 *
 * @param[in] param_list Current parameter list node
 * @return true on success, false otherwise
 */
static bool generate_reverse_paramlists(ast_node_t param_list);

void code_gen_setup(FILE *output_file) {
	instruction_set_init(output_file);
	code_generator.label_counter = 0;
	code_generator.output_file = output_file;
}

void generate_function_prologue(const char *label_name) {
	print_comment("function");
	operand_t label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = label_name}};
	print_inst_1(INST_LABEL, label);
	print_inst_0(INST_CREATEFRAME);
	print_inst_0(INST_PUSHFRAME);
}

void generate_function_epilogue(void) {
	print_inst_0(INST_POPFRAME);
	print_inst_0(INST_RETURN);
	fputc('\n', code_generator.output_file);
}

bool generate_program(ast_node_t root) {
	if (!generate_prologue(root)) {
		return false;
	}
	generate_builtins_definitions();

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

static bool generate_ext_def(ast_node_t current_node, const char *receiver_name,
							 const char *receiver_type) {
	// Function label
	char *ext_def_name_half = get_modified_name("$$", receiver_type, "$");
	if (!ext_def_name_half) {
		return false;
	}

	char *ext_def_name = get_modified_name(ext_def_name_half, current_node->as.func_def.name, "");
	free(ext_def_name_half);
	if (!ext_def_name) {
		return false;
	}

	generate_function_prologue(ext_def_name);
	free(ext_def_name);

	// Retrieve arguments from stack into local variables
	print_comment("parameters");
	if (current_node->as.func_def.has_parens) {
		ast_node_t current_param_list = current_node->as.func_def.param_lists.head;
		if (!generate_reverse_paramlists(current_param_list)) {
			return false;
		}
	}

	operand_t parameter = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = receiver_name}};

	print_inst_1(INST_DEFVAR, parameter);
	print_inst_1(INST_POPS, parameter);

	print_comment("locals");
	if (!generate_locals_decl(current_node->as.func_def.body)) {
		return false;
	}

	// Function body
	print_comment("body");
	if (current_node->as.func_def.body) {
		if (!generate_function_body(current_node->as.func_def.body)) {
			return false;
		}
	}

	return true;
}

static bool generate_extension(ast_node_t current_node) {
	ast_node_t curr_node = current_node->as.extension.methods.head;
	while (curr_node) { // cycle over all extension methods
		if (!generate_ext_def(curr_node, current_node->as.extension.unique_receiver_name,
							  current_node->as.extension.receiver_type)) {
			return false;
		}
		curr_node = curr_node->next;
	}

	return true;
}

static bool generate_reverse_params(ast_node_t param) {
	if (!param) {
		return true;
	}

	// Unwind to the end first so parameters are popped in LIFO order
	if (!generate_reverse_params(param->next)) {
		return false;
	}

	operand_t parameter = {.operand_type = OP_TYPE_VAR,
						   .frame = FT_LOCAL,
						   .value = {.var_name = param->as.param.unique_name}};

	print_inst_1(INST_DEFVAR, parameter);
	print_inst_1(INST_POPS, parameter);

	return true;
}

static bool generate_reverse_paramlists(ast_node_t param_list) {
	if (!param_list) {
		return true;
	}

	if (!generate_reverse_paramlists(param_list->next)) {
		return false;
	}

	if (!generate_reverse_params(param_list->as.param_list.params.head)) {
		return false;
	}

	return true;
}

static bool generate_function_def(ast_node_t current_node) {
	// Function label
	char *function_name = get_modified_name("$$", current_node->as.func_def.name, "");
	if (!function_name) {
		return false;
	}

	generate_function_prologue(function_name);
	free(function_name);

	// Retrieve arguments from stack into local variables
	print_comment("parameters");
	if (current_node->as.func_def.has_parens) {
		ast_node_t current_param_list = current_node->as.func_def.param_lists.head;
		if (!generate_reverse_paramlists(current_param_list)) {
			return false;
		}
	}

	print_comment("locals");
	if (!generate_locals_decl(current_node->as.func_def.body)) {
		return false;
	}

	// Function body
	print_comment("body");
	if (current_node->as.func_def.body) {
		if (!generate_function_body(current_node->as.func_def.body)) {
			return false;
		}
	}

	return true;
}

static bool generate_prologue(ast_node_t root) {
	fprintf(code_generator.output_file, ".IFJcode26\n\n");

	operand_t operand_dis = {
		.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%discard"}};

	print_inst_1(INST_DEFVAR, operand_dis);

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
	char *pref_main_label = get_modified_name("$$", main_label, "");

	if (!pref_main_label) {
		return false;
	}

	operand_t call_operand = {.operand_type = OP_TYPE_LABEL,
							  .frame = FT_NO_FRAME,
							  .value = {.label_name = pref_main_label}};

	// call main function
	print_inst_1(INST_CALL, call_operand);
	free(pref_main_label);

	operand_t exit_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 0}};

	// exit program with return code 0
	print_inst_1(INST_EXIT, exit_operand);

	fputc('\n', code_generator.output_file);

	return true;
}

char *get_modified_name(const char *prefix, const char *name, const char *suffix) {
	if (!prefix || !name || !suffix) {
		return nullptr;
	}

	// allocate space for prefix + name + null terminator
	size_t dest_size = strlen(suffix) + strlen(prefix) + strlen(name) + 1;
	char *buffer = malloc(dest_size * sizeof(char));

	if (!buffer) {
		return nullptr;
	}

	snprintf(buffer, dest_size, "%s%s%s", prefix, name, suffix);

	return buffer;
}

FILE *gen_get_output_file(void) { return code_generator.output_file; }

char *get_unique_label(const char *prefix, const char *name, const char *suffix) {
	if (!prefix || !name || !suffix) {
		return nullptr;
	}
	int label_len =
		snprintf(nullptr, 0, "%s%s$%d%s", prefix, name, code_generator.label_counter, suffix);
	if (label_len < 0) {
		return nullptr;
	}

	label_len = label_len + 1;

	char *buffer = malloc(label_len * sizeof(char));

	if (!buffer) {
		return nullptr;
	}

	snprintf(buffer, label_len, "%s%s$%d%s", prefix, name, code_generator.label_counter, suffix);

	code_generator.label_counter++;
	return buffer;
}
