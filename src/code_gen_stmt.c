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
#include <stdlib.h>

/**
 * @brief Generates code for an expression used as a statement, keep the expression value on the
 * stack
 *
 * @param[in] current_node NODE_EXPR_STMT node
 * @return true on success, false otherwise
 */
static bool generate_expr_stmt_keep(ast_node_t current_node);

/**
 * @brief Generates code for an expression used as a statement, discards the expression value on the
 * stack
 *
 * @param[in] current_node NODE_EXPR_STMT node
 * @return true on success, false otherwise
 */
static bool generate_expr_stmt_discard(ast_node_t current_node);

static bool generate_while(ast_node_t current_node);

static bool generate_for(ast_node_t current_node);

/**
 * @brief Generates code for an if statement
 *
 * @param[in] current_node NODE_IF node
 * @return true on success, false otherwise
 */
static bool generate_if(ast_node_t current_node);

/**
 * @brief Generates code for an assignment to an already declared variable
 *
 * @param[in] current_node NODE_ASSIGN node
 * @return true on success, false otherwise
 */
static bool generate_assign(ast_node_t current_node);

/**
 * @brief Generates code for a variable definition, the variable is declared by generate_locals_decl
 *
 * @param[in] current_node NODE_VAR_DEF node
 * @return true on success, false otherwise
 */
static bool generate_var_def(ast_node_t current_node);

/**
 * @brief Generates code for all statements of a block
 *
 * @param[in] current_node NODE_BLOCK node
 * @return true on success, false otherwise
 */
static bool generate_block(ast_node_t current_node);

/**
 * @brief Generates code for a return statement
 *
 * @param[in] current_node NODE_RETURN node
 * @return true on success, false otherwise
 */
static bool generate_return(ast_node_t current_node);

/**
 * @brief Generates code for a statement or a block according to its node type
 *
 * @param[in] current_node Statement or block node
 * @return true on success, false otherwise
 */
static bool generate_statement(ast_node_t current_node);

bool generate_function_body(ast_node_t current_node) { return generate_statement(current_node); }

