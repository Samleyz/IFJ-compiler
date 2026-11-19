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

	// TODO: Pass1
	// TODO: Checks with cleanup on error

	// TODO: Pass2
	// TODO: (Checks with cleanup on error)

	// TODO: (Context checks)

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
	ok &= register_builtin_function(stab, "readBoolean", TYPE_BOOL, (data_type_t[]){}, 0);
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
		if (stable_find_curlevel(stab, "readBoolean")) {
			stable_remove_symbol_curlevel(stab, "readBoolean");
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
	ok &= register_builtin_extension_function(stab, "toString$Boolean", TYPE_STRING, nullptr, 0,
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
		if (stable_find_curlevel(stab, "toString$Boolean")) {
			stable_remove_symbol_curlevel(stab, "toString$Boolean");
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
	ast_list_init(param_lists);

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
	if (!ast_list_append(param_lists, param_list_node)) {
		print_error(
			ERR_INTERNAL, 0,
			"Failed to append parameter list node to parameter lists list for builtin function.");
		ast_dtor(param_list_node);
		ast_list_init(param_lists); // Reset the list to empty
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
			ast_list_init(param_lists); // Reset the list to empty
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
			ast_list_init(param_lists); // Reset the list to empty
			return false;
		}

		// Append parameter node
		if (!ast_list_append(&param_list_node->as.param_list.params, param_node)) {
			print_error(ERR_INTERNAL, 0,
						"Failed to append parameter to parameter list for builtin function.");
			ast_dtor(param_list_node);
			ast_dtor(param_node);
			ast_list_init(param_lists); // Reset the list to empty
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

	// Check program node
	if (node->type != NODE_PROGRAM) {
		print_error(ERR_INTERNAL, node->line_num, "First pass called on non program node.");
		return false;
	}

	// Go through the items in the program
	ast_node_t current_node = node->as.program.items.head;
	while (current_node) {
		if (!sem_first_sub_program(current_node, context)) {
			return false;
		}
		current_node = current_node->next;
	}

	// Check if main found
	if (context->main_functions_count == 0) {
		print_error(ERR_SEM_UNDEFINED, 0, "No main function found in the program.");
		return false;
	}

	return true;
}

bool sem_first_sub_program(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_sub_program on nullptr.");

	switch (node->type) {
	case NODE_FUNC_DEF:
		return sem_first_node_func_def(node, context);
	case NODE_EXTENSION:
		return sem_first_node_extension(node, context);
	default:
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_first_sub_program on non function or extension node.");
		return false;
	}
}

bool sem_first_node_func_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_node_func_def on nullptr.");

	// Node checks
	if (node->type != NODE_FUNC_DEF) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_first_node_func_def on non function definition node.");
		return false;
	}
	if (!node->as.func_def.name) {
		print_error(ERR_INTERNAL, node->line_num, "Function definition has no name.");
		return false;
	}
	if (!node->as.func_def.body) {
		print_error(ERR_INTERNAL, node->line_num, "Function definition has no body.");
		return false;
	}

	// Create symbol
	symbol_t symbol = stable_init_symbol();
	if (!symbol) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to create function symbol.");
		return false;
	}

	// Prefill symbol
	symbol->symbol_variant = SYMBOL_FUNCTION;

	// Check if not defined
	if (stable_find_curlevel(context->symtable_functions, node->as.func_def.name)) {
		print_error(ERR_SEM_REDEFINITION, node->line_num,
					"Function '%s' is already defined in the current scope.",
					node->as.func_def.name);
		stable_destroy_symbol(symbol);
		return false;
	}

	// Check data type of return type if explicit
	if (node->as.func_def.return_type) {
		symbol->function.has_explicit_return_type = true;
		symbol->function.return_type = get_data_type_from_string(node->as.func_def.return_type);
	}

	// Main check
	if (node->as.func_def.is_main) {
		symbol->function.is_main = true;
		context->main_functions_count++;

		// Max 1 main
		if (context->main_functions_count > 1) {
			print_error(ERR_SEM_REDEFINITION, node->line_num,
						"Multiple main functions defined in the program.");
			stable_destroy_symbol(symbol);
			return false;
		}

		// Main parameters
		if (node->as.func_def.param_lists.count > 1 || // More parameter list
			node->as.func_def.param_lists.head->as.param_list.params.count != 0) { // It is empty
			print_error(ERR_SEM_PARAMS, node->line_num,
						"Main function must have empty parameter list.");
			stable_destroy_symbol(symbol);
			return false;
		}

		// Main return type
		if (symbol->function.return_type != TYPE_UNIT &&
			symbol->function.has_explicit_return_type) { // If explicit return type, it must be Unit
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
						"Main function must have return type Unit.");
			stable_destroy_symbol(symbol);
			return false;
		}
	}

	// Validate parameters and uniqueness
	if (!sem_first_validate_parameter_uniqueness(&node->as.func_def.param_lists, nullptr,
												 context)) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to validate parameter uniqueness for function '%s'.",
					node->as.func_def.name);
		stable_destroy_symbol(symbol);
		return false;
	}

	// Parameter lists
	symbol->function.parameter_lists = node->as.func_def.param_lists;

	// Store symbol
	if (!stable_write(context->symtable_functions, node->as.func_def.name, symbol)) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to write function '%s' to symbol table.",
					node->as.func_def.name);
		stable_destroy_symbol(symbol);
		return false;
	}

	return true;
}

