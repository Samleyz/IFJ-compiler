/**
 * @file semantic_analysis.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of semantic analysis functions
 */

#include "ast.h"
#include "error.h"
#include "semantic_analysis_private.h"
#include "stdio.h"
#include "stdlib.h"
#include "symtable.h"
#include <assert.h>
#include <string.h>

bool perform_semantic_analysis(ast_node_t root) {
	assert(root && "Called perform_semantic_analysis on nullptr.");

	// Initialize semantic context
	semantic_context_t context;
	if (!semantic_context_init(&context)) {
		print_error(ERR_INTERNAL, 0, "Failed to initialize semantic context.");
		return false;
	}

	// TODO: Passes

	// TODO: Checks

	// Cleanup
	semantic_context_dtor(&context);
	return true;
}

bool semantic_context_init(semantic_context_t *context) {
	assert(context && "Called semantic_context_init on nullptr.");

	// Prefill with zeros
	*context = (semantic_context_t){0};

	// Init symbol tables
	context->symtable_local = stable_init();
	context->symtable_functions = stable_init();
	context->symtable_extension_functions = stable_init();
	if (!context->symtable_local || !context->symtable_functions ||
		!context->symtable_extension_functions) {
		print_error(ERR_INTERNAL, 0, "Failed to initialize symbol tables.");
		semantic_context_dtor(context);
		return false;
	}

	// Init bultin functions
	if (!prefill_builtin_global_functions(context->symtable_functions)) {
		print_error(ERR_INTERNAL, 0, "Failed to prefill builtin global functions.");
		semantic_context_dtor(context);
		return false;
	}
	if (!prefill_builtin_extension_functions(context->symtable_extension_functions)) {
		print_error(ERR_INTERNAL, 0, "Failed to prefill builtin extension functions.");
		semantic_context_dtor(context);
		return false;
	}

	return true;
}

void semantic_context_dtor(semantic_context_t *context) {
	if (!context) {
		return;
	}

	// Destroy symbol tables
	if (context->symtable_local) {
		stable_dtor(context->symtable_local);
	}
	if (context->symtable_functions) {
		stable_dtor(context->symtable_functions);
	}
	if (context->symtable_extension_functions) {
		stable_dtor(context->symtable_extension_functions);
	}

	// Zero out the structure
	*context = (semantic_context_t){0};
}

bool prefill_builtin_global_functions(stable_t stab) {
	assert(stab && "Called prefill_builtin_global_functions on nullptr.");

	bool ok = true;
	ok &= register_builtin_function(stab, "print", TYPE_UNIT, (data_type_t[]){TYPE_ANY}, 1);
	ok &= register_builtin_function(stab, "readLine", TYPE_STRING, (data_type_t[]){}, 0);
	ok &= register_builtin_function(stab, "readInt", TYPE_INT, (data_type_t[]){}, 0);
	ok &= register_builtin_function(stab, "readDouble", TYPE_DOUBLE, (data_type_t[]){}, 0);
	ok &= register_builtin_function(stab, "readBool", TYPE_BOOL, (data_type_t[]){}, 0);
	ok &= register_builtin_function(stab, "toStr", TYPE_STRING, (data_type_t[]){TYPE_ANY}, 1);
	ok &= register_builtin_function(stab, "toInt", TYPE_INT, (data_type_t[]){TYPE_DOUBLE}, 1);
	ok &= register_builtin_function(stab, "strLength", TYPE_INT, (data_type_t[]){TYPE_STRING}, 1);
	ok &= register_builtin_function(stab, "subStr", TYPE_STRING,
									(data_type_t[]){TYPE_STRING, TYPE_INT, TYPE_INT}, 3);
	ok &=
		register_builtin_function(stab, "ord", TYPE_INT, (data_type_t[]){TYPE_STRING, TYPE_INT}, 2);
	ok &= register_builtin_function(stab, "chr", TYPE_STRING, (data_type_t[]){TYPE_INT}, 1);

	if (!ok) {
		// Print error
		print_error(ERR_INTERNAL, 0, "Failed to prefill builtin global functions.");

		// Cleanup
		if (stable_find_curlevel(stab, "print")) {
			stable_remove_symbol_curlevel(stab, "print");
		}
		if (stable_find_curlevel(stab, "readLine")) {
			stable_remove_symbol_curlevel(stab, "readLine");
		}
		if (stable_find_curlevel(stab, "readInt")) {
			stable_remove_symbol_curlevel(stab, "readInt");
		}
		if (stable_find_curlevel(stab, "readDouble")) {
			stable_remove_symbol_curlevel(stab, "readDouble");
		}
		if (stable_find_curlevel(stab, "readBool")) {
			stable_remove_symbol_curlevel(stab, "readBool");
		}
		if (stable_find_curlevel(stab, "toStr")) {
			stable_remove_symbol_curlevel(stab, "toStr");
		}
		if (stable_find_curlevel(stab, "toInt")) {
			stable_remove_symbol_curlevel(stab, "toInt");
		}
		if (stable_find_curlevel(stab, "strLength")) {
			stable_remove_symbol_curlevel(stab, "strLength");
		}
		if (stable_find_curlevel(stab, "subStr")) {
			stable_remove_symbol_curlevel(stab, "subStr");
		}
		if (stable_find_curlevel(stab, "ord")) {
			stable_remove_symbol_curlevel(stab, "ord");
		}
		if (stable_find_curlevel(stab, "chr")) {
			stable_remove_symbol_curlevel(stab, "chr");
		}

		return false;
	}

	return true;
}

