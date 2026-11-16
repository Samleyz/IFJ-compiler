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

static const operand_t int_1_operand = {
	.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 1}};
static const operand_t int_0_operand = {
	.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = 0}};
static const operand_t bool_true_operand = {
	.operand_type = OP_TYPE_BOOL, .frame = FT_NO_FRAME, .value = {.bool_val = true}};
static const operand_t err_40_operand = {
	.operand_type = OP_TYPE_INT, .frame = FT_NO_FRAME, .value = {.int_val = ERR_RUNTIME_SEMANTICS}};

typedef struct for_ctx {
	// name strings
	char *to_name;
	char *step_name;
	char *top_label;
	char *step_ok_label;
	char *negative_step_label;
	char *cond_end_label;
	char *end_label;
	char *next_label;

	// label operands
	operand_t operand_top_label;
	operand_t operand_step_ok_label;
	operand_t operand_negative_step_label;
	operand_t operand_cond_end_label;
	operand_t operand_end_label;
	operand_t operand_next_label;

	// variables
	operand_t operand_inter_var;
	operand_t operand_to;
	operand_t operand_step;

} for_ctx_t;

static void for_generate_epilogue(const for_ctx_t *for_info);

static bool for_generate_guards(const for_ctx_t *for_info, ast_node_t current_node);

static void for_generate_condition(const for_ctx_t *for_info, ast_node_t current_node);

static bool for_generate_init(const for_ctx_t *for_info, ast_node_t current_node);

static bool for_fill_ctx(for_ctx_t *for_info, ast_node_t current_node);
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

/**
 * @brief Emits DEFVAR for a local variable
 *
 * @param[in] var_name Name of the variable
 */
static void generate_local_defvar(const char *var_name);

/**
 * @brief Emits DEFVAR for all local variables in a list of nodes
 *
 * @param[in] head First node of the list, may be nullptr
 * @return true on success, false otherwise
 */
static bool generate_locals_decl_list(ast_node_t head);

/**
 * @brief Emits DEFVAR for the loop variable and the helper variables of a for loop, and for all
 * local variables in its subtree
 *
 * @param[in] for_node NODE_FOR node
 * @return true on success, false otherwise
 */
static bool generate_for_locals(ast_node_t for_node);

bool generate_function_body(ast_node_t current_node) { return generate_statement(current_node); }

// Tool: Claude Code (Claude Sonnet 5.5)
// Cases NODE_FUNC_CALL to NODE_TYPE_CAST and the split of generate_locals_decl into smaller
// functions implemented with AI assistance.
// Prompt: Complete the remaining cases of the switch in the same style as the existing ones.
// Prompt: Refactor the switch by splitting it into smaller functions.
bool generate_locals_decl(ast_node_t current_node) {
	// Missing optional node (else branch, step)
	if (!current_node) {
		return true;
	}

	switch (current_node->type) {
	case NODE_BLOCK:
		return generate_locals_decl_list(current_node->as.block.statements.head);
	case NODE_VAR_DEF:
		// Only declaration, the value is assigned in generate_var_def
		generate_local_defvar(current_node->as.var_def.unique_name);
		return generate_locals_decl(current_node->as.var_def.init_expr);
	case NODE_ASSIGN:
		return generate_locals_decl(current_node->as.assign.expr);
	case NODE_IF:
		return (bool)(generate_locals_decl(current_node->as.if_stmt.condition) &&
					  generate_locals_decl(current_node->as.if_stmt.then_branch) &&
					  generate_locals_decl(current_node->as.if_stmt.else_branch));
	case NODE_WHILE:
		return (bool)(generate_locals_decl(current_node->as.while_stmt.condition) &&
					  generate_locals_decl(current_node->as.while_stmt.body));
	case NODE_FOR:
		return generate_for_locals(current_node);
	case NODE_RETURN:
		return generate_locals_decl(current_node->as.return_stmt.expr);
	case NODE_EXPR_STMT:
		return generate_locals_decl(current_node->as.expr_stmt.expr);
	case NODE_BINARY_OP:
		return (bool)(generate_locals_decl(current_node->as.binary_op.left) &&
					  generate_locals_decl(current_node->as.binary_op.right));
	case NODE_UNARY_OP:
		return generate_locals_decl(current_node->as.unary_op.operand);
	case NODE_FUNC_CALL:
		// arg_lists is a list of NODE_ARG_LIST
		return (bool)(generate_locals_decl(current_node->as.func_call.callee) &&
					  generate_locals_decl_list(current_node->as.func_call.arg_lists.head));
	case NODE_ARG_LIST:
		return generate_locals_decl_list(current_node->as.arg_list.args.head);
	case NODE_MEMBER_ACCESS:
		return generate_locals_decl(current_node->as.member_access.target);
	case NODE_STRING_INTERP:
		return generate_locals_decl_list(current_node->as.string_interp.parts.head);
	case NODE_TYPE_CAST:
		return generate_locals_decl(current_node->as.type_cast.operand);
	default:
		// Leaf nodes
		return true;
	}
}

