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

	if (!sem_first(root, &context)) {
		semantic_context_dtor(&context);
		return false;
	}

	if (!sem_second(root, &context)) {
		semantic_context_dtor(&context);
		return false;
	}
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

	if (context->call_chain.top != nullptr) {
		call_chain_destroy(context);
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

	// Add pointer to function definition node to symtable
	symbol->function.function_definition = node;

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
			(node->as.func_def.param_lists.head &&	   // No parameter list
			 node->as.func_def.param_lists.head->as.param_list.params.count != 0)) { // It is empty
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

		symbol->function.function_definition = current_extension_node;

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

bool sem_second(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second on nullptr.");
	ast_node_t current_node = node->as.program.items.head;

	while (current_node) {
		switch (current_node->type) {
		case NODE_FUNC_DEF:
			if (!sem_second_node_func_def(current_node, context)) {
				return false;
			}
			break;
		case NODE_EXTENSION:
			if (!sem_second_node_extension(current_node, context)) {
				return false;
			}
			break;
		default:
			print_error(ERR_INTERNAL, current_node->line_num, "Wrong node in program found!");
		}
		current_node = current_node->next;
	}
	return true;
}

bool sem_second_node_block_body(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_block_body on nullptr.");

	ast_node_t current_node = node->as.block.statements.head;
	ast_node_t last_node = current_node;

	while (current_node) {
		if (!sem_second_stmt(current_node, context)) {
			return false;
		}

		current_node = current_node->next;
		if (current_node != nullptr) {
			last_node = last_node->next;
		}
	}

	if (!node->as.block.statements.head) {
		node->eval_type = TYPE_UNIT;
	} else {
		node->eval_type = last_node->eval_type;
	}

	return true;
}

bool sem_second_node_block(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_block on nullptr.");
	if (!stable_enter_scope(context->symtable_local)) {
		print_error(ERR_INTERNAL, node->line_num, "Symtable enter scope failed!");
		return false;
	}

	if (!sem_second_node_block_body(node, context)) {
		stable_leave_scope(context->symtable_local);
		return false;
	}

	stable_leave_scope(context->symtable_local);
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Second pass of the extension implemented with AI assistance according to the design of the
// author (the symbol of each method is found by the key name$Type and analyzed).
// Prompt: Write sem_second_node_extension. Set the unique receiver name and analyze every method
// of the extension with its symbol.
bool sem_second_node_extension(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_extension on nullptr.");
	ast_node_t method = node->as.extension.methods.head;
	symbol_t method_symbol;
	char *key;

	// The receiver variable is declared with this name by the code generator
	node->as.extension.unique_receiver_name = strdup(node->as.extension.receiver_name);
	if (!node->as.extension.unique_receiver_name) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to allocate memory for the unique receiver name.");
		return false;
	}

	while (method) {
		key =
			malloc(strlen(method->as.func_def.name) + strlen(node->as.extension.receiver_type) + 2);
		if (!key) {
			print_error(ERR_INTERNAL, method->line_num,
						"Failed to allocate memory for extension function key.");
			return false;
		}
		sprintf(key, "%s$%s", method->as.func_def.name, node->as.extension.receiver_type);

		method_symbol = stable_find_curlevel(context->symtable_extension_functions, key);
		free(key);

		if (!sem_second_analyze_function(method, context, method_symbol)) {
			return false;
		}

		method = method->next;
	}

	return true;
}

// Appends an empty return (Unit) to the end of a block
bool sem_second_append_empty_return(ast_node_t block_node) {
	ast_node_t return_node = ast_create_node(NODE_RETURN, block_node->line_num);
	if (!return_node) {
		print_error(ERR_INTERNAL, block_node->line_num, "AST node allocation failed!");
		return false;
	}

	return_node->as.return_stmt.expr = nullptr;
	return_node->eval_type = TYPE_UNIT;

	if (!ast_list_append(&block_node->as.block.statements, return_node)) {
		print_error(ERR_INTERNAL, block_node->line_num, "AST node append failed!");
		ast_dtor(return_node);
		return false;
	}

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the implicit returns implemented with AI assistance according to the design of the
// author (the third rule of the return type in the specification).
// Prompt: Write a function that checks the value type of a branch of the if at the end of a
// function. A missing branch has the Unit value, without the explicit type all branches are Unit.
bool sem_second_check_branch_type(ast_node_t branch, unsigned line_num, data_type_t expected) {
	data_type_t branch_type = TYPE_UNIT;

	if (branch) {
		branch_type = branch->eval_type;

		if (branch->type == NODE_BLOCK && branch->as.block.statements.tail &&
			branch->as.block.statements.tail->type == NODE_IF) {
			return true;
		}
	}

	// The explicit Unit discards the values
	if (expected == TYPE_UNIT) {
		return true;
	}

	// Without the explicit type all the branches have to be Unit
	if (expected == TYPE_UNKNOWN) {
		expected = TYPE_UNIT;
	}
	if (branch_type == expected || (expected == TYPE_DOUBLE && branch_type == TYPE_INT)) {
		return true;
	}

	print_error(ERR_SEM_TYPE_COMPAT, line_num,
				"The value of a branch does not match the return type of the function.");
	return false;
}

// Inserts the implicit return into one branch of an if (block or one-line statement)
bool sem_second_insert_return_into_branch(ast_node_t branch, semantic_context_t *context,
										  data_type_t expected) {
	assert(branch && context && "Called sem_second_insert_return_into_branch on nullptr.");
	ast_node_t conversion_node;

	switch (branch->type) {
	case NODE_BLOCK:
		return sem_second_insert_implicit_return(branch, context, expected);
	case NODE_EXPR_STMT:
		// One-line branch, Int is converted to the declared Double
		if (expected == TYPE_DOUBLE && branch->as.expr_stmt.expr->eval_type == TYPE_INT) {
			// Tool: Claude Code (Claude Sonnet 5.5)
			// Insertion of the Int -> Double type cast into the implicit returns implemented
			// with AI assistance (pattern taken from sem_second_node_assign).
			// Prompt: Insert the type cast in the implicit return, pass the expected type.
			conversion_node = ast_create_node(NODE_TYPE_CAST, branch->line_num);
			if (!conversion_node) {
				print_error(ERR_INTERNAL, branch->line_num,
							"Failed to create type cast node for the implicit return.");
				return false;
			}

			conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
			conversion_node->as.type_cast.operand = branch->as.expr_stmt.expr;
			conversion_node->eval_type = TYPE_DOUBLE;
			branch->as.expr_stmt.expr = conversion_node;
			branch->eval_type = TYPE_DOUBLE;
		}
		branch->type = NODE_RETURN;
		return true;
	default:
		print_error(ERR_INTERNAL, branch->line_num, "Pasting node return into if failed!");
		return false;
	}
}

// Inserts the implicit return at the end of a block
bool sem_second_insert_implicit_return(ast_node_t node, semantic_context_t *context,
									   data_type_t expected) {
	assert(node && context && "Called sem_second_insert_implicit_return on nullptr.");
	bool returns_unit = expected == TYPE_UNIT;
	ast_node_t current_if;
	ast_node_t branch;

	ast_node_t last_stmt_node = node->as.block.statements.tail;

	// Empty block
	if (last_stmt_node == nullptr) {
		return sem_second_append_empty_return(node);
	}

	switch (last_stmt_node->type) {
	case NODE_RETURN:
		// Already returns
		return true;
	case NODE_EXPR_STMT:
		// Unit: value is discarded, add an empty return; otherwise return the value
		if (returns_unit) {
			return sem_second_append_empty_return(node);
		}
		return sem_second_insert_return_into_branch(last_stmt_node, context, expected);
	case NODE_VAR_DEF:
	case NODE_ASSIGN:
	case NODE_FOR:
	case NODE_WHILE:
		// Result is Unit
		return sem_second_append_empty_return(node);
	case NODE_IF:
		// Unit: one empty return after the if, otherwise return in every branch
		if (returns_unit) {
			return sem_second_append_empty_return(node);
		}

		current_if = last_stmt_node;
		// Walk the if / else if chain
		while (current_if) {
			if (!sem_second_check_branch_type(current_if->as.if_stmt.then_branch,
											  current_if->line_num, expected) ||
				!sem_second_insert_return_into_branch(current_if->as.if_stmt.then_branch, context,
													  expected)) {
				return false;
			}

			branch = current_if->as.if_stmt.else_branch;
			if (branch != nullptr && branch->type == NODE_IF) {
				current_if = branch;
			} else {
				if (!sem_second_check_branch_type(branch, current_if->line_num, expected) ||
					(branch != nullptr &&
					 !sem_second_insert_return_into_branch(branch, context, expected))) {
					return false;
				}
				current_if = nullptr;
			}
		}
		return true;
	default:
		print_error(ERR_INTERNAL, node->line_num,
					"Wrong type in a block, failed to paste returns!");
		return false;
	}
}

// Analyzes the function body and inserts the implicit returns
bool sem_second_func_analyze_body(ast_node_t node, semantic_context_t *context,
								  symbol_t current_function) {
	assert(node && context && current_function &&
		   "Called sem_second_func_analyze_body on nullptr.");
	data_type_t expected_type;
	ast_node_t block_node;
	ast_node_t statement_node;

	// Tool: Claude Code (Claude Sonnet 5.5)
	// The one-line body is a block with one statement, changed with AI assistance.
	// Prompt: Wrap the one-line body of a function into a block with an expression statement.
	if (node->as.func_def.body->type != NODE_BLOCK) {
		block_node = ast_create_node(NODE_BLOCK, node->as.func_def.body->line_num);
		statement_node = ast_create_node(NODE_EXPR_STMT, node->as.func_def.body->line_num);
		if (!block_node || !statement_node ||
			!ast_list_append(&block_node->as.block.statements, statement_node)) {
			print_error(ERR_INTERNAL, node->line_num, "AST node allocation failed");
			ast_dtor(block_node);
			ast_dtor(statement_node);
			return false;
		}

		statement_node->as.expr_stmt.expr = node->as.func_def.body;
		node->as.func_def.body = block_node;
	}

	// checks the whole block and sets block eval type
	if (!sem_second_node_block_body(node->as.func_def.body, context)) {
		return false;
	}

	// TYPE_UNKNOWN means that the return type is not explicit
	expected_type = TYPE_UNKNOWN;
	if (current_function->function.has_explicit_return_type) {
		expected_type = current_function->function.return_type;
	}

	return sem_second_insert_implicit_return(node->as.func_def.body, context, expected_type);
}

// Checks the body type against the declared return type or infers the return type
bool sem_second_check_return_type(ast_node_t node, symbol_t current_function) {
	assert(node && current_function && "Called sem_second_check_return_type on nullptr.");
	data_type_t body_type;
	ast_node_t last_node = node->as.func_def.body;

	// Tool: Claude Code (Claude Sonnet 5.5)
	// Type of the last statement instead of the block type, changed with AI assistance.
	// Prompt: Fix check_return_type to use the type of the last statement of the body.
	if (last_node->type == NODE_BLOCK) {
		last_node = last_node->as.block.statements.tail;
	}

	// Ends with if, the returns in its branches decide the type
	if (last_node->type != NODE_RETURN) {
		if (!current_function->function.has_explicit_return_type) {
			current_function->function.return_type = TYPE_UNIT;
		}
		return true;
	}

	body_type = last_node->eval_type;

	if (current_function->function.has_explicit_return_type) {
		data_type_t expected = current_function->function.return_type;

		if (expected != TYPE_UNIT && body_type != expected) {
			if (body_type == TYPE_INT && expected == TYPE_DOUBLE) {
				// Tool: Claude Code (Claude Sonnet 5.5)
				// Insertion of the Int -> Double type cast into the return of a one-line body
				// implemented with AI assistance (pattern taken from sem_second_node_assign).
				// Prompt: Add insertion of the type cast node.
				if (node->as.func_def.body->type == NODE_RETURN) {
					ast_node_t return_node = node->as.func_def.body;
					ast_node_t conversion_node =
						ast_create_node(NODE_TYPE_CAST, return_node->line_num);
					if (!conversion_node) {
						print_error(ERR_INTERNAL, return_node->line_num,
									"Failed to create type cast node for the function return.");
						return false;
					}

					conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
					conversion_node->as.type_cast.operand = return_node->as.return_stmt.expr;
					conversion_node->eval_type = TYPE_DOUBLE;
					return_node->as.return_stmt.expr = conversion_node;
					return_node->eval_type = TYPE_DOUBLE;
				}
				// Block body: the cast is inserted together with the returns
			} else {
				print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
							"Function return type does not match the declared return type");
				return false;
			}
		}
	} else {
		if (body_type == TYPE_UNKNOWN) {
			print_error(ERR_INTERNAL, node->line_num, "Function body type has not been set!");
			return false;
		}
		current_function->function.return_type = body_type;
	}

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// The body of sem_second_node_func_def was moved here with AI assistance, the symbol is passed
// as a parameter so it works for functions and for extension methods.
// Prompt: Split func_def so the analysis gets the function symbol as a parameter.
bool sem_second_analyze_function(ast_node_t node, semantic_context_t *context,
								 symbol_t current_function) {
	assert(node && context && current_function && "Called sem_second_analyze_function on nullptr.");
	bool success = true;
	bool pushed = false; // if malloc fails

	// store previous local table
	stable_t prev_local_table = context->symtable_local;

	// store previous function name
	char *prev_function_name = context->current_function_name;
	context->current_function_name = node->as.func_def.name;

	if (current_function->function.state == FUNCTION_VISITED) {
		context->current_function_name = prev_function_name;
		return true;
	}

	current_function->function.state = FUNCTION_VISITING;

	if (!call_chain_add(context, node)) {
		print_error(ERR_INTERNAL, node->line_num, "Call chain node allocation failed!");
		success = false;
		goto cleanup;
	}

	pushed = true;

	context->symtable_local = stable_init();

	if (!context->symtable_local) {
		print_error(ERR_INTERNAL, node->line_num, "Symtable init failed!");
		success = false;
		goto cleanup;
	}

	// load params to local symtable - set is_mutable = false
	if (!sem_second_load_params(node, context, current_function)) {
		success = false;
		goto cleanup;
	}

	// analyze the body and paste returns
	if (!sem_second_func_analyze_body(node, context, current_function)) {
		success = false;
		goto cleanup;
	}

	// check or infer the return type
	if (!sem_second_check_return_type(node, current_function)) {
		success = false;
		goto cleanup;
	}

cleanup:
	context->current_function_name = prev_function_name;
	stable_dtor(context->symtable_local);
	context->symtable_local = prev_local_table;

	if (success) {
		current_function->function.state = FUNCTION_VISITED;
	}

	if (pushed) {
		call_chain_pop(context);
	}

	return success;
}

bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_def on nullptr.");

	symbol_t current_function =
		stable_find_curlevel(context->symtable_functions, node->as.func_def.name);
	assert(current_function != nullptr);

	return sem_second_analyze_function(node, context, current_function);
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
		malloc(strlen(node->as.var_def.name) + SIZE_T_MAX_DIGITS + 1 + 1); // Digits + \0 + $
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

// Tool: Claude Code (Claude Sonnet 5.5)
// Implemented with AI assistance, the if statement and the if expression share the analysis and
// the is_expr flag of the node decides.
// Prompt: Write sem_second_node_if_expr, it only calls the analysis of the if statement.
bool sem_second_node_if_expr(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_if_expr on nullptr.");

	return sem_second_node_if_stmt(node, context);
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the if and the for implemented with AI assistance according to the design of the
// author.
// Prompt: Write a function that analyzes a branch of an if or a body of a for, a block has its own
// scope and a one-line body is a statement.
bool sem_second_branch_body(ast_node_t branch, semantic_context_t *context) {
	assert(branch && context && "Called sem_second_branch_body on nullptr.");

	if (branch->type == NODE_BLOCK) {
		return sem_second_node_block(branch, context);
	}

	return sem_second_stmt(branch, context);
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the if expression implemented with AI assistance according to the design of the
// author.
// Prompt: Write a function that converts the value of an Int branch of an if expression to
// Double. The branch is an expression statement, a block or a nested if.
bool sem_second_if_branch_to_double(ast_node_t branch) {
	assert(branch && "Called sem_second_if_branch_to_double on nullptr.");
	ast_node_t value_node = branch;

	// else if, all the branches of the nested if have the value
	if (branch->type == NODE_IF) {
		if (!sem_second_if_branch_to_double(branch->as.if_stmt.then_branch) ||
			!sem_second_if_branch_to_double(branch->as.if_stmt.else_branch)) {
			return false;
		}
		branch->eval_type = TYPE_DOUBLE;
		return true;
	}

	// The value of a block is the expression statement at its end
	if (branch->type == NODE_BLOCK) {
		value_node = branch->as.block.statements.tail;
	}

	if (!sem_second_insert_double_cast(&value_node->as.expr_stmt.expr)) {
		return false;
	}
	value_node->eval_type = TYPE_DOUBLE;
	branch->eval_type = TYPE_DOUBLE;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// If statement and if expression implemented with AI assistance step by step according to the
// design of the author (one function, the is_expr flag of the node decides what it is).
// Prompt: Write sem_second_node_if_stmt. Analyze the condition and check that it is Boolean, the
// branches have their own scope. The statement has the Unit type, the branches of the expression
// have the same type or Int is converted to Double.
bool sem_second_node_if_stmt(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_if_stmt on nullptr.");
	data_type_t then_type;
	data_type_t else_type;

	if (!sem_second_expr(node->as.if_stmt.condition, context)) {
		return false;
	}

	if (node->as.if_stmt.condition->eval_type != TYPE_BOOL) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num, "The condition of if must be Boolean.");
		return false;
	}

	if (!sem_second_branch_body(node->as.if_stmt.then_branch, context)) {
		return false;
	}

	// The else branch can be missing, else if is a nested if
	if (node->as.if_stmt.else_branch &&
		!sem_second_branch_body(node->as.if_stmt.else_branch, context)) {
		return false;
	}

	if (!node->as.if_stmt.is_expr) {
		node->eval_type = TYPE_UNIT;
		return true;
	}

	// If expression, the type of the value is the type of the branches
	then_type = node->as.if_stmt.then_branch->eval_type;
	else_type = node->as.if_stmt.else_branch->eval_type;
	node->eval_type = then_type;

	if (then_type == TYPE_INT && else_type == TYPE_DOUBLE) {
		node->eval_type = TYPE_DOUBLE;
		return sem_second_if_branch_to_double(node->as.if_stmt.then_branch);
	}
	if (then_type == TYPE_DOUBLE && else_type == TYPE_INT) {
		return sem_second_if_branch_to_double(node->as.if_stmt.else_branch);
	}

	if (then_type != else_type) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"The branches of the if expression have different types.");
		return false;
	}

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// While cycle implemented with AI assistance according to the design of the author.
// Prompt: Write sem_second_node_while. The condition has to be Boolean, the body is a block with
// its own scope and the cycle has the Unit type.
bool sem_second_node_while(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_while on nullptr.");

	if (!sem_second_expr(node->as.while_stmt.condition, context)) {
		return false;
	}

	if (node->as.while_stmt.condition->eval_type != TYPE_BOOL) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num, "The condition of while must be Boolean.");
		return false;
	}

	if (!sem_second_node_block(node->as.while_stmt.body, context)) {
		return false;
	}

	node->eval_type = TYPE_UNIT;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the for implemented with AI assistance according to the design of the author.
// Prompt: Write a function that analyzes the bounds and the step of the for range, they are Int
// and a literal zero step is an error.
bool sem_second_for_range(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_for_range on nullptr.");
	ast_node_t bounds[3] = {node->as.for_stmt.from_expr, node->as.for_stmt.to_expr,
							node->as.for_stmt.step_expr};

	for (int i = 0; i < 3; i++) {
		// Missing step
		if (!bounds[i]) {
			continue;
		}

		if (!sem_second_expr(bounds[i], context)) {
			return false;
		}

		if (bounds[i]->eval_type != TYPE_INT) {
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
						"The bounds and the step of the for range must be Int.");
			return false;
		}
	}

	// Literal zero step
	if (node->as.for_stmt.step_expr && node->as.for_stmt.step_expr->type == NODE_LIT_INT &&
		node->as.for_stmt.step_expr->as.int_val == 0) {
		print_error(ERR_SEM_OTHER, node->line_num, "The step of the for range cannot be zero.");
		return false;
	}

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the for implemented with AI assistance according to the design of the author.
// Prompt: Write a function that creates the loop variable of the for, an immutable Int with a
// unique name stored in the symtable and in the node.
bool sem_second_for_variable(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_for_variable on nullptr.");
	symbol_t loop_symbol = stable_init_symbol();

	if (loop_symbol) {
		loop_symbol->symbol_variant = SYMBOL_VARIABLE;
		loop_symbol->variable.type = TYPE_INT;
		loop_symbol->variable.is_mutable = false;
		loop_symbol->variable.unique_name =
			malloc(strlen(node->as.for_stmt.var_name) + SIZE_T_MAX_DIGITS + 1 + 1);
	}
	if (!loop_symbol || !loop_symbol->variable.unique_name) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to create the loop variable.");
		stable_destroy_symbol(loop_symbol);
		return false;
	}
	sprintf(loop_symbol->variable.unique_name, "%s$%zu", node->as.for_stmt.var_name,
			context->rename_counter++);

	node->as.for_stmt.unique_var_name = strdup(loop_symbol->variable.unique_name);
	if (!node->as.for_stmt.unique_var_name ||
		!stable_write(context->symtable_local, node->as.for_stmt.var_name, loop_symbol)) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to store the loop variable.");
		stable_destroy_symbol(loop_symbol);
		return false;
	}

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// For cycle implemented with AI assistance step by step according to the design of the author.
// Prompt: Write sem_second_node_for. The range and the step are Int, the loop variable has its own
// scope, the guards are Boolean and the body is analyzed.
bool sem_second_node_for(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_for on nullptr.");
	bool success = true;

	if (!sem_second_for_range(node, context)) {
		return false;
	}

	// The loop variable has its own scope
	if (!stable_enter_scope(context->symtable_local)) {
		print_error(ERR_INTERNAL, node->line_num, "Symtable enter scope failed!");
		return false;
	}

	if (!sem_second_for_variable(node, context)) {
		success = false;
		goto cleanup;
	}

	// Guards see the loop variable
	for (ast_node_t guard = node->as.for_stmt.guards.head; guard; guard = guard->next) {
		if (!sem_second_expr(guard, context)) {
			success = false;
			goto cleanup;
		}

		if (guard->eval_type != TYPE_BOOL) {
			print_error(ERR_SEM_TYPE_COMPAT, guard->line_num, "The guard of for must be Boolean.");
			success = false;
			goto cleanup;
		}
	}

	// The body is a block with its own scope or a one-line statement
	if (!sem_second_branch_body(node->as.for_stmt.body, context)) {
		success = false;
		goto cleanup;
	}

	node->eval_type = TYPE_UNIT;

cleanup:
	stable_leave_scope(context->symtable_local);
	return success;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Return statement implemented with AI assistance according to the design of the author (the
// function is the top of the call chain, the type is checked against the explicit return type).
// Prompt: Write sem_second_node_return. Return needs an explicit return type, the empty return is
// allowed only for Unit, Int is converted to Double, other mismatch is the error 6.
bool sem_second_node_return(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_return on nullptr.");
	ast_node_t function_node;
	ast_node_t conversion_node;
	data_type_t expected;

	function_node = context->call_chain.top->function_node;

	if (!function_node->as.func_def.return_type) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"Function with return needs an explicit return type.");
		return false;
	}
	expected = get_data_type_from_string(function_node->as.func_def.return_type);

	// Empty return
	if (!node->as.return_stmt.expr) {
		if (expected != TYPE_UNIT) {
			print_error(ERR_SEM_OTHER, node->line_num,
						"Return without a value in a function that does not return Unit.");
			return false;
		}
		node->eval_type = TYPE_UNIT;
		return true;
	}

	if (!sem_second_expr(node->as.return_stmt.expr, context)) {
		return false;
	}
	node->eval_type = node->as.return_stmt.expr->eval_type;

	// Unit discards the value, same types are fine
	if (expected == TYPE_UNIT || node->eval_type == expected) {
		return true;
	}

	if (node->eval_type != TYPE_INT || expected != TYPE_DOUBLE) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"Returned value does not match the declared return type.");
		return false;
	}

	// Implicit conversion from Int to Double
	conversion_node = ast_create_node(NODE_TYPE_CAST, node->line_num);
	if (!conversion_node) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to create type cast node for the returned value.");
		return false;
	}
	conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
	conversion_node->as.type_cast.operand = node->as.return_stmt.expr;
	conversion_node->eval_type = TYPE_DOUBLE;
	node->as.return_stmt.expr = conversion_node;
	node->eval_type = TYPE_DOUBLE;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Expression statement implemented with AI assistance according to the design of the author.