bool prefill_builtin_extension_functions(stable_t stab) {
	assert(stab && "Called prefill_builtin_extension_functions on nullptr.");

	bool ok = true;
	ok &= register_builtin_extension_function(stab, "toInt$Double", TYPE_INT, nullptr, 0,
											  TYPE_DOUBLE);
	ok &= register_builtin_extension_function(stab, "floor$Double", TYPE_DOUBLE, nullptr, 0,
											  TYPE_DOUBLE);
	ok &= register_builtin_extension_function(stab, "length$String", TYPE_INT, nullptr, 0,
											  TYPE_STRING);
	ok &= register_builtin_extension_function(stab, "toString$Int", TYPE_STRING, nullptr, 0,
											  TYPE_INT);
	ok &= register_builtin_extension_function(stab, "toString$Double", TYPE_STRING, nullptr, 0,
											  TYPE_DOUBLE);
	ok &= register_builtin_extension_function(stab, "toString$String", TYPE_STRING, nullptr, 0,
											  TYPE_STRING);
	ok &= register_builtin_extension_function(stab, "toString$Bool", TYPE_STRING, nullptr, 0,
											  TYPE_BOOL);
	ok &= register_builtin_extension_function(stab, "toString$Unit", TYPE_STRING, nullptr, 0,
											  TYPE_UNIT);

	if (!ok) {
		// Print error
		print_error(ERR_INTERNAL, 0, "Failed to prefill builtin extension functions.");

		// Cleanup
		if (stable_find_curlevel(stab, "toInt$Double")) {
			stable_remove_symbol_curlevel(stab, "toInt$Double");
		}
		if (stable_find_curlevel(stab, "floor$Double")) {
			stable_remove_symbol_curlevel(stab, "floor$Double");
		}
		if (stable_find_curlevel(stab, "length$String")) {
			stable_remove_symbol_curlevel(stab, "length$String");
		}
		if (stable_find_curlevel(stab, "toString$Int")) {
			stable_remove_symbol_curlevel(stab, "toString$Int");
		}
		if (stable_find_curlevel(stab, "toString$Double")) {
			stable_remove_symbol_curlevel(stab, "toString$Double");
		}
		if (stable_find_curlevel(stab, "toString$String")) {
			stable_remove_symbol_curlevel(stab, "toString$String");
		}
		if (stable_find_curlevel(stab, "toString$Bool")) {
			stable_remove_symbol_curlevel(stab, "toString$Bool");
		}
		if (stable_find_curlevel(stab, "toString$Unit")) {
			stable_remove_symbol_curlevel(stab, "toString$Unit");
		}

		return false;
	}

	return true;
}

