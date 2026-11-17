/**
 * @file code_gen_builtins.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the IFJcode26 built-in runtime functions
 * @date 2026-11-09
 */

#include "code_gen_private.h"
#include "instruction_set.h"
#include <stdio.h>

/**
 * @brief Generates built-in function strLength ($$strLength)
 */
static void generate_builtin_strlength(void);

/**
 * @brief Generates built-in function print ($$print)
 */
static void generate_builtin_print(void);

/**
 * @brief Generates built-in function readLine ($$readLine)
 */
static void generate_builtin_readLine(void);

/**
 * @brief Generates built-in function readInt ($$readInt)
 */
static void generate_builtin_readInt(void);

/**
 * @brief Generates built-in function readDouble ($$readDouble)
 */
static void generate_builtin_readDouble(void);

/**
 * @brief Generates built-in function readBoolean ($$readBoolean)
 */
static void generate_builtin_readBoolean(void);

/**
 * @brief Generates built-in function toStr ($$toStr)
 */
static void generate_builtin_toStr(void);

/**
 * @brief Generates built-in function toInt ($$toInt)
 */
static void generate_builtin_toInt(void);

/**
 * @brief Generates built-in function subStr ($$subStr)
 */
static void generate_builtin_subStr(void);

/**
 * @brief Generates built-in function ord ($$ord)
 */
static void generate_builtin_ord(void);

/**
 * @brief Generates built-in function chr ($$chr)
 */
static void generate_builtin_chr(void);

/**
 * @brief Generates built-in function floor ($$floor), the Double member .floor
 */
static void generate_builtin_floor(void);

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

// Tool: Gemini
// Built-in print function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_print function emitting IFJcode26 for print.
// Note: Generated WRITE instructions and Unit () handling verified against IFJ26 specification.
static void generate_builtin_print(void) {
	generate_function_prologue("$$print");

	operand_t value_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "value"}};
	operand_t nil_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t unit_string = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "()"}};

	operand_t unit_label = {.operand_type = OP_TYPE_LABEL,
							.frame = FT_NO_FRAME,
							.value = {.label_name = "$print$unit"}};
	operand_t end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$print$end"}};

	print_inst_1(INST_DEFVAR, value_operand);
	print_inst_1(INST_POPS, value_operand);

	// If value is nil (Unit), print "()"
	print_inst_3(INST_JUMPIFEQ, unit_label, value_operand, nil_operand);

	// Otherwise print standard value directly via WRITE
	print_inst_1(INST_WRITE, value_operand);
	print_inst_1(INST_JUMP, end_label);

	// Unit branch
	print_inst_1(INST_LABEL, unit_label);
	print_inst_1(INST_WRITE, unit_string);

	// Epilogue & return Unit (nil)
	print_inst_1(INST_LABEL, end_label);
	print_inst_1(INST_PUSHS, nil_operand);

	generate_function_epilogue();
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
	operand_t nil_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {.operand_type = OP_TYPE_LABEL,
						  .frame = FT_NO_FRAME,
						  .value = {.label_name = "$readInt$ok"}};
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
	operand_t nil_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {.operand_type = OP_TYPE_LABEL,
						  .frame = FT_NO_FRAME,
						  .value = {.label_name = "$readDouble$ok"}};
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