bool sem_first_node_extension(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_first_node_extension on nullptr.");

	// Node checks
	if (node->type != NODE_EXTENSION) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_first_node_extension on non-extension node.");
		return false;
	}
	if (!node->as.extension.receiver_name) {
		print_error(ERR_INTERNAL, node->line_num, "Extension has no receiver name.");
		return false;
	}
	if (!node->as.extension.receiver_type) {
		print_error(ERR_INTERNAL, node->line_num, "Extension has no receiver type.");
		return false;
	}

	// Go through extension functions
	ast_node_t current_extension_node = node->as.extension.methods.head;
	while (current_extension_node) {
		// Node check
		if (current_extension_node->type != NODE_FUNC_DEF) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Invalid node type in extension methods list.");
			return false;
		}
		if (!current_extension_node->as.func_def.name) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Extension function has no name.");
			return false;
		}

		// Make key
		char *key = malloc(strlen(current_extension_node->as.func_def.name) +
						   strlen(node->as.extension.receiver_type) + 2);
		if (!key) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(key, "%s$%s", current_extension_node->as.func_def.name,
				node->as.extension.receiver_type);

		// Check if not defined
		if (stable_find_curlevel(context->symtable_extension_functions, key)) {
			print_error(
				ERR_SEM_REDEFINITION, current_extension_node->line_num,
				"Extension function '%s' for type '%s' is already defined in the current scope.",
				current_extension_node->as.func_def.name, node->as.extension.receiver_type);
			free(key);
			return false;
		}

		// Create symbol
		symbol_t symbol = stable_init_symbol();
		if (!symbol) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Failed to create extension function symbol.");
			free(key);
			return false;
		}

		// Prefill symbol
		symbol->symbol_variant = SYMBOL_FUNCTION;

		// Return type
		if (current_extension_node->as.func_def.return_type) {
			symbol->function.has_explicit_return_type = true;
			symbol->function.return_type =
				get_data_type_from_string(current_extension_node->as.func_def.return_type);
		}

		// Extension parameters
		symbol->function.extension_parameter_type =
			get_data_type_from_string(node->as.extension.receiver_type);
		symbol->function.extension_parameter_name = node->as.extension.receiver_name;

		// Validate parameters and uniqueness
		if (!sem_first_validate_parameter_uniqueness(
				&current_extension_node->as.func_def.param_lists, node->as.extension.receiver_name,
				context)) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Failed to validate parameter uniqueness for extension function.");
			free(key);
			stable_destroy_symbol(symbol);
			return false;
		}

		// Parameter lists
		symbol->function.parameter_lists = current_extension_node->as.func_def.param_lists;

		// Add extension function to symbol table
		if (!stable_write(context->symtable_extension_functions, key, symbol)) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Failed to add extension function to symbol table.");
			free(key);
			stable_destroy_symbol(symbol);
			return false;
		}
		free(key);

		// Move to next
		current_extension_node = current_extension_node->next;
	}

	return true;
}

