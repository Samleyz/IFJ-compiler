/**
 * @file code_gen_expr.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions responsible for expression generation
 * @date 2026-11-08
 */

#include "ast.h"
#include "code_gen_private.h"
#include "instruction_set.h"
#include "token.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Names of the types that can be a receiver of an extension method, indexed by data_type_t
 *
 * The names are used in the labels of extension methods ($$Type$name). Types other than Int,
 * Double, String and Bool have no entry, so the index has to be checked first.
 */
static const char *const type_names[] = {
	[TYPE_INT] = "Int",		 [TYPE_DOUBLE] = "Double", [TYPE_STRING] = "String",
	[TYPE_BOOL] = "Boolean", [TYPE_UNIT] = "Unit",
};

/**
 * @brief First temporary global variable (%tmp_1), declared in the program prologue
 *
 * It may hold a value only until the next generated expression or CALL.
 */
static const operand_t operand_tmp_1 = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%tmp_1"}};
/**
 * @brief Second temporary global variable (%tmp_2), declared in the program prologue
 *
 * It may hold a value only until the next generated expression or CALL.
 */
static const operand_t operand_tmp_2 = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%tmp_2"}};
/** @brief Integer constant -1 */
static const operand_t int_neg1_operand = {
	.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = -1}};
/** @brief Double constant -1 */
static const operand_t double_neg1_operand = {
	.operand_type = OP_TYPE_FLOAT, .frame = FT_NO_FRAME, .value = {.double_val = -1}};
/** @brief Empty string constant, the value of an interpolated string without any part */
static const operand_t string_empty_operand = {
	.operand_type = OP_TYPE_STRING, .frame = FT_NO_FRAME, .value = {.string_val = ""}};

/** @brief Boolean constant true */
static const operand_t bool_true_operand = {
	.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};
/** @brief Boolean constant false */
static const operand_t bool_false_operand = {
	.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = false}};

/**
 * @brief Pushes an Int literal on the stack
 *
 * @param[in] current_node NODE_LIT_INT node
 */
static void generate_int_push(ast_node_t current_node);

/**
 * @brief Pushes a Double literal on the stack
 *
 * @param[in] current_node NODE_LIT_DOUBLE node
 */
static void generate_double_push(ast_node_t current_node);

/**
 * @brief Pushes a String literal on the stack
 *
 * @param[in] current_node NODE_LIT_STRING node
 */
static void generate_string_push(ast_node_t current_node);

/**
 * @brief Pushes a Boolean literal on the stack
 *
 * @param[in] current_node NODE_LIT_BOOL node
 */
static void generate_bool_push(ast_node_t current_node);

/**
 * @brief Pushes the Unit value (nil) on the stack
 */
static void generate_unit_push(void);

/**
 * @brief Pushes the value of a local variable on the stack
 *
 * @param[in] current_node NODE_IDENTIFIER node, its unique_name is the name of the variable
 */
static void generate_var_push(ast_node_t current_node);

/**
 * @brief Generates the short-circuit operators && and ||
 *
 * The right operand is evaluated only when the left one does not decide the result. Both paths
 * leave exactly one Boolean on the stack.
 *
 * @param[in] current_node NODE_BINARY_OP node with the operator && or ||
 * @return true on success, false otherwise
 */
static bool generate_logical_op(ast_node_t current_node);

/**
 * @brief Generates a binary operation, the result is left on the stack
 *
 * Dispatches to generate_logical_op, generate_relation_op or generate_arithmetic_op.
 *
 * @param[in] current_node NODE_BINARY_OP node
 * @return true on success, false otherwise
 */
static bool generate_binary_op(ast_node_t current_node);

/**
 * @brief Generates a unary operation (-, + or !), the result is left on the stack
 *
 * @param[in] current_node NODE_UNARY_OP node
 * @return true on success, false otherwise
 */
static bool generate_unary_op(ast_node_t current_node);

/**
 * @brief Generates a call of a function or of an extension method
 *
 * The callee is either NODE_IDENTIFIER (a function, label $$name) or NODE_MEMBER_ACCESS (an
 * extension method, label $$Type$name, the receiver is pushed as the first argument). The
 * arguments are evaluated from left to right and the result is left on the stack.
 *
 * @param[in] current_node NODE_FUNC_CALL node
 * @return true on success, false otherwise
 */
static bool generate_function_call(ast_node_t current_node);