// Tool: Gemini
// Built-in readBoolean function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_readBoolean function emitting IFJcode26 for readBoolean.
// Note: Generated instructions and EOF/error fallback to false verified against IFJ26
// specification.
static void generate_builtin_readBoolean(void) {
	generate_function_prologue("$$readBoolean");

	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t type_operand = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "bool"}};
	operand_t nil_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	operand_t ok_label = {.operand_type = OP_TYPE_LABEL,
						  .frame = FT_NO_FRAME,
						  .value = {.label_name = "$readBoolean$ok"}};
	operand_t false_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = false}};

	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_2(INST_READ, result_operand, type_operand);
	print_inst_3(INST_JUMPIFNEQ, ok_label, result_operand, nil_operand);
	print_inst_2(INST_MOVE, result_operand, false_operand);
	print_inst_1(INST_LABEL, ok_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

// Tool: Gemini
// Built-in toStr function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_toStr function emitting IFJcode26 for toStr.
// Note: Generated TYPE inspection and conversion branches for int, float, bool, nil, and string
// verified against IFJ26 specification.
static void generate_builtin_toStr(void) {
	generate_function_prologue("$$toStr");

	operand_t value_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "value"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t type_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "type"}};

	operand_t type_int_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "int"}};
	operand_t type_float_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "float"}};
	operand_t type_bool_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "bool"}};
	operand_t type_nil_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "nil"}};

	operand_t unit_str_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "()"}};
	operand_t str_true_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "true"}};
	operand_t str_false_operand = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = "false"}};

	operand_t true_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};

	operand_t int_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$toStr$int"}};
	operand_t float_label = {.operand_type = OP_TYPE_LABEL,
							 .frame = FT_NO_FRAME,
							 .value = {.label_name = "$toStr$float"}};
	operand_t bool_label = {.operand_type = OP_TYPE_LABEL,
							.frame = FT_NO_FRAME,
							.value = {.label_name = "$toStr$bool"}};
	operand_t bool_true_label = {.operand_type = OP_TYPE_LABEL,
								 .frame = FT_NO_FRAME,
								 .value = {.label_name = "$toStr$bool_true"}};
	operand_t nil_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$toStr$nil"}};
	operand_t end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$toStr$end"}};

	print_inst_1(INST_DEFVAR, value_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_DEFVAR, type_operand);
	print_inst_1(INST_POPS, value_operand);

	// Inspect dynamic type of argument
	print_inst_2(INST_TYPE, type_operand, value_operand);

	print_inst_3(INST_JUMPIFEQ, int_label, type_operand, type_int_operand);
	print_inst_3(INST_JUMPIFEQ, float_label, type_operand, type_float_operand);
	print_inst_3(INST_JUMPIFEQ, bool_label, type_operand, type_bool_operand);
	print_inst_3(INST_JUMPIFEQ, nil_label, type_operand, type_nil_operand);

	// String / default branch: copy directly
	print_inst_2(INST_MOVE, result_operand, value_operand);
	print_inst_1(INST_JUMP, end_label);

	// Int branch
	print_inst_1(INST_LABEL, int_label);
	print_inst_2(INST_INT2STR, result_operand, value_operand);
	print_inst_1(INST_JUMP, end_label);

	// Float branch
	print_inst_1(INST_LABEL, float_label);
	print_inst_2(INST_FLOAT2STR, result_operand, value_operand);
	print_inst_1(INST_JUMP, end_label);

	// Bool branch
	print_inst_1(INST_LABEL, bool_label);
	print_inst_3(INST_JUMPIFEQ, bool_true_label, value_operand, true_operand);
	print_inst_2(INST_MOVE, result_operand, str_false_operand);
	print_inst_1(INST_JUMP, end_label);
	print_inst_1(INST_LABEL, bool_true_label);
	print_inst_2(INST_MOVE, result_operand, str_true_operand);
	print_inst_1(INST_JUMP, end_label);

	// Nil / Unit branch
	print_inst_1(INST_LABEL, nil_label);
	print_inst_2(INST_MOVE, result_operand, unit_str_operand);

	// Normal return
	print_inst_1(INST_LABEL, end_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
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

// Tool: Gemini
// Built-in subStr function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_subStr function emitting IFJcode26 for subStr.
// Note: Generated runtime error 40 checks and loop-based substring extraction verified against
// IFJ26 specification.
static void generate_builtin_subStr(void) {
	generate_function_prologue("$$subStr");

	operand_t end_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "end"}};
	operand_t start_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "start"}};
	operand_t string_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "string"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t len_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "len"}};
	operand_t cond_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "cond"}};
	operand_t i_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "i"}};
	operand_t char_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "char"}};

	operand_t zero_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 0}};
	operand_t one_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 1}};
	operand_t err_code_operand = {.operand_type = OP_TYPE_INT,
								  .frame = FT_NO_FRAME,
								  .value = {.int_val = ERR_RUNTIME_SEMANTICS}};
	operand_t true_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};
	operand_t false_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = false}};
	operand_t empty_string = {
		.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = ""}};

	operand_t err_label = {.operand_type = OP_TYPE_LABEL,
						   .frame = FT_NO_FRAME,
						   .value = {.label_name = "$subStr$err"}};
	operand_t loop_label = {.operand_type = OP_TYPE_LABEL,
							.frame = FT_NO_FRAME,
							.value = {.label_name = "$subStr$loop"}};
	operand_t end_label = {.operand_type = OP_TYPE_LABEL,
						   .frame = FT_NO_FRAME,
						   .value = {.label_name = "$subStr$end"}};

	print_inst_1(INST_DEFVAR, end_operand);
	print_inst_1(INST_DEFVAR, start_operand);
	print_inst_1(INST_DEFVAR, string_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_DEFVAR, len_operand);
	print_inst_1(INST_DEFVAR, cond_operand);
	print_inst_1(INST_DEFVAR, i_operand);
	print_inst_1(INST_DEFVAR, char_operand);

	// Arguments are popped in LIFO order (end, start, string)
	print_inst_1(INST_POPS, end_operand);
	print_inst_1(INST_POPS, start_operand);
	print_inst_1(INST_POPS, string_operand);

	// Check: start < 0
	print_inst_3(INST_LT, cond_operand, start_operand, zero_operand);
	print_inst_3(INST_JUMPIFEQ, err_label, cond_operand, true_operand);

	// Check: end < 0
	print_inst_3(INST_LT, cond_operand, end_operand, zero_operand);
	print_inst_3(INST_JUMPIFEQ, err_label, cond_operand, true_operand);

	// Check: start > end
	print_inst_3(INST_GT, cond_operand, start_operand, end_operand);
	print_inst_3(INST_JUMPIFEQ, err_label, cond_operand, true_operand);

	// Check: start >= length (i.e. NOT start < length)
	print_inst_2(INST_STRLEN, len_operand, string_operand);
	print_inst_3(INST_LT, cond_operand, start_operand, len_operand);
	print_inst_3(INST_JUMPIFEQ, err_label, cond_operand, false_operand);

	// Check: end > length
	print_inst_3(INST_GT, cond_operand, end_operand, len_operand);
	print_inst_3(INST_JUMPIFEQ, err_label, cond_operand, true_operand);

	// Extraction loop
	print_inst_2(INST_MOVE, result_operand, empty_string);
	print_inst_2(INST_MOVE, i_operand, start_operand);

	print_inst_1(INST_LABEL, loop_label);
	print_inst_3(INST_LT, cond_operand, i_operand, end_operand);
	print_inst_3(INST_JUMPIFEQ, end_label, cond_operand, false_operand);

	print_inst_3(INST_GETCHAR, char_operand, string_operand, i_operand);
	print_inst_3(INST_CONCAT, result_operand, result_operand, char_operand);
	print_inst_3(INST_ADD, i_operand, i_operand, one_operand);
	print_inst_1(INST_JUMP, loop_label);

	// Normal return
	print_inst_1(INST_LABEL, end_label);
	print_inst_1(INST_PUSHS, result_operand);
	generate_function_epilogue();

	// Error 40 exit path
	print_inst_1(INST_LABEL, err_label);
	print_inst_1(INST_EXIT, err_code_operand);
	fputc('\n', gen_get_output_file());
}