bool sem_first_validate_parameter_uniqueness(ast_list_t *param_lists, const char *receiver_name,
											 semantic_context_t *context) {
	assert(param_lists && context && "Called sem_first_validate_parameter_uniqueness on nullptr.");

	// Make new scope for parameters
	if (!stable_enter_scope(context->symtable_local)) {
		print_error(ERR_INTERNAL, 0, "Failed to enter new scope for parameter uniqueness check.");
		return false;
	}

	// Register receiver name if exists
	if (receiver_name) {
		symbol_t receiver_symbol = stable_init_symbol();
		if (!receiver_symbol) {
			print_error(ERR_INTERNAL, 0,
						"Failed to create receiver symbol for parameter uniqueness check.");
			stable_leave_scope(context->symtable_local);
			return false;
		}
		receiver_symbol->symbol_variant = SYMBOL_VARIABLE;
		if (!stable_write(context->symtable_local, receiver_name, receiver_symbol)) {
			print_error(ERR_INTERNAL, 0,
						"Failed to register receiver name '%s' in parameter uniqueness check.",
						receiver_name);
			stable_leave_scope(context->symtable_local);
			stable_destroy_symbol(receiver_symbol);
			return false;
		}
	}

	// Compare parameters in all parameter lists
	ast_node_t current_param_list_node = param_lists->head;
	while (current_param_list_node) {
		// Node checks
		if (current_param_list_node->type != NODE_PARAM_LIST) {
			print_error(ERR_INTERNAL, current_param_list_node->line_num,
						"Invalid node type in parameter lists.");
			stable_leave_scope(context->symtable_local);
			return false;
		}

		ast_node_t current_param_node = current_param_list_node->as.param_list.params.head;
		while (current_param_node) {
			// Node checks
			if (current_param_node->type != NODE_PARAM) {
				print_error(ERR_INTERNAL, current_param_node->line_num,
							"Invalid node type in parameter list.");
				stable_leave_scope(context->symtable_local);
				return false;
			}
			if (!current_param_node->as.param.name) {
				print_error(ERR_INTERNAL, current_param_node->line_num, "Parameter has no name.");
				stable_leave_scope(context->symtable_local);
				return false;
			}

			// Check if aleady registered
			if (stable_find_curlevel(context->symtable_local, current_param_node->as.param.name)) {
				print_error(ERR_SEM_REDEFINITION, current_param_node->line_num,
							"Parameter '%s' is already defined in the current scope.",
							current_param_node->as.param.name);
				stable_leave_scope(context->symtable_local);
				return false;
			}

			// Set evaluation type of the parameter
			current_param_node->eval_type =
				get_data_type_from_string(current_param_node->as.param.type_name);
			if (current_param_node->eval_type == TYPE_UNKNOWN) {
				print_error(ERR_SEM_UNDEFINED, current_param_node->line_num,
							"Parameter '%s' has unknown type '%s'.",
							current_param_node->as.param.name,
							current_param_node->as.param.type_name);
				stable_leave_scope(context->symtable_local);
				return false;
			}

			// Register the parameter
			symbol_t param_symbol = stable_init_symbol();
			if (!param_symbol) {
				print_error(ERR_INTERNAL, current_param_node->line_num,
							"Failed to create parameter symbol for parameter uniqueness check.");
				stable_leave_scope(context->symtable_local);
				return false;
			}
			param_symbol->symbol_variant = SYMBOL_VARIABLE;
			param_symbol->variable.type = current_param_node->eval_type;
			if (!stable_write(context->symtable_local, current_param_node->as.param.name,
							  param_symbol)) {
				print_error(ERR_INTERNAL, current_param_node->line_num,
							"Failed to register parameter '%s' in parameter uniqueness check.",
							current_param_node->as.param.name);
				stable_leave_scope(context->symtable_local);
				stable_destroy_symbol(param_symbol);
				return false;
			}

			// Move to next parameter
			current_param_node = current_param_node->next;
		}

		// Move to next parameter list
		current_param_list_node = current_param_list_node->next;
	}

	stable_leave_scope(context->symtable_local);
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
	assert(node && context && "Called sem_second_node_program on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_block(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_block on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_extension(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_extension on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_def on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_var_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_var_def on nullptr.");

	// Node checks
	if (node->type != NODE_VAR_DEF) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_var_def on non variable definition node.");
		return false;
	}
	if (!node->as.var_def.name) {
		print_error(ERR_INTERNAL, node->line_num, "Variable definition has no name.");
		return false;
	}
	if (!node->as.var_def.init_expr) {
		print_error(ERR_INTERNAL, node->line_num,
					"Variable definition has no initialization expression.");
		return false;
	}

	// Check if variable is already defined in the current scope
	if (stable_find_curlevel(context->symtable_local, node->as.var_def.name)) {
		print_error(ERR_SEM_REDEFINITION, node->line_num,
					"Variable '%s' is already defined in the current scope.",
					node->as.var_def.name);
		return false;
	}

	// Pass expression
	if (!sem_second_expr(node->as.var_def.init_expr, context)) {
		throw_warning(node->line_num,
					  "Failed to process variable definition value for variable '%s'.",
					  node->as.var_def.name);
		return false;
	}

	// Create symbol
	symbol_t sym = stable_init_symbol();
	if (!sym) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to create symbol for variable '%s'.",
					node->as.var_def.name);
		return false;
	}
	sym->symbol_variant = SYMBOL_VARIABLE;

	// Create unique name
	sym->variable.unique_name =
		malloc(strlen(node->as.var_def.name) + 20 + 1 + 1); // Enough for size_t digits + \0 + $
	if (!sym->variable.unique_name) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to allocate memory for variable unique name.");
		stable_destroy_symbol(sym);
		return false;
	}
	sprintf(sym->variable.unique_name, "%s$%zu", node->as.var_def.name, context->rename_counter++);
	node->as.var_def.unique_name = strdup(sym->variable.unique_name);
	if (!node->as.var_def.unique_name) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to allocate memory for variable unique name in AST.");
		stable_destroy_symbol(sym);
		return false;
	}

	// Fill out symbol
	sym->variable.is_mutable = !node->as.var_def.is_val;
	if (node->as.var_def.type_name) {
		// Explicit type
		sym->variable.type = get_data_type_from_string(node->as.var_def.type_name);
		if (sym->variable.type == TYPE_UNKNOWN) {
			print_error(ERR_SEM_UNDEFINED, node->line_num, "Variable '%s' has unknown type '%s'.",
						node->as.var_def.name, node->as.var_def.type_name);
			stable_destroy_symbol(sym);
			return false;
		}
		// Check implicit conversion
		if (sym->variable.type != node->as.var_def.init_expr->eval_type) {
			if (sym->variable.type == TYPE_DOUBLE &&
				node->as.var_def.init_expr->eval_type == TYPE_INT) {
				// Implicit conversion from Int to Double
				ast_node_t conversion_node = ast_create_node(NODE_TYPE_CAST, node->line_num);
				if (!conversion_node) {
					print_error(ERR_INTERNAL, node->line_num,
								"Failed to create type cast node for implicit conversion from Int "
								"to Double.");
					stable_destroy_symbol(sym);
					return false;
				}
				conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
				conversion_node->eval_type = TYPE_DOUBLE;
				conversion_node->as.type_cast.operand = node->as.var_def.init_expr;
				node->as.var_def.init_expr = conversion_node;

			} else {
				print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
							"Type mismatch in variable definition for variable '%s'.",
							node->as.var_def.name);
				stable_destroy_symbol(sym);
				return false;
			}
		}
	} else {
		if (node->as.var_def.init_expr->eval_type == TYPE_UNKNOWN) {
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num, "Variable '%s' has unknown type.",
						node->as.var_def.name);
			stable_destroy_symbol(sym);
			return false;
		}

		// Implicit type from initialization expression
		sym->variable.type = node->as.var_def.init_expr->eval_type;
	}

	// Set eval type
	node->eval_type = TYPE_UNIT;

	// Write symbol to the symbol table
	if (!stable_write(context->symtable_local, node->as.var_def.name, sym)) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to write symbol for variable '%s' to the symbol table.",
					node->as.var_def.name);
		stable_destroy_symbol(sym);
		return false;
	}
	return true;
}