/**
 * @brief Generates a member access without a call
 *
 * Built-in members (toString, toInt, floor, length) call the matching built-in function, any
 * other member is an extension method without parameters.
 *
 * @param[in] current_node NODE_MEMBER_ACCESS node
 * @return true on success, false otherwise
 */
static bool generate_member_access(ast_node_t current_node);

/**
 * @brief Generates an interpolated string, the resulting String is left on the stack
 *
 * Each part is converted by $$toStr and the parts are concatenated from left to right.
 *
 * @param[in] current_node NODE_STRING_INTERP node
 * @return true on success, false otherwise
 */
static bool generate_string_interpolation(ast_node_t current_node);

/**
 * @brief Generates the implicit conversion of an Int to a Double
 *
 * @param[in] current_node NODE_TYPE_CAST node, its target type has to be TYPE_DOUBLE
 * @return true on success, false otherwise
 */
static bool generate_type_cast(ast_node_t current_node);

bool generate_expression(ast_node_t current_node) {
	switch (current_node->type) {
	case NODE_LIT_INT:
		generate_int_push(current_node);
		break;
	case NODE_LIT_DOUBLE:
		generate_double_push(current_node);
		break;
	case NODE_LIT_STRING:
		generate_string_push(current_node);
		break;
	case NODE_LIT_BOOL:
		generate_bool_push(current_node);
		break;
	case NODE_LIT_UNIT:
		generate_unit_push();
		break;
	case NODE_IDENTIFIER:
		generate_var_push(current_node);
		break;
	case NODE_BINARY_OP:
		return generate_binary_op(current_node);
	case NODE_UNARY_OP:
		return generate_unary_op(current_node);
	case NODE_FUNC_CALL:
		return generate_function_call(current_node);
	case NODE_MEMBER_ACCESS:
		return generate_member_access(current_node);
	case NODE_STRING_INTERP:
		return generate_string_interpolation(current_node);
	case NODE_IF:
		return generate_if(current_node);
	case NODE_TYPE_CAST:
		return generate_type_cast(current_node);
	default:
		return false;
	}

	return true;
}

static void generate_int_push(ast_node_t current_node) {
	operand_t int_operand = {.operand_type = OP_TYPE_INT,
							 .frame = FT_NO_FRAME,
							 .value = {.int_val = current_node->as.int_val}};
	print_inst_1(INST_PUSHS, int_operand);
}

static void generate_double_push(ast_node_t current_node) {
	operand_t double_operand = {.operand_type = OP_TYPE_FLOAT,
								.frame = FT_NO_FRAME,
								.value = {.double_val = current_node->as.double_val}};
	print_inst_1(INST_PUSHS, double_operand);
}

static void generate_string_push(ast_node_t current_node) {
	operand_t string_operand = {.operand_type = OP_TYPE_STRING,
								.frame = FT_NO_FRAME,
								.value = {.string_val = current_node->as.string_val}};
	print_inst_1(INST_PUSHS, string_operand);
}

static void generate_bool_push(ast_node_t current_node) {
	operand_t bool_operand = {.operand_type = OP_TYPE_BOOL,
							  .frame = FT_NO_FRAME,
							  .value = {.bool_val = current_node->as.bool_val}};
	print_inst_1(INST_PUSHS, bool_operand);
}

static void generate_unit_push(void) {
	operand_t unit_operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
	print_inst_1(INST_PUSHS, unit_operand);
}

static void generate_var_push(ast_node_t current_node) {
	operand_t var_operand = {.operand_type = OP_TYPE_VAR,
							 .frame = FT_LOCAL,
							 .value = {.var_name = current_node->as.id.unique_name}};
	print_inst_1(INST_PUSHS, var_operand);
}