static void generate_local_defvar(const char *var_name) {
	operand_t operand = {
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = var_name}};
	print_inst_1(INST_DEFVAR, operand);
}

static bool generate_locals_decl_list(ast_node_t head) {
	ast_node_t curr_node = head;
	while (curr_node) {
		if (!generate_locals_decl(curr_node)) {
			return false;
		}
		curr_node = curr_node->next;
	}

	return true;
}

static bool generate_for_locals(ast_node_t for_node) {
	// Loop from variable
	generate_local_defvar(for_node->as.for_stmt.unique_var_name);

	// Loop step variable
	char *step_var_name = get_modified_name("%", for_node->as.for_stmt.unique_var_name, "$step");
	if (!step_var_name) {
		return false;
	}
	generate_local_defvar(step_var_name);
	free(step_var_name);

	// Loop to variable
	char *to_var_name = get_modified_name("%", for_node->as.for_stmt.unique_var_name, "$to");
	if (!to_var_name) {
		return false;
	}
	generate_local_defvar(to_var_name);
	free(to_var_name);

	return (bool)(generate_locals_decl(for_node->as.for_stmt.from_expr) &&
				  generate_locals_decl(for_node->as.for_stmt.to_expr) &&
				  generate_locals_decl(for_node->as.for_stmt.step_expr) &&
				  generate_locals_decl_list(for_node->as.for_stmt.guards.head) &&
				  generate_locals_decl(for_node->as.for_stmt.body));
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
	print_comment("return");

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
	char *top_label = get_unique_label("$", "whilestmt", "$top");
	operand_t operand_top_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = top_label}};

	if (!top_label) {
		return false;
	}

	char *end_label = get_unique_label("$", "whilestmt", "$end");
	operand_t operand_end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = end_label}};

	if (!end_label) {
		free(top_label);
		return false;
	}

	// top label
	print_comment("while");
	print_inst_1(INST_LABEL, operand_top_label);

	if (!generate_expression(current_node->as.while_stmt.condition)) {
		free(top_label);
		free(end_label);
		return false;
	}

	// condition
	print_inst_1(INST_PUSHS, bool_true_operand);
	print_inst_1(INST_JUMPIFNEQS, operand_end_label);

	fputc('\n', gen_get_output_file());

	// body
	print_comment("while body");
	if (!generate_statement(current_node->as.while_stmt.body)) {
		free(top_label);
		free(end_label);
		return false;
	}

	fputc('\n', gen_get_output_file());

	// jump to the top
	print_inst_1(INST_JUMP, operand_top_label);

	print_comment("end while");
	print_inst_1(INST_LABEL, operand_end_label);

	fputc('\n', gen_get_output_file());

	free(top_label);
	free(end_label);
	return true;
}

