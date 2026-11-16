/**
 * @file code_gen_expr.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions responsible for expression generation
 * @date 2026-11-08
 */

#include "ast.h"
#include "code_gen_private.h"
#include "instruction_set.h"

static void generate_int_push(ast_node_t current_node);

static void generate_double_push(ast_node_t current_node);

static void generate_string_push(ast_node_t current_node);

static void generate_bool_push(ast_node_t current_node);

static void generate_unit_push(void);

static void generate_var_push(ast_node_t current_node);

static bool generate_binary_op(ast_node_t current_node);

static bool generate_unary_op(ast_node_t current_node);

static bool generate_function_call(ast_node_t current_node);

static bool generate_member_access(ast_node_t current_node);

static bool generate_string_interpolation(ast_node_t current_node);

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

static bool generate_binary_op(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

static bool generate_unary_op(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

static bool generate_function_call(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

static bool generate_member_access(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

static bool generate_string_interpolation(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}

static bool generate_type_cast(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
}