bool sem_second_node_assign(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_assign on nullptr.");

	// Node checks
	if (node->type != NODE_ASSIGN) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_assign on non assignment node.");
		return false;
	}
	if (!node->as.assign.target_id) {
		print_error(ERR_INTERNAL, node->line_num, "Assignment has no target.");
		return false;
	}
	if (!node->as.assign.expr) {
		print_error(ERR_INTERNAL, node->line_num, "Assignment has no value.");
		return false;
	}

	// Check if defined
	symbol_t sym = stable_find_alllevel(context->symtable_local, node->as.assign.target_id);
	if (!sym) {
		print_error(ERR_SEM_UNDEFINED, node->line_num,
					"Variable '%s' is not defined in the current scope.",
					node->as.assign.target_id);
		return false;
	}

	// Check if mutable
	if (!sym->variable.is_mutable) {
		print_error(ERR_SEM_IMMUTABLE, node->line_num, "Cannot assign to immutable variable '%s'.",
					node->as.assign.target_id);
		return false;
	}

	// Check types
	if (!sem_second_expr(node->as.assign.expr, context)) {
		throw_warning(node->line_num, "Failed to process assignment value for variable '%s'.",
					  node->as.assign.target_id);
		return false;
	}
	if (sym->variable.type == node->as.assign.expr->eval_type) {
		// Types match
	} else if (sym->variable.type == TYPE_DOUBLE && node->as.assign.expr->eval_type == TYPE_INT) {
		// Implicit conversion from Int to Double
		ast_node_t conversion_node = ast_create_node(NODE_TYPE_CAST, node->line_num);
		if (!conversion_node) {
			print_error(
				ERR_INTERNAL, node->line_num,
				"Failed to create type cast node for implicit conversion from Int to Double.");
			return false;
		}
		conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
		conversion_node->as.type_cast.operand = node->as.assign.expr;
		node->as.assign.expr = conversion_node;
		conversion_node->eval_type = TYPE_DOUBLE;
	} else {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"Type mismatch in assignment to variable '%s': expected '%s', got '%s'.",
					node->as.assign.target_id, data_type_string_forms[sym->variable.type],
					data_type_string_forms[node->as.assign.expr->eval_type]);
		return false;
	}

	// Set unique name for the target variable
	node->as.assign.unique_target_id = strdup(sym->variable.unique_name);
	if (!node->as.assign.unique_target_id) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to allocate memory for variable unique name in assignment.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_UNIT;

	return true;
}

