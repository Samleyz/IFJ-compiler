/**
 * @file code_gen_expr.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions responsible for expression generation
 * @date 2026-11-08
 */

#include "ast.h"
#include "code_gen_private.h"

static void generate_int_push(ast_node_t current_node);

static void generate_double_push(ast_node_t current_node);

static void generate_string_push(ast_node_t current_node);

static void generate_bool_push(ast_node_t current_node);

static void generate_unit_push(ast_node_t current_node);

static bool generate_var_push(ast_node_t current_node);

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
		generate_unit_push(current_node);
		break;
	case NODE_IDENTIFIER:
		return generate_var_push(current_node);
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
	// TODO
	(void)current_node;
}

static void generate_double_push(ast_node_t current_node) {
	// TODO
	(void)current_node;
}

static void generate_string_push(ast_node_t current_node) {
	// TODO
	(void)current_node;
}

static void generate_bool_push(ast_node_t current_node) {
	// TODO
	(void)current_node;
}

static void generate_unit_push(ast_node_t current_node) {
	// TODO
	(void)current_node;
}

static bool generate_var_push(ast_node_t current_node) {
	// TODO
	(void)current_node;
	return false;
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