static bool generate_logical_op(ast_node_t current_node) {
	bool success = true;

	char *false_label = get_unique_label("$", "andstmt", "$false");
	char *end_label_and = get_unique_label("$", "andstmt", "$end");
	char *end_label_or = get_unique_label("$", "orstmt", "$end");
	char *true_label = get_unique_label("$", "orstmt", "$true");

	operand_t operand_false_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = false_label}};
	operand_t operand_end_label_and = {.operand_type = OP_TYPE_LABEL,
									   .frame = FT_NO_FRAME,
									   .value = {.label_name = end_label_and}};
	operand_t operand_end_label_or = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = end_label_or}};
	operand_t operand_true_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = true_label}};

	if (!false_label || !end_label_and || !end_label_or || !true_label) {
		success = false;
		goto cleanup;
	}
	if (current_node->as.binary_op.operator_type == OPERATOR_AND) {
		if (!generate_expression(current_node->as.binary_op.left)) {
			success = false;
			goto cleanup;
		}
		print_inst_1(INST_PUSHS, bool_false_operand);
		print_inst_1(INST_JUMPIFEQS, operand_false_label);

		if (!generate_expression(current_node->as.binary_op.right)) {
			success = false;
			goto cleanup;
		}
		print_inst_1(INST_JUMP, operand_end_label_and);
		print_inst_1(INST_LABEL, operand_false_label);
		print_inst_1(INST_PUSHS, bool_false_operand);
		print_inst_1(INST_LABEL, operand_end_label_and);
	} else {
		if (!generate_expression(current_node->as.binary_op.left)) {
			success = false;
			goto cleanup;
		}
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFEQS, operand_true_label);

		if (!generate_expression(current_node->as.binary_op.right)) {
			success = false;
			goto cleanup;
		}
		print_inst_1(INST_JUMP, operand_end_label_or);
		print_inst_1(INST_LABEL, operand_true_label);
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_LABEL, operand_end_label_or);
	}

cleanup:
	free(false_label);
	free(end_label_and);
	free(end_label_or);
	free(true_label);

	return success;
}

/**
 * @brief Generates a relational operator over the two operands already on the stack
 *
 * There are no stack instructions for <=, >= and !=, so they are the opposite comparison
 * followed by NOTS.
 *
 * @param[in] operator_type One of <, >, <=, >=, == and !=
 * @return true on success, false for any other operator
 */
static bool generate_relation_op(op_type_t operator_type) {
	switch (operator_type) {
	case OPERATOR_LESS_EQUAL:
		print_inst_0(INST_GTS);
		print_inst_0(INST_NOTS);
		break;
	case OPERATOR_GREATER_EQUAL:
		print_inst_0(INST_LTS);
		print_inst_0(INST_NOTS);
		break;
	case OPERATOR_EQUAL:
		print_inst_0(INST_EQS);
		break;
	case OPERATOR_NOT_EQUAL:
		print_inst_0(INST_EQS);
		print_inst_0(INST_NOTS);
		break;
	case OPERATOR_GREATER:
		print_inst_0(INST_GTS);
		break;
	case OPERATOR_LESS:
		print_inst_0(INST_LTS);
		break;
	default:
		return false;
	}

	return true;
}

/**
 * @brief Generates an arithmetic operator over the two operands already on the stack
 *
 * The + operator over Strings is a concatenation (CONCAT has no stack variant, so it works over
 * the temporary variables). The / operator is the integer division for Int and the decimal
 * division for Double.
 *
 * @param[in] current_node NODE_BINARY_OP node with the operator +, -, * or /
 * @return true on success, false otherwise
 */
static bool generate_arithmetic_op(ast_node_t current_node) {
	switch (current_node->as.binary_op.operator_type) {
	case OPERATOR_PLUS:
		if (current_node->as.binary_op.left->eval_type == TYPE_STRING ||
			current_node->as.binary_op.right->eval_type == TYPE_STRING) {

			print_inst_1(INST_POPS, operand_tmp_1);
			print_inst_1(INST_POPS, operand_tmp_2);
			print_inst_3(INST_CONCAT, operand_tmp_2, operand_tmp_2, operand_tmp_1);

			print_inst_1(INST_PUSHS, operand_tmp_2);
		} else {
			print_inst_0(INST_ADDS);
		}
		break;
	case OPERATOR_STAR:
		print_inst_0(INST_MULS);
		break;
	case OPERATOR_MINUS:
		print_inst_0(INST_SUBS);
		break;
	case OPERATOR_SLASH:
		if (current_node->as.binary_op.left->eval_type == TYPE_DOUBLE ||
			current_node->as.binary_op.right->eval_type == TYPE_DOUBLE) {
			print_inst_0(INST_DIVS);
		} else {
			print_inst_0(INST_IDIVS);
		}
		break;
	default:
		return false;
	}
	return true;
}

