/**
 * @file code_gen_expr.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions responsible for expression generation
 * @date 2026-11-08
 */

#include "ast.h"
#include "code_gen_private.h"
#include "instruction_set.h"
#include <token.h>

static const operand_t operand_dis = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%discard"}};

static const operand_t operand_tmp_1 = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%tmp_1"}};
static const operand_t operand_tmp_2 = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%tmp_2"}};
static const operand_t operand_tmp_3 = {
	.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%tmp_3"}};

static void generate_int_push(ast_node_t current_node);

static void generate_double_push(ast_node_t current_node);

static void generate_string_push(ast_node_t current_node);

static void generate_bool_push(ast_node_t current_node);

static void generate_unit_push(void);

static void generate_var_push(ast_node_t current_node);

static bool generate_logical_op(ast_node_t current_node);

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

static bool generate_logical_op(ast_node_t current_node) {
	if (current_node->as.binary_op.operator_type == OPERATOR_AND) {

	} else {
	}
}

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
