/**
 * @file ast.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Implementation of abstract syntax tree operations
 * @date 2026-10-20
 */

#include "ast.h"
#include <stdlib.h>

ast_node_t ast_create_node(node_type_t type, unsigned line_num) {
	ast_node_t node = calloc(1, sizeof(struct ast_node));
	if (!node) {
		return NULL;
	}

	node->type = type;
	node->line_num = line_num;
	node->eval_type = TYPE_UNKNOWN;

	return node;
}

// Tool: Gemini
// AST operations implementation (list appending, and recursive tree destruction).
// Prompt: Implement AST operations for list appending, and recursive destruction
// for all node types.
// Note: Memory cleanup and recursive traversal verified according to project requirements.

ast_node_t ast_append(ast_node_t list, ast_node_t item) {
	if (!list) {
		return item;
	}

	ast_node_t current = list;
	while (current->next) {
		current = current->next;
	}
	current->next = item;

	return list;
}

ast_node_t ast_dtor(ast_node_t root) {
	while (root) {
		switch (root->type) {
		case NODE_PROGRAM:
			ast_dtor(root->as.program.first_item);
			break;

		case NODE_BLOCK:
			ast_dtor(root->as.block.first_stmt);
			break;

		case NODE_EXTENSION:
			free(root->as.extension.receiver_name);
			free(root->as.extension.receiver_type);
			ast_dtor(root->as.extension.methods);
			break;

		case NODE_FUNC_DEF:
			free(root->as.func_def.name);
			free(root->as.func_def.return_type);
			ast_dtor(root->as.func_def.param_lists);
			ast_dtor(root->as.func_def.body);
			break;

		case NODE_PARAM_LIST:
			ast_dtor(root->as.param_list.first_param);
			break;

		case NODE_PARAM:
			free(root->as.param.name);
			free(root->as.param.type_name);
			break;

		case NODE_VAR_DEF:
			free(root->as.var_def.name);
			free(root->as.var_def.type_name);
			ast_dtor(root->as.var_def.init_expr);
			break;

		case NODE_ASSIGN:
			free(root->as.assign.target_id);
			ast_dtor(root->as.assign.expr);
			break;

		case NODE_IF:
			ast_dtor(root->as.if_stmt.condition);
			ast_dtor(root->as.if_stmt.then_branch);
			ast_dtor(root->as.if_stmt.else_branch);
			break;

		case NODE_WHILE:
			ast_dtor(root->as.while_stmt.condition);
			ast_dtor(root->as.while_stmt.body);
			break;

		case NODE_FOR:
			free(root->as.for_stmt.var_name);
			ast_dtor(root->as.for_stmt.from_expr);
			ast_dtor(root->as.for_stmt.to_expr);
			ast_dtor(root->as.for_stmt.step_expr);
			ast_dtor(root->as.for_stmt.guards);
			ast_dtor(root->as.for_stmt.body);
			break;

		case NODE_RETURN:
			ast_dtor(root->as.return_stmt.expr);
			break;

		case NODE_EXPR_STMT:
			ast_dtor(root->as.expr_stmt.expr);
			break;

		case NODE_FUNC_CALL:
			free(root->as.func_call.name);
			ast_dtor(root->as.func_call.args);
			break;

		case NODE_MEMBER_ACCESS:
			ast_dtor(root->as.member_access.target);
			free(root->as.member_access.member_name);
			ast_dtor(root->as.member_access.args);
			break;

		case NODE_STRING_INTERP:
			ast_dtor(root->as.string_interp.parts);
			break;

		case NODE_TYPE_CAST:
			ast_dtor(root->as.type_cast.operand);
			break;

		case NODE_BINARY_OP:
			ast_dtor(root->as.binary_op.left);
			ast_dtor(root->as.binary_op.right);
			break;

		case NODE_UNARY_OP:
			ast_dtor(root->as.unary_op.operand);
			break;

		case NODE_IDENTIFIER:
			free(root->as.id.name);
			break;

		case NODE_LIT_STRING:
			free(root->as.string_val);
			break;

		case NODE_LIT_INT:
		case NODE_LIT_DOUBLE:
		case NODE_LIT_BOOL:
			break;
		}

		ast_node_t next = root->next;
		free(root);
		root = next;
	}

	return NULL;
}