bool generate_locals_decl(ast_node_t current_node) {
	// Missing optional node (else branch, step)
	if (!current_node) {
		return true;
	}

	switch (current_node->type) {
	case NODE_BLOCK:
		ast_node_t curr_node_while = current_node->as.block.statements.head;
		while (curr_node_while) {
			if (!generate_locals_decl(curr_node_while)) {
				return false;
			}
			curr_node_while = curr_node_while->next;
		}
		break;
	case NODE_VAR_DEF:
		// Only declaration, the value is assigned in generate_var_def
		operand_t operand_var_def = {.operand_type = OP_TYPE_VAR,
									 .frame = FT_LOCAL,
									 .value = {.var_name = current_node->as.var_def.unique_name}};
		print_inst_1(INST_DEFVAR, operand_var_def);

		if (!generate_locals_decl(current_node->as.var_def.init_expr)) {
			return false;
		}
		break;
	case NODE_ASSIGN:
		if (!generate_locals_decl(current_node->as.assign.expr)) {
			return false;
		}
		break;
	case NODE_IF:
		if (!generate_locals_decl(current_node->as.if_stmt.condition)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.if_stmt.then_branch)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.if_stmt.else_branch)) {
			return false;
		}
		break;
	case NODE_WHILE:
		if (!generate_locals_decl(current_node->as.while_stmt.condition)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.while_stmt.body)) {
			return false;
		}
		break;
	case NODE_FOR:
		// Loop variable
		operand_t operand_for_def = {
			.operand_type = OP_TYPE_VAR,
			.frame = FT_LOCAL,
			.value = {.var_name = current_node->as.for_stmt.unique_var_name}};
		print_inst_1(INST_DEFVAR, operand_for_def);

		if (!generate_locals_decl(current_node->as.for_stmt.from_expr)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.for_stmt.to_expr)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.for_stmt.step_expr)) {
			return false;
		}
		ast_node_t curr_node_for = current_node->as.for_stmt.guards.head;
		while (curr_node_for) {
			if (!generate_locals_decl(curr_node_for)) {
				return false;
			}
			curr_node_for = curr_node_for->next;
		}
		if (!generate_locals_decl(current_node->as.for_stmt.body)) {
			return false;
		}
		break;
	case NODE_RETURN:
		if (!generate_locals_decl(current_node->as.return_stmt.expr)) {
			return false;
		}
		break;
	case NODE_EXPR_STMT:
		if (!generate_locals_decl(current_node->as.expr_stmt.expr)) {
			return false;
		}
		break;
	case NODE_BINARY_OP:
		if (!generate_locals_decl(current_node->as.binary_op.left)) {
			return false;
		}
		if (!generate_locals_decl(current_node->as.binary_op.right)) {
			return false;
		}
		break;
	case NODE_UNARY_OP:
		if (!generate_locals_decl(current_node->as.unary_op.operand)) {
			return false;
		}
		break;
	// Tool: Claude Code (Claude Sonnet 5.5)
	// Cases NODE_FUNC_CALL to NODE_TYPE_CAST implemented with AI assistance.
	// Prompt: Complete the remaining cases of the switch in the same style as the existing ones.
	case NODE_FUNC_CALL:
		if (!generate_locals_decl(current_node->as.func_call.callee)) {
			return false;
		}
		// List of argument lists
		ast_node_t curr_arg_list = current_node->as.func_call.arg_lists.head;
		while (curr_arg_list) {
			ast_node_t curr_arg = curr_arg_list->as.arg_list.args.head;
			while (curr_arg) {
				if (!generate_locals_decl(curr_arg)) {
					return false;
				}
				curr_arg = curr_arg->next;
			}
			curr_arg_list = curr_arg_list->next;
		}
		break;
	case NODE_MEMBER_ACCESS:
		if (!generate_locals_decl(current_node->as.member_access.target)) {
			return false;
		}
		break;
	case NODE_STRING_INTERP:
		ast_node_t curr_part = current_node->as.string_interp.parts.head;
		while (curr_part) {
			if (!generate_locals_decl(curr_part)) {
				return false;
			}
			curr_part = curr_part->next;
		}
		break;
	case NODE_TYPE_CAST:
		if (!generate_locals_decl(current_node->as.type_cast.operand)) {
			return false;
		}
		break;
	default:
		// Leaf nodes
		break;
	}

	return true;
}

static bool generate_block(ast_node_t current_node) {
	ast_node_t curr_node = current_node->as.block.statements.head;
	while (curr_node) {
		if (!generate_statement(curr_node)) {
			return false;
		}
		curr_node = curr_node->next;
	}

	return true;
}

static bool generate_statement(ast_node_t current_node) {
	switch (current_node->type) {
	case NODE_VAR_DEF:
		return generate_var_def(current_node);
	case NODE_ASSIGN:
		return generate_assign(current_node);
	case NODE_IF:
		return generate_if(current_node);
	case NODE_WHILE:
		return generate_while(current_node);
	case NODE_FOR:
		return generate_for(current_node);
	case NODE_RETURN:
		return generate_return(current_node);
	case NODE_EXPR_STMT:
		return generate_expr_stmt_discard(current_node);
	case NODE_BLOCK:
		return generate_block(current_node);
	default:
		return false;
	}
}