// Prompt: Write sem_second_node_expr_stmt. Analyze the expression and copy its eval type.
bool sem_second_node_expr_stmt(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_expr_stmt on nullptr.");

	if (!sem_second_expr(node->as.expr_stmt.expr, context)) {
		return false;
	}

	node->eval_type = node->as.expr_stmt.expr->eval_type;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper of the binary operator implemented with AI assistance according to the design of the
// author (the rules of the operators are taken from the specification, chapter 3).
// Prompt: Write a function that decides if the operand types are valid for the binary operator
// and what is the type of the result.
bool sem_second_binary_op_result_type(op_type_t operator_type, data_type_t left_type,
									  data_type_t right_type, data_type_t *result_type) {
	assert(result_type && "Called sem_second_binary_op_result_type on nullptr.");
	bool numbers = (bool)((left_type == TYPE_INT || left_type == TYPE_DOUBLE) &&
						  (right_type == TYPE_INT || right_type == TYPE_DOUBLE));

	switch (operator_type) {
	case OPERATOR_PLUS:
	case OPERATOR_MINUS:
	case OPERATOR_STAR:
	case OPERATOR_SLASH:
		if (numbers) {
			*result_type = TYPE_DOUBLE;
			if (left_type == TYPE_INT && right_type == TYPE_INT) {
				*result_type = TYPE_INT;
			}
			return true;
		}
		// Concatenation
		if (operator_type == OPERATOR_PLUS && left_type == TYPE_STRING &&
			right_type == TYPE_STRING) {
			*result_type = TYPE_STRING;
			return true;
		}
		return false;
	case OPERATOR_LESS:
	case OPERATOR_LESS_EQUAL:
	case OPERATOR_GREATER:
	case OPERATOR_GREATER_EQUAL:
		*result_type = TYPE_BOOL;
		return (bool)(numbers || (left_type == TYPE_STRING && right_type == TYPE_STRING));
	case OPERATOR_EQUAL:
	case OPERATOR_NOT_EQUAL:
		*result_type = TYPE_BOOL;
		return (bool)(numbers || left_type == right_type);
	case OPERATOR_AND:
	case OPERATOR_OR:
		*result_type = TYPE_BOOL;
		return (bool)(left_type == TYPE_BOOL && right_type == TYPE_BOOL);
	default:
		return false;
	}
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Implemented with AI assistance (pattern taken from sem_second_node_assign).
// Prompt: Write a function that wraps an operand into the type cast node from Int to Double.
bool sem_second_insert_double_cast(ast_node_t *operand) {
	assert(operand && *operand && "Called sem_second_insert_double_cast on nullptr.");

	ast_node_t conversion_node = ast_create_node(NODE_TYPE_CAST, (*operand)->line_num);
	if (!conversion_node) {
		print_error(ERR_INTERNAL, (*operand)->line_num, "Failed to create type cast node.");
		return false;
	}

	conversion_node->as.type_cast.target_type = TYPE_DOUBLE;
	conversion_node->as.type_cast.operand = *operand;
	conversion_node->eval_type = TYPE_DOUBLE;
	*operand = conversion_node;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Binary operator implemented with AI assistance step by step according to the design of the
// author (the rules of the operators are taken from the specification, chapter 3).
// Prompt: Write sem_second_node_binary_op. Analyze the operands, check their types, convert Int to
// Double and set the type of the result.
bool sem_second_node_binary_op(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_binary_op on nullptr.");
	data_type_t left_type;
	data_type_t right_type;
	data_type_t result_type;

	if (!sem_second_expr(node->as.binary_op.left, context) ||
		!sem_second_expr(node->as.binary_op.right, context)) {
		return false;
	}

	left_type = node->as.binary_op.left->eval_type;
	right_type = node->as.binary_op.right->eval_type;

	if (!sem_second_binary_op_result_type(node->as.binary_op.operator_type, left_type, right_type,
										  &result_type)) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"Type mismatch of the operands of the binary operator.");
		return false;
	}

	// The Int operand is converted if the other one is Double
	if (left_type == TYPE_INT && right_type == TYPE_DOUBLE &&
		!sem_second_insert_double_cast(&node->as.binary_op.left)) {
		return false;
	}
	if (left_type == TYPE_DOUBLE && right_type == TYPE_INT &&
		!sem_second_insert_double_cast(&node->as.binary_op.right)) {
		return false;
	}

	node->eval_type = result_type;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Unary operator implemented with AI assistance according to the design of the author (the rules
// are taken from the specification, chapter 3 and the extension BOOLTHEN).
// Prompt: Write sem_second_node_unary_op. The unary + and - are for numbers, the ! is for Boolean,
// the type of the result is the type of the operand.
bool sem_second_node_unary_op(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_unary_op on nullptr.");
	data_type_t operand_type;
	bool valid;

	if (!sem_second_expr(node->as.unary_op.operand, context)) {
		return false;
	}
	operand_type = node->as.unary_op.operand->eval_type;

	if (node->as.unary_op.operator_type == OPERATOR_EX_MARK) {
		valid = operand_type == TYPE_BOOL;
	} else {
		valid = (bool)(operand_type == TYPE_INT || operand_type == TYPE_DOUBLE);
	}

	if (!valid) {
		print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
					"Type mismatch of the operand of the unary operator.");
		return false;
	}

	node->eval_type = operand_type;
	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Helper that finds the symbol of an extension method for a member access, implemented with AI
// assistance according to the specification of the author (pattern of the lookup is taken from
// sem_second_node_arg_list).
// Prompt: Write sem_second_find_extension_symbol, it analyzes the target of the member access,
// builds the key name$Type, searches symtable_extension_functions and prints an error when the
// method is not found.
symbol_t sem_second_find_extension_symbol(ast_node_t member_access_node,
										  semantic_context_t *context) {
	assert(member_access_node && context && "Called sem_second_find_extension_symbol on nullptr.");
	ast_node_t target = member_access_node->as.member_access.target;
	char *member_name = member_access_node->as.member_access.member_name;
	char *key;
	symbol_t extension_symbol;

	if (!sem_second_expr(target, context)) {
		return nullptr;
	}

	// Key is name$Type, the same as in the first pass
	key = malloc(strlen(member_name) + strlen(data_type_string_forms[target->eval_type]) + 2);
	if (!key) {
		print_error(ERR_INTERNAL, member_access_node->line_num,
					"Failed to allocate memory for extension function key.");
		return nullptr;
	}
	sprintf(key, "%s$%s", member_name, data_type_string_forms[target->eval_type]);

	extension_symbol = stable_find_alllevel(context->symtable_extension_functions, key);
	free(key);

	if (!extension_symbol) {
		print_error(ERR_SEM_UNDEFINED, member_access_node->line_num,
					"Extension function '%s' for type '%s' is not defined.", member_name,
					data_type_string_forms[target->eval_type]);
	}

	return extension_symbol;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Function call analysis implemented with AI assistance according to the design of the author
// (symbol lookup, recursion check with the call chain, arguments checked by
// sem_second_node_arg_list).
// Prompt: Write sem_second_node_func_call. Find the symbol of the callee (identifier or member
// access), analyze unvisited functions, check the recursion of the visiting ones, check the
// arguments and set the eval type to the return type of the function.
bool sem_second_node_func_call(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_call on nullptr.");
	ast_node_t callee = node->as.func_call.callee;
	symbol_t function_symbol;

	// Find the symbol of the called function
	if (callee->type == NODE_IDENTIFIER) {
		function_symbol = stable_find_alllevel(context->symtable_functions, callee->as.id.name);
		if (!function_symbol) {
			print_error(ERR_SEM_UNDEFINED, node->line_num,
						"Function '%s' is not defined in the current scope.", callee->as.id.name);
			return false;
		}
	} else {
		function_symbol = sem_second_find_extension_symbol(callee, context);
		if (!function_symbol) {
			return false;
		}
	}

	// Analyze the function first, the return type has to be known
	if (function_symbol->function.state == FUNCTION_UNVISITED) {
		if (!sem_second_analyze_function(function_symbol->function.function_definition, context,
										 function_symbol)) {
			return false;
		}
	} else if (function_symbol->function.state == FUNCTION_VISITING) {
		// Recursion, every function in the cycle needs an explicit return type
		if (!call_chain_check_cycle(context, function_symbol->function.function_definition)) {
			return false;
		}
	}

	if (!sem_second_node_arg_list(node, context)) {
		return false;
	}

	node->eval_type = function_symbol->function.return_type;
	return true;
}

bool sem_second_node_member_access(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_member_access on nullptr.");

	symbol_t mem_acc_symbol = sem_second_find_extension_symbol(node, context);
	if (!mem_acc_symbol) {
		return false;
	}

	if (!(mem_acc_symbol->function.parameter_lists.head == nullptr)) {
		print_error(ERR_SEM_PARAMS, node->line_num, "Member access function can not have params!");
		return false;
	}

	if (mem_acc_symbol->function.state == FUNCTION_UNVISITED) {
		if (!sem_second_analyze_function(mem_acc_symbol->function.function_definition, context,
										 mem_acc_symbol)) {
			return false;
		}
	} else if (mem_acc_symbol->function.state == FUNCTION_VISITING) {
		if (!call_chain_check_cycle(context, mem_acc_symbol->function.function_definition)) {
			return false;
		}
	}

	node->eval_type = mem_acc_symbol->function.return_type;

	return true;
}

// Tool: Claude Code (Claude Sonnet 5.5)
// Interpolated string implemented with AI assistance according to the design of the author.
// Prompt: Write sem_second_node_string_interp. Analyze all the parts, any type is allowed because
// the values are converted like toStr, the result is String.
bool sem_second_node_string_interp(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_string_interp on nullptr.");

	for (ast_node_t part = node->as.string_interp.parts.head; part; part = part->next) {
		if (!sem_second_expr(part, context)) {
			return false;
		}
	}

	node->eval_type = TYPE_STRING;
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

		// Check target eval type if not checked yet
		if (!node->as.member_access.target->eval_type) {
			if (!sem_second_expr(node->as.member_access.target, context)) {
				throw_warning(node->line_num, "Failed to process member access target.");
				return false;
			}
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

		// Check target eval type if not checked yet
		if (!node->as.func_call.callee->as.member_access.target->eval_type) {
			if (!sem_second_expr(node->as.func_call.callee->as.member_access.target, context)) {
				throw_warning(node->line_num,
							  "Failed to process member access target in function call.");
				return false;
			}
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
			print_error(ERR_SEM_OTHER, node->line_num,
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
								"Unexpected type in function parameter.");
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
		char *function_name = node->as.id.name;

		ast_node_t identifier_node = ast_create_node(NODE_IDENTIFIER, node->line_num);

		if (!identifier_node) {
			print_error(ERR_INTERNAL, node->line_num, "AST node allocation failed!");
			return false;
		}
		identifier_node->as.id.name = function_name;

		node->type = NODE_FUNC_CALL;
		node->as.func_call.callee = identifier_node;
		node->as.func_call.has_parens = false;
		ast_list_init(&node->as.func_call.arg_lists);

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

bool sem_second_load_params(ast_node_t node, semantic_context_t *context,
							symbol_t function_symbol) {
	assert(node && context && function_symbol && "Called sem_second_load_params on nullptr.");

	// Tool: Claude Code (Claude Sonnet 5.5)
	// The receiver of an extension method is added to the local symtable, implemented with AI
	// assistance (the code is a shorter version of the removed branch of this function).
	// Prompt: Load the receiver of the extension method as an immutable variable.
	if (function_symbol->function.extension_parameter_name) {
		symbol_t receiver_symbol = stable_init_symbol();
		if (!receiver_symbol) {
			print_error(ERR_INTERNAL, node->line_num, "Failed to create receiver symbol.");
			return false;
		}

		receiver_symbol->symbol_variant = SYMBOL_VARIABLE;
		receiver_symbol->variable.type = function_symbol->function.extension_parameter_type;
		receiver_symbol->variable.is_mutable = false;
		receiver_symbol->variable.unique_name =
			strdup(function_symbol->function.extension_parameter_name);
		if (!receiver_symbol->variable.unique_name) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to allocate memory for the receiver unique name.");
			stable_destroy_symbol(receiver_symbol);
			return false;
		}

		if (!stable_write(context->symtable_local,
						  function_symbol->function.extension_parameter_name, receiver_symbol)) {
			print_error(ERR_INTERNAL, node->line_num,
						"Failed to store the receiver in the local symbol table.");
			stable_destroy_symbol(receiver_symbol);
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

	return true;
}

bool call_chain_add(semantic_context_t *context, ast_node_t function_node) {
	call_chain_item_t new_item = malloc(sizeof(struct call_chain_item));

	if (!new_item) {
		return false;
	}

	new_item->function_node = function_node;
	new_item->previous_item = context->call_chain.top;

	context->call_chain.top = new_item;
	context->call_chain.size++;

	return true;
}

void call_chain_init(semantic_context_t *context) {
	context->call_chain.size = 0;
	context->call_chain.top = nullptr;
}

void call_chain_destroy(semantic_context_t *context) {
	call_chain_item_t current_item = context->call_chain.top;

	while (current_item) {
		call_chain_item_t temp_item = current_item;
		current_item = current_item->previous_item;
		free(temp_item);
		context->call_chain.size--;
	}

	context->call_chain.top = nullptr;
}

void call_chain_pop(semantic_context_t *context) {
	assert(context->call_chain.top != nullptr);
	call_chain_item_t temp_item = context->call_chain.top;
	context->call_chain.top = context->call_chain.top->previous_item;
	free(temp_item);
	context->call_chain.size--;
}

bool call_chain_check_cycle(semantic_context_t *context, ast_node_t cycle_start) {
	call_chain_item_t current_item = context->call_chain.top;

	while (current_item) {
		if (current_item->function_node->as.func_def.return_type == NULL) {
			print_error(ERR_SEM_TYPE_COMPAT, cycle_start->line_num,
						"Recursion without explicit function type!");
			return false;
		}
		if (current_item->function_node == cycle_start) {
			return true;
		}
		current_item = current_item->previous_item;
	}

	print_error(ERR_INTERNAL, cycle_start->line_num,
				"Function is being analyzed but is missing from the call chain!");
	return false;
}