// Tool: Gemini
// Built-in ord function code generation implemented with AI assistance.
// Prompt: Implement generate_builtin_ord function emitting IFJcode26 for ord.
// Note: Generated bounds checks (0 <= index < length) and STRI2INT verified against IFJ26
// specification.
static void generate_builtin_ord(void) {
	generate_function_prologue("$$ord");

	operand_t index_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "index"}};
	operand_t string_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "string"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t cond_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "cond"}};
	operand_t len_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "len"}};

	operand_t neg_one_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = -1}};
	operand_t zero_operand = {
		.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 0}};
	operand_t true_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};
	operand_t false_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = false}};
	operand_t end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$ord$end"}};

	print_inst_1(INST_DEFVAR, index_operand);
	print_inst_1(INST_DEFVAR, string_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_DEFVAR, cond_operand);
	print_inst_1(INST_DEFVAR, len_operand);

	// Arguments are popped in LIFO order (index was pushed last)
	print_inst_1(INST_POPS, index_operand);
	print_inst_1(INST_POPS, string_operand);

	// Default result is -1 for out-of-bounds or empty string
	print_inst_2(INST_MOVE, result_operand, neg_one_operand);

	// Check if index < 0
	print_inst_3(INST_LT, cond_operand, index_operand, zero_operand);
	print_inst_3(INST_JUMPIFEQ, end_label, cond_operand, true_operand);

	// Check if index < length
	print_inst_2(INST_STRLEN, len_operand, string_operand);
	print_inst_3(INST_LT, cond_operand, index_operand, len_operand);
	print_inst_3(INST_JUMPIFEQ, end_label, cond_operand, false_operand);

	// Valid index: get ASCII value
	print_inst_3(INST_STRI2INT, result_operand, string_operand, index_operand);

	print_inst_1(INST_LABEL, end_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
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

// Tool: Claude Code (Sonnet 5.5)
// Prompt: Write generate_builtin_floor, the IFJcode26 built-in $$floor for the Double member
// .floor (rounds down, also for negative numbers).
// Note: FLOAT2INT truncates towards zero, so 1.0 is subtracted when the truncated value is
// greater than the argument (negative numbers with a fractional part).
static void generate_builtin_floor(void) {
	generate_function_prologue("$$floor");

	operand_t param_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "double_num"}};
	operand_t int_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "truncated_int"}};
	operand_t result_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "result"}};
	operand_t less_operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = "is_less"}};

	operand_t one_operand = {
		.operand_type = OP_TYPE_FLOAT, .frame = FT_NO_FRAME, .value = {.double_val = 1.0}};
	operand_t false_operand = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = false}};
	operand_t end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = "$floor$end"}};

	print_inst_1(INST_DEFVAR, param_operand);
	print_inst_1(INST_DEFVAR, int_operand);
	print_inst_1(INST_DEFVAR, result_operand);
	print_inst_1(INST_DEFVAR, less_operand);
	print_inst_1(INST_POPS, param_operand);

	// truncate towards zero and convert back to Double
	print_inst_2(INST_FLOAT2INT, int_operand, param_operand);
	print_inst_2(INST_INT2FLOAT, result_operand, int_operand);

	// the argument is below the truncated value (negative with a fraction): subtract 1.0
	print_inst_3(INST_LT, less_operand, param_operand, result_operand);
	print_inst_3(INST_JUMPIFEQ, end_label, less_operand, false_operand);
	print_inst_3(INST_SUB, result_operand, result_operand, one_operand);

	print_inst_1(INST_LABEL, end_label);
	print_inst_1(INST_PUSHS, result_operand);

	generate_function_epilogue();
}

void generate_builtins_definitions(void) {
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
	generate_builtin_floor();
}