bool register_builtin_function(stable_t stab, const char *name, data_type_t return_type,
							   data_type_t *param_types, size_t param_count) {
	assert(stab && name && return_type && param_types &&
		   "Called register_builtin_function with invalid parameters.");

	// Create symbol for the function
	symbol_t sym = stable_init_symbol();
	if (!sym) {
		return false;
	}

	// Fill out symbol
	sym->symbol_variant = SYMBOL_FUNCTION;
	sym->function.state = FUNCTION_VISITED;
	sym->function.is_main = false;
	sym->function.is_builtin = true;
	sym->function.has_explicit_return_type = true;
	sym->function.return_type = return_type;
	ast_list_init(&(sym->function.parameter_lists)); // Will be rewritten if has parameters lists
	sym->function.extension_parameter_type = TYPE_UNKNOWN;
	sym->function.extension_parameter_name = nullptr;

	// Create parameter list for the function
	if (!create_parameter_list_for_builtin_function(&(sym->function.parameter_lists), param_types,
													param_count)) {
		print_error(ERR_INTERNAL, 0, "Failed to create parameter lists for builtin function '%s'.",
					name);
		stable_destroy_symbol(sym);
		return false;
	}

	// Write symbol to the symbol table
	if (!stable_write(stab, name, sym)) {
		print_error(ERR_INTERNAL, 0, "Failed to write builtin function '%s' to symbol table.",
					name);
		stable_destroy_symbol(sym);
		return false;
	}
	return true;
}

bool register_builtin_extension_function(stable_t stab, const char *name, data_type_t return_type,
										 data_type_t *param_types, size_t param_count,
										 data_type_t receiver_type) {
	assert(stab && name && return_type && receiver_type &&
		   "Called register_builtin_extension_function with invalid parameters.");

	// Create symbol for the function
	symbol_t sym = stable_init_symbol();
	if (!sym) {
		return false;
	}

	// Fill out symbol
	sym->symbol_variant = SYMBOL_FUNCTION;
	sym->function.state = FUNCTION_VISITED;
	sym->function.is_main = false;
	sym->function.is_builtin = true;
	sym->function.has_explicit_return_type = true;
	sym->function.return_type = return_type;
	ast_list_init(&(sym->function.parameter_lists)); // Will be rewritten if has parameters lists
	sym->function.extension_parameter_type = receiver_type;
	sym->function.extension_parameter_name = nullptr;

	// Create parameter list for the function
	if (param_types) {
		if (!create_parameter_list_for_builtin_function(&(sym->function.parameter_lists),
														param_types, param_count)) {
			print_error(ERR_INTERNAL, 0,
						"Failed to create parameter lists for builtin extension function '%s'.",
						name);
			stable_destroy_symbol(sym);
			return false;
		}
	}

	// Write symbol to the symbol table
	if (!stable_write(stab, name, sym)) {
		print_error(ERR_INTERNAL, 0,
					"Failed to write builtin extension function '%s' to symbol table.", name);
		stable_destroy_symbol(sym);
		return false;
	}
	return true;
}

bool create_parameter_list_for_builtin_function(ast_list_t *param_lists, data_type_t *param_types,
												size_t param_count) {
	assert(param_lists && (param_count == 0 || param_types) &&
		   "Called create_parameter_list_for_builtin_function with invalid parameters.");

	// Create parameter list list
	ast_list_init(&param_lists);

	if (!param_types) {
		// No parameter list to create
		return true;
	}

	// Create node
	ast_node_t param_list_node = ast_create_node(NODE_PARAM_LIST, 0);
	if (!param_list_node) {
		print_error(ERR_INTERNAL, 0, "Failed to create parameter list node for builtin function.");
		return false;
	}

	// Append the parameter list to the list of parameter lists
	if (!ast_list_append(&param_lists, param_list_node)) {
		print_error(
			ERR_INTERNAL, 0,
			"Failed to append parameter list node to parameter lists list for builtin function.");
		ast_dtor(param_list_node);
		ast_list_init(&param_lists); // Reset the list to empty
		return false;
	}

	// Create parameter list
	ast_list_init(&(param_list_node->as.param_list.params));
	for (size_t i = 0; i < param_count; i++) {
		// Create parameter node
		ast_node_t param_node = ast_create_node(NODE_PARAM, 0);
		if (!param_node) {
			print_error(ERR_INTERNAL, 0, "Failed to create parameter node for builtin function.");
			ast_dtor(param_list_node);
			ast_list_init(&param_lists); // Reset the list to empty
			return false;
		}

		// Create data type
		param_node->as.param.type_name = strdup(data_type_string_forms[param_types[i]]);
		param_node->eval_type = param_types[i];
		if (!param_node->as.param.type_name) {
			print_error(ERR_INTERNAL, 0,
						"Failed to allocate memory for parameter type name for builtin function.");
			ast_dtor(param_list_node);
			ast_dtor(param_node);
			ast_list_init(&param_lists); // Reset the list to empty
			return false;
		}

		// Append parameter node
		if (!ast_list_append(&param_list_node->as.param_list.params, param_node)) {
			print_error(ERR_INTERNAL, 0,
						"Failed to append parameter to parameter list for builtin function.");
			ast_dtor(param_list_node);
			ast_dtor(param_node);
			ast_list_init(&param_lists); // Reset the list to empty
			return false;
		}
	}

	return true;
}

