/**
 * @file code_gen_stmt.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of the functions responsible for statement and program flow control
 * generation
 * @date 2026-11-08
 */

#include "ast.h"
#include "code_gen_private.h"
#include "instruction_set.h"

static bool generate_block(ast_node_t current_node);

static bool generate_return(ast_node_t current_node);

static bool generate_statement(ast_node_t current_node);

bool generate_function_body(ast_node_t current_node) {
	if (current_node->type == NODE_BLOCK) {
		if (!generate_block(current_node)) {
			return false;
		}
	} else {
		if (!generate_return(current_node)) {
			return false;
		}
	}
	return true;
}

static bool generate_block(ast_node_t current_node) {
	ast_node_t curr_node = current_node->as.block.statements.head;
	while (curr_node) {
		generate_statement(curr_node);
		curr_node = curr_node->next;
	}

	return true;
}

static bool generate_statement(ast_node_t current_node) {
	switch (current_node->type) {
	case NODE_VAR_DEF:
		break;
	case NODE_ASSIGN:
		break;
	case NODE_IF:
		break;
	case NODE_WHILE:
		break;
	case NODE_FOR:
		break;
	case NODE_RETURN:
		return generate_return(current_node);
	case NODE_EXPR_STMT:
		break;
	case NODE_BLOCK:
		return generate_block(current_node);
	default:
		return false;
	}
	return false;
}

static bool generate_return(ast_node_t current_node) {

	if (current_node->as.return_stmt.expr == NULL) {
		operand_t operand = {.operand_type = OP_TYPE_NIL, .frame = FT_NO_FRAME, .value = {0}};
		print_inst_1(INST_PUSHS, operand);
	} else {
		if (!generate_expression(current_node->as.return_stmt.expr)) {
			return false;
		}
	}

	generate_function_epilogue();

	return true;
}