static bool for_fill_ctx(for_ctx_t *for_info, ast_node_t current_node) {
	// name strings
	for_info->to_name = get_modified_name("%", current_node->as.for_stmt.unique_var_name, "$to");
	for_info->step_name =
		get_modified_name("%", current_node->as.for_stmt.unique_var_name, "$step");
	for_info->top_label = get_unique_label("$", "forstmt", "$top");
	for_info->step_ok_label = get_unique_label("$", "forstmt", "$step_ok");
	for_info->negative_step_label = get_unique_label("$", "forstmt", "$negative_step");
	for_info->cond_end_label = get_unique_label("$", "forstmt", "$cond_end");
	for_info->end_label = get_unique_label("$", "forstmt", "$end");
	for_info->next_label = get_unique_label("$", "forstmt", "$next");

	// label operands
	for_info->operand_top_label = (operand_t){.operand_type = OP_TYPE_LABEL,
											  .frame = FT_NO_FRAME,
											  .value = {.label_name = for_info->top_label}};
	for_info->operand_step_ok_label = (operand_t){.operand_type = OP_TYPE_LABEL,
												  .frame = FT_NO_FRAME,
												  .value = {.label_name = for_info->step_ok_label}};

	for_info->operand_negative_step_label =
		(operand_t){.operand_type = OP_TYPE_LABEL,
					.frame = FT_NO_FRAME,
					.value = {.label_name = for_info->negative_step_label}};
	for_info->operand_cond_end_label =
		(operand_t){.operand_type = OP_TYPE_LABEL,
					.frame = FT_NO_FRAME,
					.value = {.label_name = for_info->cond_end_label}};
	for_info->operand_end_label = (operand_t){.operand_type = OP_TYPE_LABEL,
											  .frame = FT_NO_FRAME,
											  .value = {.label_name = for_info->end_label}};

	for_info->operand_next_label = (operand_t){.operand_type = OP_TYPE_LABEL,
											   .frame = FT_NO_FRAME,
											   .value = {.label_name = for_info->next_label}};
	// local variables operands
	for_info->operand_inter_var =
		(operand_t){.operand_type = OP_TYPE_VAR,
					.frame = FT_LOCAL,
					.value = {.var_name = current_node->as.for_stmt.unique_var_name}};
	for_info->operand_to = (operand_t){
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = for_info->to_name}};
	for_info->operand_step = (operand_t){
		.operand_type = OP_TYPE_VAR, .frame = FT_LOCAL, .value = {.var_name = for_info->step_name}};

	return (bool)(for_info->to_name && for_info->step_name && for_info->top_label &&
				  for_info->step_ok_label && for_info->negative_step_label &&
				  for_info->cond_end_label && for_info->end_label && for_info->next_label);
}

static bool for_generate_init(const for_ctx_t *for_info, ast_node_t current_node) {
	// from, by, to statements evaluation and assignment
	if (!generate_expression(current_node->as.for_stmt.from_expr)) {
		return false;
	}

	print_inst_1(INST_POPS, for_info->operand_inter_var);

	if (!generate_expression(current_node->as.for_stmt.to_expr)) {
		return false;
	}

	print_inst_1(INST_POPS, for_info->operand_to);

	if (current_node->as.for_stmt.step_expr) {
		if (!generate_expression(current_node->as.for_stmt.step_expr)) {
			return false;
		}
	} else {
		print_inst_1(INST_PUSHS, int_1_operand);
	}

	print_inst_1(INST_POPS, for_info->operand_step);

	if (current_node->as.for_stmt.step_expr) {
		// if step == 0 exit 40
		print_inst_1(INST_PUSHS, int_0_operand);

		print_inst_1(INST_PUSHS, for_info->operand_step);

		print_inst_0(INST_EQS);

		print_inst_1(INST_PUSHS, bool_true_operand);

		print_inst_1(INST_JUMPIFNEQS, for_info->operand_step_ok_label);

		print_inst_1(INST_EXIT, err_40_operand);

		print_inst_1(INST_LABEL, for_info->operand_step_ok_label);
	}

	return true;
}

static void for_generate_condition(const for_ctx_t *for_info, ast_node_t current_node) {
	if (current_node->as.for_stmt.is_inclusive) {
		// step > 0
		// false jmp label_negative
		print_inst_1(INST_PUSHS, for_info->operand_step);
		print_inst_1(INST_PUSHS, int_0_operand);

		print_inst_0(INST_GTS);

		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFNEQS, for_info->operand_negative_step_label);
		// iter <= to
		// if false jmp label_end
		print_inst_1(INST_PUSHS, for_info->operand_inter_var);
		print_inst_1(INST_PUSHS, for_info->operand_to);

		print_inst_0(INST_GTS);
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFEQS, for_info->operand_end_label);
		// jmp label_cond_end
		print_inst_1(INST_JUMP, for_info->operand_cond_end_label);
		// label_negative
		print_inst_1(INST_LABEL, for_info->operand_negative_step_label);
		// iter >= to
		// if false jmp label_end
		print_inst_1(INST_PUSHS, for_info->operand_inter_var);
		print_inst_1(INST_PUSHS, for_info->operand_to);

		print_inst_0(INST_LTS);
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFEQS, for_info->operand_end_label);

		// label_cond_end
		print_inst_1(INST_LABEL, for_info->operand_cond_end_label);
	} else {
		// iter < to
		// if false jmp label_end
		print_inst_1(INST_PUSHS, for_info->operand_inter_var);
		print_inst_1(INST_PUSHS, for_info->operand_to);

		print_inst_0(INST_LTS);
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFNEQS, for_info->operand_end_label);
	}
}