data_type_t get_data_type_from_string(const char *type_str) {
	assert(type_str && "Called get_data_type_from_string on nullptr.");

	if (strcmp(type_str, "Int") == 0) {
		return TYPE_INT;
	} else if (strcmp(type_str, "Double") == 0) {
		return TYPE_DOUBLE;
	} else if (strcmp(type_str, "String") == 0) {
		return TYPE_STRING;
	} else if (strcmp(type_str, "Boolean") == 0) {
		return TYPE_BOOL;
	} else if (strcmp(type_str, "Unit") == 0) {
		return TYPE_UNIT;
	} else if (strcmp(type_str, "Any") == 0) {
		return TYPE_ANY;
	} else {
		return TYPE_UNKNOWN;
	}
}

/*
####################################################################################################
# First pass functions
####################################################################################################
*/

bool sem_first(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first on nullptr.");
	// TODO
	return true;
}

bool sem_first_sub_program(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_sub_program on nullptr.");
	// TODO
	return true;
}

bool sem_first_node_func_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_node_func_def on nullptr.");
	// TODO
	return true;
}

bool sem_first_node_extension(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_node_extension on nullptr.");
	// TODO
	return true;
}

/*
####################################################################################################
# Second pass functions
####################################################################################################
*/

bool sem_second_stmt(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_stmt on nullptr.");

	switch (node->type) {
	case NODE_VAR_DEF:
		return sem_second_node_var_def(node, context);
	case NODE_ASSIGN:
		return sem_second_node_assign(node, context);
	case NODE_IF:
		return sem_second_node_if_stmt(node, context); // if - statement
	case NODE_WHILE:
		return sem_second_node_while(node, context);
	case NODE_FOR:
		return sem_second_node_for(node, context);
	case NODE_RETURN:
		return sem_second_node_return(node, context);
	case NODE_EXPR_STMT:
		return sem_second_node_expr_stmt(node, context);
	default:
		print_error(ERR_INTERNAL, node->line_num, "Called sem_second_stmt on non statement node.");
		return false;
	}
}

bool sem_second_expr(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_expr on nullptr.");

	switch (node->type) {
	case NODE_BINARY_OP:
		return sem_second_node_binary_op(node, context);
	case NODE_UNARY_OP:
		return sem_second_node_unary_op(node, context);
	case NODE_FUNC_CALL:
		return sem_second_node_func_call(node, context);
	case NODE_MEMBER_ACCESS:
		return sem_second_node_member_access(node, context);
	case NODE_STRING_INTERP:
		return sem_second_node_string_interp(node, context);
	case NODE_IF:
		// Need value from if
		bool storage1 = context->is_in_expression_if;
		context->is_in_expression_if = true;

		bool status = sem_second_node_if_expr(node, context); // if - expression

		// Restore
		context->is_in_expression_if = storage1;

		return status;
	case NODE_IDENTIFIER:
		return sem_second_node_identifier(node, context);
	case NODE_LIT_INT:
		return sem_second_node_lit_int(node, context);
	case NODE_LIT_DOUBLE:
		return sem_second_node_lit_double(node, context);
	case NODE_LIT_STRING:
		return sem_second_node_lit_string(node, context);
	case NODE_LIT_BOOL:
		return sem_second_node_lit_bool(node, context);
	case NODE_LIT_UNIT:
		return sem_second_node_lit_unit(node, context);
	default:
		print_error(ERR_INTERNAL, node->line_num, "Called sem_second_expr on non expression node.");
		return false;
	}
}

bool sem_second_node_program(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_block(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_extension(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_var_def(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_assign(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_if_expr(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_if_stmt(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_while(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_for(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_return(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_expr_stmt(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_binary_op(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_unary_op(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_func_call(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_member_access(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_string_interp(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_type_cast(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_param_list(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_param(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_identifier(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_lit_int(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_lit_double(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_lit_string(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_lit_bool(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_node_lit_unit(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}