static bool generate_return(ast_node_t current_node) {

	// Return without an expression returns Unit (nil)
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

static bool generate_expr_stmt_discard(ast_node_t current_node) {
	if (!generate_expression(current_node->as.expr_stmt.expr)) {
		return false;
	}
	operand_t operand_dis = {
		.operand_type = OP_TYPE_VAR, .frame = FT_GLOBAL, .value = {.var_name = "%discard"}};

	print_inst_1(INST_POPS, operand_dis);

	return true;
}

static bool generate_expr_stmt_keep(ast_node_t current_node) {
	return generate_expression(current_node->as.expr_stmt.expr);
}

static bool generate_while(ast_node_t current_node) {
	char *top_label = get_unique_label("$", "whilestmt", "top");
	operand_t operand_top_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = top_label}};

	if (!top_label) {
		return false;
	}

	char *end_label = get_unique_label("$", "whilestmt", "end");
	operand_t operand_end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = end_label}};

	if (!end_label) {
		free(top_label);
		return false;
	}

	operand_t operand_eq = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};

	// top label
	print_inst_1(INST_LABEL, operand_top_label);

	if (!generate_expression(current_node->as.while_stmt.condition)) {
		free(top_label);
		free(end_label);
		return false;
	}

	fputc('\n', gen_get_output_file());

	// condition
	print_inst_1(INST_PUSHS, operand_eq);
	print_inst_1(INST_JUMPIFNEQS, operand_end_label);

	fputc('\n', gen_get_output_file());

	// body
	if (!generate_statement(current_node->as.while_stmt.body)) {
		free(top_label);
		free(end_label);
		return false;
	}

	fputc('\n', gen_get_output_file());

	// jump to the top
	print_inst_1(INST_JUMP, operand_top_label);

	print_inst_1(INST_LABEL, operand_end_label);

	fputc('\n', gen_get_output_file());

	free(top_label);
	free(end_label);
	return true;
}

static bool generate_for(ast_node_t current_node) {
	// TODO
	return false;
}

static bool generate_if(ast_node_t current_node) {
	if (!generate_expression(current_node->as.if_stmt.condition)) {
		return false;
	}
	char *then_label = get_unique_label("$", "ifstmt", "then");
	operand_t operand_then_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = then_label}};

	if (!then_label) {
		return false;
	}

	char *else_label = get_unique_label("$", "ifstmt", "else");
	operand_t operand_else_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = else_label}};

	if (!else_label) {
		free(then_label);
		return false;
	}
	char *end_label = get_unique_label("$", "ifstmt", "end");

	if (!end_label) {
		free(then_label);
		free(else_label);
		return false;
	}
	operand_t operand_end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = end_label}};

	operand_t operand_eq = {
		.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};

	// condition
	print_inst_1(INST_PUSHS, operand_eq);
	print_inst_1(INST_JUMPIFEQS, operand_then_label);

	fputc('\n', gen_get_output_file());

	// else branch
	print_inst_1(INST_LABEL, operand_else_label);

	if (current_node->as.if_stmt.else_branch) {
		if (!generate_statement(current_node->as.if_stmt.else_branch)) {
			free(then_label);
			free(else_label);
			free(end_label);
			return false;
		}
	}
	print_inst_1(INST_JUMP, operand_end_label);

	fputc('\n', gen_get_output_file());

	// then branch
	print_inst_1(INST_LABEL, operand_then_label);

	if (!generate_statement(current_node->as.if_stmt.then_branch)) {
		free(then_label);
		free(else_label);
		free(end_label);
		return false;
	}

	fputc('\n', gen_get_output_file());

	print_inst_1(INST_LABEL, operand_end_label);

	fputc('\n', gen_get_output_file());

	free(then_label);
	free(else_label);
	free(end_label);

	return true;
}

static bool generate_assign(ast_node_t current_node) {
	if (!generate_expression(current_node->as.assign.expr)) {
		return false;
	}
	operand_t operand = {.operand_type = OP_TYPE_VAR,
						 .frame = FT_LOCAL,
						 .value = {.var_name = current_node->as.assign.unique_target_id}};
	print_inst_1(INST_POPS, operand);
	return true;
}

static bool generate_var_def(ast_node_t current_node) {
	if (!generate_expression(current_node->as.var_def.init_expr)) {
		return false;
	}
	operand_t operand = {.operand_type = OP_TYPE_VAR,
						 .frame = FT_LOCAL,
						 .value = {.var_name = current_node->as.var_def.unique_name}};
	print_inst_1(INST_POPS, operand);
	return true;
}