static bool for_generate_guards(const for_ctx_t *for_info, ast_node_t current_node) {
	ast_node_t curr_guard = current_node->as.for_stmt.guards.head;
	while (curr_guard) {
		if (!generate_expression(curr_guard)) {
			return false;
		}
		print_inst_1(INST_PUSHS, bool_true_operand);
		print_inst_1(INST_JUMPIFNEQS, for_info->operand_next_label);

		curr_guard = curr_guard->next;
	}

	return true;
}

static void for_generate_epilogue(const for_ctx_t *for_info) {
	// label next
	print_inst_1(INST_LABEL, for_info->operand_next_label);

	// next incrementation/decrementation

	print_inst_3(INST_ADD, for_info->operand_inter_var, for_info->operand_inter_var,
				 for_info->operand_step);

	print_inst_1(INST_JUMP, for_info->operand_top_label);

	print_inst_1(INST_LABEL, for_info->operand_end_label);
}

static bool generate_for(ast_node_t current_node) {
	bool success = true;

	for_ctx_t for_info = {0};

	if (!for_fill_ctx(&for_info, current_node)) {
		success = false;
		goto cleanup;
	}

	// generate for init
	if (!for_generate_init(&for_info, current_node)) {
		success = false;
		goto cleanup;
	}

	print_inst_1(INST_LABEL, for_info.operand_top_label);

	// cycle
	// condition
	for_generate_condition(&for_info, current_node);

	// generate guards
	// if false jmp next
	if (!for_generate_guards(&for_info, current_node)) {
		success = false;
		goto cleanup;
	}

	// generate for body
	if (!generate_statement(current_node->as.for_stmt.body)) {
		success = false;
		goto cleanup;
	}

	for_generate_epilogue(&for_info);

cleanup:
	free(for_info.top_label);
	free(for_info.to_name);
	free(for_info.step_name);
	free(for_info.step_ok_label);
	free(for_info.negative_step_label);
	free(for_info.cond_end_label);
	free(for_info.end_label);
	free(for_info.next_label);

	return success;
}

static bool generate_if(ast_node_t current_node) {
	print_comment("if");

	if (!generate_expression(current_node->as.if_stmt.condition)) {
		return false;
	}
	char *then_label = get_unique_label("$", "ifstmt", "$then");
	operand_t operand_then_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = then_label}};

	if (!then_label) {
		return false;
	}

	char *else_label = get_unique_label("$", "ifstmt", "$else");
	operand_t operand_else_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = else_label}};

	if (!else_label) {
		free(then_label);
		return false;
	}
	char *end_label = get_unique_label("$", "ifstmt", "$end");

	if (!end_label) {
		free(then_label);
		free(else_label);
		return false;
	}
	operand_t operand_end_label = {
		.operand_type = OP_TYPE_LABEL, .frame = FT_NO_FRAME, .value = {.label_name = end_label}};

	// condition
	print_inst_1(INST_PUSHS, bool_true_operand);
	print_inst_1(INST_JUMPIFEQS, operand_then_label);

	fputc('\n', gen_get_output_file());

	// else branch
	print_comment("else");
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
	print_comment("then");
	print_inst_1(INST_LABEL, operand_then_label);

	if (!generate_statement(current_node->as.if_stmt.then_branch)) {
		free(then_label);
		free(else_label);
		free(end_label);
		return false;
	}

	fputc('\n', gen_get_output_file());

	print_comment("end if");
	print_inst_1(INST_LABEL, operand_end_label);

	fputc('\n', gen_get_output_file());

	free(then_label);
	free(else_label);
	free(end_label);

	return true;
}

static bool generate_assign(ast_node_t current_node) {
	print_comment("assign");

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
	print_comment("val/var definition");

	if (!generate_expression(current_node->as.var_def.init_expr)) {
		return false;
	}
	operand_t operand = {.operand_type = OP_TYPE_VAR,
						 .frame = FT_LOCAL,
						 .value = {.var_name = current_node->as.var_def.unique_name}};
	print_inst_1(INST_POPS, operand);
	return true;
}