static bool generate_binary_op(ast_node_t current_node) {
	if (current_node->as.binary_op.operator_type == OPERATOR_AND ||
		current_node->as.binary_op.operator_type == OPERATOR_OR) {
		return generate_logical_op(current_node);
	}

	if (!generate_expression(current_node->as.binary_op.left)) {
		return false;
	}

	if (!generate_expression(current_node->as.binary_op.right)) {
		return false;
	}

	switch (current_node->as.binary_op.operator_type) {
	// relation
	case OPERATOR_LESS_EQUAL:
	case OPERATOR_GREATER_EQUAL:
	case OPERATOR_EQUAL:
	case OPERATOR_NOT_EQUAL:
	case OPERATOR_GREATER:
	case OPERATOR_LESS:
		return generate_relation_op(current_node->as.binary_op.operator_type);

	// arithmetic
	case OPERATOR_PLUS:
	case OPERATOR_STAR:
	case OPERATOR_MINUS:
	case OPERATOR_SLASH:
		return generate_arithmetic_op(current_node);
	default:
		return false;
	}

	return true;
}

static bool generate_unary_op(ast_node_t current_node) {
	if (!generate_expression(current_node->as.unary_op.operand)) {
		return false;
	}

	switch (current_node->as.unary_op.operator_type) {
	case OPERATOR_MINUS:
		if (current_node->as.unary_op.operand->eval_type == TYPE_INT) {
			print_inst_1(INST_PUSHS, int_neg1_operand);
			print_inst_0(INST_MULS);
		} else {
			print_inst_1(INST_PUSHS, double_neg1_operand);
			print_inst_0(INST_MULS);
		}
		break;
	case OPERATOR_PLUS:
		break;
	case OPERATOR_EX_MARK:
		print_inst_0(INST_NOTS);
		break;
	default:
		return false;
	}
	return true;
}

/**
 * @brief Generates all arguments of a call, so that they are on the stack
 *
 * The argument lists and the arguments in them are evaluated from left to right.
 *
 * @param[in] current_node NODE_FUNC_CALL node
 * @return true on success, false otherwise
 */
static bool func_call_push_args(ast_node_t current_node) {
	ast_node_t current_arg_list = current_node->as.func_call.arg_lists.head;
	while (current_arg_list) {
		ast_node_t current_arg = current_arg_list->as.arg_list.args.head;
		while (current_arg) {
			if (!generate_expression(current_arg)) {
				return false;
			}
			current_arg = current_arg->next;
		}
		current_arg_list = current_arg_list->next;
	}

	return true;
}

static bool generate_plain_call(ast_node_t current_node) {
	if (current_node->as.func_call.has_parens) {
		if (!func_call_push_args(current_node)) {
			return false;
		}
	}

	char *function_label =
		get_modified_name("$$", current_node->as.func_call.callee->as.id.name, "");
	if (!function_label) {
		return false;
	}

	operand_t function_name = {.operand_type = OP_TYPE_LABEL,
							   .frame = FT_NO_FRAME,
							   .value = {.label_name = function_label}};
	print_inst_1(INST_CALL, function_name);

	free(function_label);
	return true;
}

static bool generate_method_call(ast_node_t current_node) {
	if (!generate_expression(current_node->as.func_call.callee->as.member_access.target)) {
		return false;
	}

	if (current_node->as.func_call.has_parens) {
		if (!func_call_push_args(current_node)) {
			return false;
		}
	}

	unsigned type_idx = current_node->as.func_call.callee->as.member_access.target->eval_type;

	if (type_idx < TYPE_INT || type_idx > TYPE_UNIT) {
		return false;
	}

	char *function_label_tmp = get_modified_name("$$", type_names[type_idx], "$");
	char *function_label = get_modified_name(
		function_label_tmp, current_node->as.func_call.callee->as.member_access.member_name, "");
	if (!function_label || !function_label_tmp) {
		free(function_label);
		free(function_label_tmp);
		return false;
	}

	operand_t function_name = {.operand_type = OP_TYPE_LABEL,
							   .frame = FT_NO_FRAME,
							   .value = {.label_name = function_label}};
	print_inst_1(INST_CALL, function_name);

	free(function_label);
	free(function_label_tmp);
	return true;
}

