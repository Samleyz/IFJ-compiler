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

/**
 * @brief Generates definitions of IFJcode26 built-in functions
 */
static void generate_builtins_definitions(void);

void code_gen_setup(FILE *output_file) {
	instruction_set_init(output_file);
	code_generator.label_counter = 0;
	code_generator.output_file = output_file;
}

static void generate_function_prologue(const char *label_name) {
	operand_t label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = label_name}};
	print_inst_1(INST_LABEL, label);
	print_inst_0(INST_CREATEFRAME);
	print_inst_0(INST_PUSHFRAME);
}

static void generate_function_epilogue(void) {
	print_inst_0(INST_POPFRAME);
	print_inst_0(INST_RETURN);
	putc('\n', code_generator.output_file);
}

static void generate_builtin_strlength(void) {
	generate_function_prologue("$$strLength");

	operand_t param_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "string"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};

	print_inst_1(INST_DEFVAR, param_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_POPS, param_operand);

	print_inst_2(INST_STRLEN, result_operand, param_operand);

	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

static void generate_builtin_print(void) {
	// TODO
}

// Tool: Gemini
// Built-in readLine function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_readLine function emitting IFJcode26 for readLine.
// Note: Generated instructions and EOF handling verified against IFJ26 specification.
static void generate_builtin_readLine(void) {
	generate_function_prologue("$$readLine");

	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t type_operand = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "string"}};
	operand_t nil_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {.operand_type = OP_TYPE_LABEL,
						  .frame = FT_NO_FRAME,
						  .value = {.label_name = "$readLine$ok"}};
	operand_t empty_string = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = ""}};

	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_2(INST_READ, result_operand, type_operand);
	print_inst_3(INST_JUMPIFNEQ, ok_label, result_operand, nil_operand);
	print_inst_2(INST_MOVE, result_operand, empty_string);
	print_inst_1(INST_LABEL, ok_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

// Tool: Gemini
// Built-in readInt function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_readInt function emitting IFJcode26 for readInt.
// Note: Generated instructions and EOF/error fallback to 0 verified against IFJ26 specification.
static void generate_builtin_readInt(void) {
	generate_function_prologue("$$readInt");

	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t type_operand = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "int"}};
	operand_t nil_operand = {
		.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$readInt$ok"}};
	operand_t zero_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 0}};

	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_2(INST_READ, result_operand, type_operand);
	print_inst_3(INST_JUMPIFNEQ, ok_label, result_operand, nil_operand);
	print_inst_2(INST_MOVE, result_operand, zero_operand);
	print_inst_1(INST_LABEL, ok_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

// Tool: Gemini
// Built-in readDouble function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_readDouble function emitting IFJcode26 for readDouble.
// Note: Generated instructions and EOF/error fallback to 0.0 verified against IFJ26 specification.
static void generate_builtin_readDouble(void) {
	generate_function_prologue("$$readDouble");

	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t type_operand = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "float"}};
	operand_t nil_operand = {
		.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$readDouble$ok"}};
	operand_t zero_operand = {
		.operand_type = OP_TYPE_FLOAT, .frame = FT_NO_FRAME, .value = {.double_val = 0.0}};

	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_2(INST_READ, result_operand, type_operand);
	print_inst_3(INST_JUMPIFNEQ, ok_label, result_operand, nil_operand);
	print_inst_2(INST_MOVE, result_operand, zero_operand);
	print_inst_1(INST_LABEL, ok_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

static void generate_builtin_readBoolean(void) {
	// TODO
}

static void generate_builtin_toStr(void) {
	// TODO
}

static void generate_builtin_toInt(void) {
	generate_function_prologue("$$toInt");

	operand_t param_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "double_num"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};

	print_inst_1(INST_DEFVAR, param_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_POPS, param_operand);

	print_inst_2(INST_FLOAT2INT, result_operand, param_operand);

	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

static void generate_builtin_subStr(void) {
	// TODO
}

static void generate_builtin_ord(void) {
	// TODO
}

static void generate_builtin_chr(void) {
	generate_function_prologue("$$chr");

	operand_t param_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "int_num"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};

	print_inst_1(INST_DEFVAR, param_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_POPS, param_operand);

	print_inst_2(INST_INT2CHAR, result_operand, param_operand);

	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

static void generate_builtins_definitions(void) {
	generate_builtin_strlength();
	generate_builtin_print();
	generate_builtin_readLine();
	generate_builtin_readInt();
	generate_builtin_readDouble();
	generate_builtin_readBoolean();
	generate_builtin_toStr();
	generate_builtin_toInt();
	generate_builtin_subStr();
	generate_builtin_ord();
	generate_builtin_chr();
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

static bool generate_reverse_params(ast_node_t param) {
	if (!param) {
		return true;
	}

	// Unwind to the end first so parameters are popped in LIFO order
	if (!generate_reverse_params(param->next)) {
		return false;
	}

	operand_t parameter = {
		.operand_type = OP_TYPE_VAR,
		.frame = FT_LOCAL,
		.value = {.var_name = param->as.param.name}}; // TODO: switch to sym_entry unique name

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

bool generate_function_def(ast_node_t current_node) {
	// Function label
	char *function_name = get_prefixed_name("$$", current_node->as.func_def.name);
	if (!function_name) {
		return false;
	}

	generate_function_prologue(function_name);
	free(function_name);

	// Retrieve arguments from stack into local variables
	if (current_node->as.func_def.has_parens) {
		ast_node_t current_param_list = current_node->as.func_def.param_lists.head;
		if (!generate_reverse_paramlists(current_param_list)) {
			return false;
		}
	}

	// Function body
	if (current_node->as.func_def.body) {
		if (!generate_statement(current_node->as.func_def.body)) {
			return false;
		}
	}

	// Epilogue
	generate_function_epilogue();

	return true;
}

static bool generate_prologue(ast_node_t root) {
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