bool sem_second_node_if_expr(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_if_expr on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_if_stmt(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_if_stmt on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_while(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_while on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_for(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_for on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_return(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_return on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_expr_stmt(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_expr_stmt on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_binary_op(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_binary_op on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_unary_op(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_unary_op on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_func_call(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_call on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_member_access(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_member_access on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_string_interp(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_string_interp on nullptr.");
	// TODO
	return true;
}

bool sem_second_node_arg_list(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_arg_list on nullptr.");

	// Node checks
	if (node->type != NODE_FUNC_CALL && node->type != NODE_MEMBER_ACCESS) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_arg_list on non function call node.");
		return false;
	}
	if (node->type == NODE_FUNC_CALL && !node->as.func_call.callee) {
		print_error(ERR_INTERNAL, node->line_num, "Function call has no callee.");
		return false;
	}

	// Get function symbol from the symbol table
	symbol_t function_symbol = nullptr;
	if (node->type == NODE_MEMBER_ACCESS) {
		if (!node->as.member_access.target) {
			print_error(ERR_INTERNAL, node->line_num, "Member access has no target.");
			return false;
		}
		if (!node->as.member_access.member_name) {
			print_error(ERR_INTERNAL, node->line_num, "Member access has no member name.");
			return false;
		}
		if (!node->as.member_access.target->eval_type) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access callee has no evaluation type.");
			return false;
		}

		// Make key
		char *key =
			malloc(strlen(data_type_string_forms[node->as.member_access.target->eval_type]) +
				   strlen(node->as.member_access.member_name) + 1 + 1);
		if (!key) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(key, "%s$%s", node->as.member_access.member_name,
				data_type_string_forms[node->as.member_access.target->eval_type]);

		// Search symtable
		function_symbol = stable_find_alllevel(context->symtable_extension_functions, key);
		free(key);
		if (!function_symbol) {
			print_error(
				ERR_SEM_UNDEFINED, node->line_num,
				"Extension function '%s' for type '%s' is not defined in the current scope.",
				node->as.member_access.member_name,
				data_type_string_forms[node->as.member_access.target->eval_type]);
			return false;
		}

		// Check extension parameter type
		if (function_symbol->function.extension_parameter_type !=
			node->as.member_access.target->eval_type) {
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
						"Extension function '%s' expects receiver type '%s', but got '%s'.",
						node->as.member_access.member_name,
						data_type_string_forms[function_symbol->function.extension_parameter_type],
						data_type_string_forms[node->as.member_access.target->eval_type]);
			return false;
		}

		// Check symbol has no parameter lists
		if (function_symbol->function.parameter_lists.count != 0) {
			print_error(ERR_SEM_PARAMS, node->line_num,
						"Extension function '%s' has parameter lists, but should not.",
						node->as.member_access.member_name);
			return false;
		}

		return true;
	} else if (node->type == NODE_FUNC_CALL &&
			   node->as.func_call.callee->type == NODE_MEMBER_ACCESS) {
		if (!node->as.func_call.callee->as.member_access.member_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access in function call has no member name.");
			return false;
		}
		if (!node->as.func_call.callee->as.member_access.target) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access in function call has no target.");
			return false;
		}
		if (!node->as.func_call.callee->as.member_access.target->eval_type) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access target in function call has no evaluation type.");
			return false;
		}

		// Make key
		char *key = malloc(strlen(data_type_string_forms[node->as.func_call.callee->as.member_access
															 .target->eval_type]) +
						   strlen(node->as.func_call.callee->as.member_access.member_name) + 1 + 1);
		if (!key) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(
			key, "%s$%s", node->as.func_call.callee->as.member_access.member_name,
			data_type_string_forms[node->as.func_call.callee->as.member_access.target->eval_type]);

		// Search symtable
		function_symbol = stable_find_alllevel(context->symtable_extension_functions, key);
		free(key);
		if (!function_symbol) {
			print_error(
				ERR_SEM_UNDEFINED, node->line_num,
				"Extension function '%s' for type '%s' is not defined in the current scope.",
				node->as.func_call.callee->as.member_access.member_name,
				data_type_string_forms[node->as.func_call.callee->as.member_access.target
										   ->eval_type]);
			return false;
		}
	} else {
		if (!node->as.func_call.callee) {
			print_error(ERR_INTERNAL, node->line_num, "Function call has no callee.");
			return false;
		}
		if (node->as.func_call.callee->type != NODE_IDENTIFIER) {
			print_error(ERR_INTERNAL, node->line_num,
						"Function call callee is not an identifier or member access.");
			return false;
		}
		if (!node->as.func_call.callee->as.id.name) {
			print_error(ERR_INTERNAL, node->line_num, "Function call callee has no name.");
			return false;
		}
		function_symbol = stable_find_alllevel(context->symtable_functions,
											   node->as.func_call.callee->as.id.name);
		if (!function_symbol) {
			print_error(ERR_SEM_UNDEFINED, node->line_num,
						"Function '%s' is not defined in the current scope.",
						node->as.func_call.callee->as.id.name);
			return false;
		}
	}

	// Check argument lists lengths
	if (function_symbol->function.parameter_lists.count != node->as.func_call.arg_lists.count) {
		print_error(
			ERR_SEM_PARAMS, node->line_num, "Function expects %zu argument lists, but got %zu.",
			function_symbol->function.parameter_lists.count, node->as.func_call.arg_lists.count);
		return false;
	}

	// Go through argument lists and check lengths
	ast_node_t current_argument_list_node = node->as.func_call.arg_lists.head;
	ast_node_t current_function_param_list_node = function_symbol->function.parameter_lists.head;
	while (current_argument_list_node && current_function_param_list_node) {
		// Node checks
		if (current_argument_list_node->type != NODE_ARG_LIST) {
			print_error(ERR_INTERNAL, current_argument_list_node->line_num,
						"Invalid node type in function call argument lists.");
			return false;
		}

		// Check argument list lengths
		if (current_argument_list_node->as.arg_list.args.count !=
			current_function_param_list_node->as.param_list.params.count) {
			print_error(ERR_SEM_PARAMS, current_argument_list_node->line_num,
						"Function expects %zu parameters in argument list, but got %zu.",
						current_function_param_list_node->as.param_list.params.count,
						current_argument_list_node->as.arg_list.args.count);
			return false;
		}

		// Go through arguments and check types
		ast_list_t *current_arg_list_for_cast_insertion =
			&current_argument_list_node->as.arg_list.args;
		ast_node_t previous_argument_node_for_cast_insertion = nullptr;

		ast_node_t current_argument_node = current_argument_list_node->as.arg_list.args.head;
		ast_node_t current_function_param_node =
			current_function_param_list_node->as.param_list.params.head;
		while (current_argument_node && current_function_param_node) {
			// Node checks
			if (current_function_param_node->type != NODE_PARAM) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Invalid node type in function parameter list.");
				return false;
			}

			// Get argument type
			if (!sem_second_expr(current_argument_node, context)) {
				throw_warning(current_argument_node->line_num,
							  "Failed to process function call argument for function.");
				return false;
			}

			// Check correct type
			if (current_argument_node->eval_type != current_function_param_node->eval_type) {
				if (current_function_param_node->eval_type == TYPE_DOUBLE &&
					current_argument_node->eval_type == TYPE_INT) {
					// Implicit conversion from Int to Double
					// Create cast node
					ast_node_t cast_node =
						ast_create_node(NODE_TYPE_CAST, current_argument_node->line_num);
					if (!cast_node) {
						print_error(
							ERR_INTERNAL, current_argument_node->line_num,
							"Failed to create type cast node for implicit conversion from Int "
							"to Double.");
						return false;
					}
					cast_node->eval_type = TYPE_DOUBLE;
					cast_node->as.type_cast.target_type = TYPE_DOUBLE;
					cast_node->as.type_cast.operand = current_argument_node;

					// Insert to argument list
					cast_node->next = current_argument_node->next;
					current_argument_node->next = nullptr;
					if (current_arg_list_for_cast_insertion->head == current_argument_node &&
						current_arg_list_for_cast_insertion->tail == current_argument_node) {
						// Only
						current_arg_list_for_cast_insertion->head = cast_node;
						current_arg_list_for_cast_insertion->tail = cast_node;
					} else if (!previous_argument_node_for_cast_insertion) {
						// First
						current_arg_list_for_cast_insertion->head = cast_node;
					} else if (!cast_node->next) {
						// Last
						previous_argument_node_for_cast_insertion->next = cast_node;
						current_arg_list_for_cast_insertion->tail = cast_node;
					} else {
						// Middle
						previous_argument_node_for_cast_insertion->next = cast_node;
					}
					current_argument_node = cast_node;

				} else if (current_function_param_node->eval_type != TYPE_ANY ||
						   !current_argument_node->eval_type) {
					print_error(ERR_SEM_PARAMS, current_argument_node->line_num,
								"Function expects type %s for parameter, but got %s.",
								data_type_string_forms[current_function_param_node->eval_type],
								data_type_string_forms[current_argument_node->eval_type]);
					return false;
				}
				// Type mismatch, but any type is allowed -> continue
			}

			// Move to next parameter
			previous_argument_node_for_cast_insertion = current_argument_node;
			current_argument_node = current_argument_node->next;
			current_function_param_node = current_function_param_node->next;
		}

		// Move to next parameter list
		current_argument_list_node = current_argument_list_node->next;
		current_function_param_list_node = current_function_param_list_node->next;
	}

	// TODO
	return true;
}

bool sem_second_node_identifier(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_identifier on nullptr.");

	// Node checks
	if (node->type != NODE_IDENTIFIER) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_identifier on non identifier node.");
		return false;
	}
	if (!node->as.id.name) {
		print_error(ERR_INTERNAL, node->line_num, "Identifier node has no name.");
		return false;
	}

	// Check if identifier is defined in the current scope
	symbol_t variable_symbol = stable_find_alllevel(context->symtable_local, node->as.id.name);
	symbol_t function_symbol = stable_find_alllevel(context->symtable_functions, node->as.id.name);

	// Check if identifier is defined
	if (variable_symbol) {
		node->eval_type = variable_symbol->variable.type;
		node->as.id.unique_name = strdup(variable_symbol->variable.unique_name);
		if (!node->as.id.unique_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for variable unique name in identifier.");
			return false;
		}
	} else if (function_symbol) {
		// TODO: change node to func call with calle identifier
		if (!sem_second_node_func_call(node, context)) {
			throw_warning(node->line_num, "Failed to process function call for identifier '%s'.",
						  node->as.id.name);
			return false;
		}
		node->eval_type = function_symbol->function.return_type;
	} else {
		print_error(ERR_SEM_UNDEFINED, node->line_num,
					"Identifier '%s' is not defined in the current scope.", node->as.id.name);
		return false;
	}

	return true;
}

bool sem_second_node_lit_int(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_lit_int on nullptr.");

	// Node checks
	if (node->type != NODE_LIT_INT) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_lit_int on non integer literal node.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_INT;

	return true;
}

bool sem_second_node_lit_double(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_lit_double on nullptr.");

	// Node checks
	if (node->type != NODE_LIT_DOUBLE) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_lit_double on non double literal node.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_DOUBLE;

	return true;
}

bool sem_second_node_lit_string(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_lit_string on nullptr.");

	// Node checks
	if (node->type != NODE_LIT_STRING) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_lit_string on non string literal node.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_STRING;

	return true;
}

bool sem_second_node_lit_bool(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_lit_bool on nullptr.");

	// Node checks
	if (node->type != NODE_LIT_BOOL) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_lit_bool on non boolean literal node.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_BOOL;

	return true;
}

bool sem_second_node_lit_unit(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_lit_unit on nullptr.");

	// Node checks
	if (node->type != NODE_LIT_UNIT) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_lit_unit on non unit literal node.");
		return false;
	}

	// Set eval type
	node->eval_type = TYPE_UNIT;

	return true;
}

bool sem_second_load_params(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_load_params on nullptr.");

	// Get function symbol from the symbol table
	symbol_t function_symbol = nullptr;
	if (node->type == NODE_FUNC_DEF && node->as.func_def.) {
		// Node checks
		if (!node->as.member_access.member_name) {
			print_error(ERR_INTERNAL, node->line_num, "Member access has no member name.");
			return false;
		}
		if (!node->as.member_access.target) {
			print_error(ERR_INTERNAL, node->line_num, "Member access has no target.");
			return false;
		}
		if (!node->as.member_access.target->eval_type) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access callee has no evaluation type.");
			return false;
		}

		// Make key
		char *key =
			malloc(strlen(data_type_string_forms[node->as.member_access.target->eval_type]) +
				   strlen(node->as.member_access.member_name) + 1 + 1);
		if (!key) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(key, "%s$%s", node->as.member_access.member_name,
				data_type_string_forms[node->as.member_access.target->eval_type]);

		// Search symtable
		function_symbol = stable_find_alllevel(context->symtable_extension_functions, key);
		free(key);
		if (!function_symbol) {
			print_error(
				ERR_SEM_UNDEFINED, node->line_num,
				"Extension function '%s' for type '%s' is not defined in the current scope.",
				node->as.member_access.member_name,
				data_type_string_forms[node->as.member_access.target->eval_type]);
			return false;
		}

		// Put the extension symbol to local symtable
		symbol_t local_extension_symbol = stable_init_symbol();
		if (!local_extension_symbol) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to create local extension function symbol.");
			return false;
		}
		local_extension_symbol->symbol_variant = SYMBOL_VARIABLE;
		local_extension_symbol->variable.type = function_symbol->function.extension_parameter_type;
		local_extension_symbol->variable.is_mutable = false;
		local_extension_symbol->variable.unique_name =
			strdup(function_symbol->function.extension_parameter_name);
		if (!local_extension_symbol->variable.unique_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for local extension function parameter name.");
			stable_destroy_symbol(local_extension_symbol);
			return false;
		}
		if (!stable_write(context->symtable_local,
						  function_symbol->function.extension_parameter_name,
						  local_extension_symbol)) {
			print_error(
				ERR_INTERNAL, node->line_num,
				"Failed to write local extension function parameter symbol to symbol table.");
			stable_destroy_symbol(local_extension_symbol);
			return false;
		}
	} else if (node->type == NODE_FUNC_CALL &&
			   node->as.func_call.callee->type == NODE_MEMBER_ACCESS) {
		// Node checks
		if (!node->as.func_call.callee->as.member_access.member_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access in function call has no member name.");
			return false;
		}
		if (!node->as.func_call.callee->as.member_access.target) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access in function call has no target.");
			return false;
		}
		if (!node->as.func_call.callee->as.member_access.target->eval_type) {
			print_error(ERR_INTERNAL, node->line_num,
						"Member access target in function call has no evaluation type.");
			return false;
		}

		// Make key
		char *key = malloc(strlen(data_type_string_forms[node->as.func_call.callee->as.member_access
															 .target->eval_type]) +
						   strlen(node->as.func_call.callee->as.member_access.member_name) + 1 + 1);
		if (!key) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(
			key, "%s$%s", node->as.func_call.callee->as.member_access.member_name,
			data_type_string_forms[node->as.func_call.callee->as.member_access.target->eval_type]);

		// Search symtable
		function_symbol = stable_find_alllevel(context->symtable_extension_functions, key);
		free(key);
		if (!function_symbol) {
			print_error(
				ERR_SEM_UNDEFINED, node->line_num,
				"Extension function '%s' for type '%s' is not defined in the current scope.",
				node->as.func_call.callee->as.member_access.member_name,
				data_type_string_forms[node->as.func_call.callee->as.member_access.target
										   ->eval_type]);
			return false;
		}

		// Put the extension symbol to local symtable
		symbol_t local_extension_symbol = stable_init_symbol();
		if (!local_extension_symbol) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to create local extension function symbol.");
			return false;
		}
		local_extension_symbol->symbol_variant = SYMBOL_VARIABLE;
		local_extension_symbol->variable.type = function_symbol->function.extension_parameter_type;
		local_extension_symbol->variable.is_mutable = false;
		local_extension_symbol->variable.unique_name =
			strdup(function_symbol->function.extension_parameter_name);
		if (!local_extension_symbol->variable.unique_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for local extension function parameter name.");
			stable_destroy_symbol(local_extension_symbol);
			return false;
		}
		if (!stable_write(context->symtable_local,
						  function_symbol->function.extension_parameter_name,
						  local_extension_symbol)) {
			print_error(
				ERR_INTERNAL, node->line_num,
				"Failed to write local extension function parameter symbol to symbol table.");
			stable_destroy_symbol(local_extension_symbol);
			return false;
		}
	} else {
		function_symbol = stable_find_alllevel(context->symtable_functions, node->as.func_def.name);
		if (!function_symbol) {
			print_error(ERR_SEM_UNDEFINED, node->line_num,
						"Function '%s' is not defined in the current scope.",
						node->as.func_def.name);
			return false;
		}
	}

	// Go through parameter lists
	ast_node_t current_function_param_list_node = function_symbol->function.parameter_lists.head;
	while (current_function_param_list_node) {
		// Node checks
		if (current_function_param_list_node->type != NODE_PARAM_LIST) {
			print_error(ERR_INTERNAL, current_function_param_list_node->line_num,
						"Invalid node type in function parameter lists.");
			return false;
		}

		// Go through parameters
		ast_node_t current_function_param_node =
			current_function_param_list_node->as.param_list.params.head;
		while (current_function_param_node) {
			// Node checks
			if (current_function_param_node->type != NODE_PARAM) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Invalid node type in function parameter list.");
				return false;
			}

			// Check if parameter is defined
			if (stable_find_alllevel(context->symtable_local,
									 current_function_param_node->as.param.name)) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Parameter '%s' is already defined in the current scope.",
							current_function_param_node->as.param.name);
				return false;
			}

			// Create symbol
			symbol_t param_symbol = stable_init_symbol();
			if (!param_symbol) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Failed to create parameter symbol for parameter '%s'.",
							current_function_param_node->as.param.name);
				return false;
			}

			// Fill out symbol
			param_symbol->symbol_variant = SYMBOL_VARIABLE;
			param_symbol->variable.is_mutable = false; // Parameters are immutable
			param_symbol->variable.type = current_function_param_node->eval_type;
			param_symbol->variable.unique_name = strdup(current_function_param_node->as.param.name);
			if (!param_symbol->variable.unique_name) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Failed to allocate memory for parameter unique name '%s'.",
							current_function_param_node->as.param.name);
				stable_destroy_symbol(param_symbol);
				return false;
			}

			// Set unique name in AST node
			current_function_param_node->as.param.unique_name =
				strdup(param_symbol->variable.unique_name);
			if (!current_function_param_node->as.param.unique_name) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Failed to allocate memory for parameter unique name in AST node '%s'.",
							current_function_param_node->as.param.name);
				stable_destroy_symbol(param_symbol);
				return false;
			}

			// Store symbol in the local symbol table
			if (!stable_write(context->symtable_local, current_function_param_node->as.param.name,
							  param_symbol)) {
				print_error(ERR_INTERNAL, current_function_param_node->line_num,
							"Failed to store parameter '%s' in the local symbol table.",
							current_function_param_node->as.param.name);
				stable_destroy_symbol(param_symbol);
				return false;
			}

			// Move to next parameter
			current_function_param_node = current_function_param_node->next;
		}

		// Move to next parameter list
		current_function_param_list_node = current_function_param_list_node->next;
	}

	// TODO
	return true;
}