static bool generate_function_call(ast_node_t current_node) {
	if (current_node->as.func_call.callee->type == NODE_IDENTIFIER) {
		if (!generate_plain_call(current_node)) {
			return false;
		}
	} else if (current_node->as.func_call.callee->type == NODE_MEMBER_ACCESS) {
		if (!generate_method_call(current_node)) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

/**
 * @brief Checks whether a member access is one of the built-in members
 *
 * @param[in] current_node NODE_MEMBER_ACCESS node
 * @return true for toString, toInt, length and floor, false otherwise
 */
static bool is_builtin_member(ast_node_t current_node) {
	return (bool)(strcmp(current_node->as.member_access.member_name, "toString") == 0 ||
				  strcmp(current_node->as.member_access.member_name, "toInt") == 0 ||
				  strcmp(current_node->as.member_access.member_name, "length") == 0 ||
				  strcmp(current_node->as.member_access.member_name, "floor") == 0);
}

/** @brief Names of the built-in members as they are written in a program */
static char *builtin_members[] = {"toString", "toInt", "floor", "length"};

/** @brief Names of the built-in functions behind the members, in the same order */
static char *builtin_functions[] = {"toStr", "toInt", "floor", "strLength"};

/**
 * @brief Maps the name of a built-in member to the name of the built-in function behind it
 *
 * @param[in] function_name Name of the member, such as "toString"
 * @return Name of the function, such as "toStr", or nullptr if it is not a built-in member
 */
static char *map_function_name(const char *function_name) {
	for (int i = 0; i < 4; i++) {
		if (strcmp(function_name, builtin_members[i]) == 0) {
			return builtin_functions[i];
		}
	}

	return nullptr;
}

static bool generate_member_access(ast_node_t current_node) {
	char *function_label = nullptr;
	char *function_label_tmp = nullptr;

	bool success = true;

	if (!generate_expression(current_node->as.member_access.target)) {
		success = false;
		goto cleanup;
	}

	if (is_builtin_member(current_node)) {
		char *function_name = map_function_name(current_node->as.member_access.member_name);

		function_label = get_modified_name("$$", function_name, "");
		if (!function_label) {
			success = false;
			goto cleanup;
		}

		operand_t function_name_op = {.operand_type = OP_TYPE_LABEL,
									  .frame = FT_NO_FRAME,
									  .value = {.label_name = function_label}};
		print_inst_1(INST_CALL, function_name_op);

	} else {
		unsigned type_idx = current_node->as.member_access.target->eval_type;

		if (type_idx < TYPE_INT || type_idx > TYPE_UNIT) {
			success = false;
			goto cleanup;
		}

		function_label_tmp = get_modified_name("$$", type_names[type_idx], "$");
		function_label =
			get_modified_name(function_label_tmp, current_node->as.member_access.member_name, "");
		if (!function_label || !function_label_tmp) {
			success = false;
			goto cleanup;
		}

		operand_t function_name = {.operand_type = OP_TYPE_LABEL,
								   .frame = FT_NO_FRAME,
								   .value = {.label_name = function_label}};
		print_inst_1(INST_CALL, function_name);
	}

cleanup:
	free(function_label);
	free(function_label_tmp);
	return success;
}

static bool generate_string_interpolation(ast_node_t current_node) {
	char *to_str_label = get_modified_name("$$", "toStr", "");
	if (!to_str_label) {
		return false;
	}
	operand_t operand_to_str_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = to_str_label}};
	ast_node_t curr_part = current_node->as.string_interp.parts.head;

	if (curr_part == nullptr) {
		print_inst_1(INST_PUSHS, string_empty_operand);
		free(to_str_label);
		return true;
	}

	while (curr_part) {
		if (!generate_expression(curr_part)) {
			free(to_str_label);
			return false;
		}

		print_inst_1(INST_CALL, operand_to_str_label);
		if (curr_part != current_node->as.string_interp.parts.head) {
			print_inst_1(INST_POPS, operand_tmp_2);
			print_inst_1(INST_POPS, operand_tmp_1);
			print_inst_3(INST_CONCAT, operand_tmp_1, operand_tmp_1, operand_tmp_2);
			print_inst_1(INST_PUSHS, operand_tmp_1);
		}

		curr_part = curr_part->next;
	}

	free(to_str_label);
	return true;
}

static bool generate_type_cast(ast_node_t current_node) {
	if (current_node->as.type_cast.target_type != TYPE_DOUBLE) {
		return false;
	}

	if (!generate_expression(current_node->as.type_cast.operand)) {
		return false;
	}

	print_inst_0(INST_INT2FLOATS);
	return true;
}
