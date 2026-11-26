/**
 * @file semantic_analysis.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
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

	// Sttore symbol pointer for later access
	node->sym_entry = symbol;

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

	// Name of receiver (always unique in the scope of extension)
	if (!(node->as.extension.unique_receiver_name = strdup(node->as.extension.receiver_name))) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to duplicate receiver name.");
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
		// Sttore symbol pointer for later access
		current_extension_node->sym_entry = symbol;

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

bool sem_second(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second on nullptr.");

	// Node checks
	if (node->type != NODE_PROGRAM) {
		print_error(ERR_INTERNAL, node->line_num, "Second pass called on non program node.");
		return false;
	}

	// Go through items in the program
	ast_node_t current_node = node->as.program.items.head;
	while (current_node) {
		if (current_node->type == NODE_FUNC_DEF) {
			if (!sem_second_want_function(current_node, context)) {
				return false;
			}
		} else if (current_node->type == NODE_EXTENSION) {
			if (!sem_second_want_extension(current_node, context)) {
				return false;
			}
		} else {
			print_error(ERR_INTERNAL, current_node->line_num,
						"Second pass called on invalid node type.");
			return false;
		}

		// Move to next
		current_node = current_node->next;
	}

	// Success
	return true;
}

bool sem_second_want_extension(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_extension on nullptr.");

	// Node checks
	if (node->type != NODE_EXTENSION) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_extension on non-extension node.");
		return false;
	}

	// Go through extension method functions
	ast_node_t current_extension_node = node->as.extension.methods.head;
	while (current_extension_node) {
		// Node check
		if (current_extension_node->type != NODE_FUNC_DEF) {
			print_error(ERR_INTERNAL, current_extension_node->line_num,
						"Extension method is not a function definition.");
			return false;
		}

		// Want to pass the funstion in extension
		if (!sem_second_want_function(current_extension_node, context)) {
			return false;
		}
		current_extension_node = current_extension_node->next;
	}
	return true;
}

bool sem_second_want_function(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_want_function on nullptr.");

	// Node checks
	if (node->type != NODE_FUNC_DEF) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_want_function on non function definition node.");
		return false;
	}
	if (!node->as.func_def.body) {
		print_error(ERR_INTERNAL, node->line_num, "Function definition has no body.");
		return false;
	}
	if (!node->sym_entry) {
		print_error(ERR_INTERNAL, node->line_num,
					"Function definition has no symbol entry for second pass.");
		return false;
	}

	// Check state
	if (((symbol_t)node->sym_entry)->function.state == FUNCTION_VISITED) {
		// Skip already passed
		return true;
	} else if (((symbol_t)node->sym_entry)->function.state == FUNCTION_UNVISITED) {
		// Should be passed
		return sem_second_node_func_def(node, context);
	} else if (((symbol_t)node->sym_entry)->function.state == FUNCTION_VISITING) {
		// Recursive call detected
		// Go through function of the call stack
		function_stack_t *call_stack = context->function_stack;
		while (call_stack) {
			// Check each for explicit return type
			if (!call_stack->symbol->function.has_explicit_return_type) {
				print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
							"Recursive function does not have explicit return type, which is "
							"required for recursive functions.");
				return false;
			}

			// Stop at the recursion function
			if (call_stack->symbol == node->sym_entry) {
				break;
			}

			// Move to next
			call_stack = call_stack->caller_function_stack;
		}
		return true;
	}

	// Invalid state
	print_error(ERR_INTERNAL, node->line_num,
				"Function definition has invalid state for second pass.");
	return false;
}

bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_def on nullptr.");

	// Node checks
	if (node->type != NODE_FUNC_DEF) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_node_func_def on non function definition node.");
		return false;
	}
	if (!node->as.func_def.body) {
		print_error(ERR_INTERNAL, node->line_num, "Function definition has no body.");
		return false;
	}
	if (!node->sym_entry) {
		print_error(ERR_INTERNAL, node->line_num,
					"Function definition has no symbol entry for second pass.");
		return false;
	}

	// Create level on function call stack for it
	function_stack_t function_stack_level = {0};
	function_stack_level.symbol = node->sym_entry;
	function_stack_level.definition_node = node;
	function_stack_level.caller_function_stack = context->function_stack;
	function_stack_level.local_scope = stable_init(); // Create local scope for the function
	if (!function_stack_level.local_scope) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to create local scope for function definition.");
		return false;
	}
	// Set to context (push)
	context->function_stack = &function_stack_level;
	// Set function state
	((symbol_t)node->sym_entry)->function.state = FUNCTION_VISITING;

	// Check function body
	bool status = sem_second_node_func_def_body(node, context);

	// Remove from context (pop)
	stable_dtor(function_stack_level.local_scope);
	context->function_stack = function_stack_level.caller_function_stack;

	// Set function state
	((symbol_t)node->sym_entry)->function.state = FUNCTION_VISITED;

	return status;
}

bool sem_second_node_func_def_body(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_node_func_def_body on nullptr.");

	// Node checks
	if (!node->as.func_def.body) {
		print_error(ERR_INTERNAL, node->line_num, "Function definition has no body.");
		return false;
	}

	// Put parameters to local scope
	if (!sem_second_put_params_to_local_scope(node, context)) {
		return false;
	}

	// Ensure function has body
	if (!sem_second_ensure_enclosed_body(node, context)) {
		return false;
	}

	// Check body types
	bool need_value = !( // Explicit unit function throw away return value
		((symbol_t)node->sym_entry)->function.has_explicit_return_type && // Explicit return type
		((symbol_t)node->sym_entry)->function.return_type == TYPE_UNIT);  // That is Unit
	if (!sem_second_scoped_resolve(node->as.func_def.body, context, need_value)) {
		return false;
	}

	// Resolve return type
	if (!((symbol_t)node->sym_entry)->function.has_explicit_return_type) {
		if (node->as.func_def.body->eval_type == TYPE_UNKNOWN) {
			print_error(ERR_INTERNAL, node->line_num, "Invalid return type for function.");
			return false;
		}
		// Store implicit function return type
		((symbol_t)node->sym_entry)->function.return_type = node->as.func_def.body->eval_type;
	} else if (need_value) {
		// Check return type compatibility
		if (!sem_second_return_type_compatibility(
				node->as.func_def.body, ((symbol_t)node->sym_entry)->function.return_type)) {
			return false;
		}
	}

	// Check implicit main return type to be unit
	if (((symbol_t)node->sym_entry)->function.is_main &&
		!((symbol_t)node->sym_entry)->function.has_explicit_return_type) {
		if (node->as.func_def.body->eval_type != TYPE_UNIT) {
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
						"Main function must have return type Unit.");
			return false;
		}
	}

	// Insert returns
	if (!sem_second_insert_returns(node->as.func_def.body, context)) {
		return false;
	}

	// Success
	return true;
}

bool sem_second_put_params_to_local_scope(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_put_params_to_local_scope on nullptr.");

	// Put receiver parameter if extension function
	if (((symbol_t)node->sym_entry)->function.extension_parameter_name) {
		if (!declare_variable(
				context, ((symbol_t)node->sym_entry)->function.extension_parameter_name,
				((symbol_t)node->sym_entry)->function.extension_parameter_type, false, false)) {
			return false;
		}
	}

	// Put parameters to local scope
	// Go through parameter lists
	ast_node_t current_param_list_node = node->as.func_def.param_lists.head;
	while (current_param_list_node) {
		// Node checks
		// TODO

		// Go through parameters in the list
		ast_node_t current_param_node = current_param_list_node->as.param_list.params.head;
		while (current_param_node) {
			// Node checks
			// TODO

			// Declare the parameter
			symbol_t param_symbol = declare_variable(context, current_param_node->as.param.name,
													 current_param_node->eval_type, false, false);
			if (!param_symbol) {
				return false;
			}

			// Store unique name
			if (!(current_param_node->as.param.unique_name =
					  strdup(param_symbol->variable.unique_name))) {
				print_error(ERR_INTERNAL, current_param_node->line_num,
							"Failed to allocate memory for parameter unique name.");
				return false;
			}

			// Move to next parameter
			current_param_node = current_param_node->next;
		}

		// Move to next parameter list
		current_param_list_node = current_param_list_node->next;
	}

	return true;
}

symbol_t declare_variable(semantic_context_t *context, const char *name, data_type_t type,
						  bool is_mutable, bool should_be_unique) {
	assert(context && name && type && "Called declare_variable with invalid parameters.");

	// Check if not defined in the current scope
	if (stable_find_curlevel(context->function_stack->local_scope, name)) {
		print_error(ERR_SEM_REDEFINITION, 0,
					"Variable '%s' is already defined in the current scope.", name);
		return nullptr;
	}

	// Create symbol
	symbol_t symbol = stable_init_symbol();
	if (!symbol) {
		print_error(ERR_INTERNAL, 0, "Failed to create variable symbol.");
		return nullptr;
	}

	// Fill out symbol
	symbol->variable.unique_name = make_name(context, name, should_be_unique);
	if (!symbol->variable.unique_name) {
		print_error(ERR_INTERNAL, 0, "Failed to generate unique name for variable '%s'.", name);
		stable_destroy_symbol(symbol);
		return nullptr;
	}
	symbol->symbol_variant = SYMBOL_VARIABLE;
	symbol->variable.type = type;
	symbol->variable.is_mutable = is_mutable;

	// Write symbol to the symbol table
	if (!stable_write(context->function_stack->local_scope, name, symbol)) {
		print_error(ERR_INTERNAL, 0, "Failed to write variable '%s' to symbol table.", name);
		stable_destroy_symbol(symbol);
		return nullptr;
	}

	return symbol;
}

char *make_name(semantic_context_t *context, const char *name, bool should_be_unique) {
	assert(context && name && "Called make_name with invalid parameters.");

	if (should_be_unique) {
		// Generate unique name with counter
		char *unique_name =
			malloc(strlen(name) + SIZE_T_MAX_DIGITS + 1 + 1); // Enough for name + counter
		if (!unique_name) {
			print_error(ERR_INTERNAL, 0, "Failed to allocate memory for unique name.");
			return nullptr;
		}
		sprintf(unique_name, "%s$%zu", name, context->rename_counter++);
		return unique_name;
	} else {
		// Return a copy of the name
		char *unique_name = strdup(name);
		if (!unique_name) {
			print_error(ERR_INTERNAL, 0, "Failed to allocate memory for unique name.");
			return nullptr;
		}
		return unique_name;
	}
}

bool sem_second_ensure_enclosed_body(ast_node_t node, semantic_context_t *context) {
	assert(node && context && "Called sem_second_ensure_enclosed_body on nullptr.");

	// Already in block
	if (node->as.func_def.body->type == NODE_BLOCK) {
		return true;
	}

	// Create new block and statement nodes to enclose the body in
	ast_node_t new_block = ast_create_node(NODE_BLOCK, node->as.func_def.body->line_num);
	ast_node_t new_stmt = ast_create_node(NODE_EXPR_STMT, node->as.func_def.body->line_num);
	if (!new_block || !new_stmt) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to create new block or statement node.");
		ast_dtor(new_block);
		ast_dtor(new_stmt);
		return false;
	}

	// Enclose
	ast_list_init(&(new_block->as.block.statements));
	if (!ast_list_append(&(new_block->as.block.statements), new_stmt)) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to append new statement to new block.");
		ast_dtor(new_block);
		ast_dtor(new_stmt);
		return false;
	}
	new_stmt->as.expr_stmt.expr = node->as.func_def.body;
	node->as.func_def.body = new_block;
	return true;
}

bool sem_second_scoped_resolve(ast_node_t node, semantic_context_t *context, bool need_value) {
	assert(node && context && "Called sem_second_scoped_resolve on nullptr.");

	// Enter into scope of the block
	if (!stable_enter_scope(context->function_stack->local_scope)) {
		print_error(ERR_INTERNAL, node->line_num, "Failed to enter scope of the block.");
		return false;
	}

	// Resolve the block
	if (!sem_second_statement(node, context, need_value)) {
		return false;
	}

	// Leave the scope of the block
	stable_leave_scope(context->function_stack->local_scope);

	// Success
	return true;
}

bool sem_second_statement(ast_node_t node, semantic_context_t *context, bool need_value) {
	assert(node && context && "Called sem_second_statement on nullptr.");

	// Decide based on the type of statement
	switch (node->type) {
	case NODE_BLOCK:
		if (!sem_second_block(node, context, need_value)) {
			return false;
		}
		return true;
		break;
	case NODE_VAR_DEF:
		if (!sem_second_var_def(node, context)) {
			return false;
		}
		node->eval_type = TYPE_UNIT;
		return true;
		break;
	case NODE_ASSIGN:
		if (!sem_second_assign(node, context)) {
			return false;
		}
		node->eval_type = TYPE_UNIT;
		return true;
		break;
	case NODE_IF:
		if (!sem_second_if(node, context, need_value)) {
			return false;
		}
		return true;
		break;
	case NODE_WHILE:
		if (!sem_second_while(node, context)) {
			return false;
		}
		node->eval_type = TYPE_UNIT;
		return true;
		break;
	case NODE_FOR:
		if (!sem_second_for(node, context)) {
			return false;
		}
		node->eval_type = TYPE_UNIT;
		return true;
		break;
	case NODE_RETURN:
		if (!sem_second_return(node, context)) {
			return false;
		}
		return true;
		break;
	case NODE_EXPR_STMT:
		// Patch fix for if (whether it is statement or expression should decide semantics)
		if (node->as.expr_stmt.expr->type == NODE_IF) {
			return sem_second_if(node->as.expr_stmt.expr, context, need_value);
		}

		if (!sem_second_expression(node->as.expr_stmt.expr, context)) {
			return false;
		}
		node->eval_type = node->as.expr_stmt.expr->eval_type;
		return true;
		break;
	default:
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_statement on invalid statement node type.");
		return false;
	}
	return true;
}

bool sem_second_block(ast_node_t node, semantic_context_t *context, bool need_value) {
	assert(node && context && "Called sem_second_block on nullptr.");

	// Node checks
	if (node->type != NODE_BLOCK) {
		print_error(ERR_INTERNAL, node->line_num, "Called sem_second_block on non block node.");
		return false;
	}

	// Go through statements in the block
	ast_node_t current_stmt_node = node->as.block.statements.head;
	node->eval_type = TYPE_UNIT;
	while (current_stmt_node) {
		// Check if last statement that needs value for block
		bool need_value_really = false;
		if (current_stmt_node->next == nullptr) { // Last statement in the block
			need_value_really = need_value;
		}

		// Pass the statement
		if (!sem_second_statement(current_stmt_node, context, need_value_really)) {
			return false;
		}

		// Store evaluated type
		node->eval_type = current_stmt_node->eval_type;

		// Move to next
		current_stmt_node = current_stmt_node->next;
	}
	return true;
}

bool sem_second_var_def(ast_node_t node, semantic_context_t *context) {

	// Node checks
	if (node->type != NODE_VAR_DEF) {
		print_error(ERR_INTERNAL, node->line_num,
					"Called sem_second_var_def on non variable definition node.");
		return false;
	}

	// Resolve definition expression
	if (!sem_second_expression(node->as.var_def.init_expr, context)) {
		return false;
	}

	// Resolve type compatibility
	data_type_t init_expr_type = node->as.var_def.init_expr->eval_type;
	if (node->as.var_def.type_name) {
		if (!get_data_type_from_string(node->as.var_def.type_name)) {
			print_error(ERR_SEM_UNDEFINED, node->line_num,
						"Variable definition has unknown type '%s'.", node->as.var_def.type_name);
			return false;
		}

		// Explicit type - check compatibility
		if (!sem_ensure_compatible_type(node->as.var_def.init_expr,
										get_data_type_from_string(node->as.var_def.type_name),
										ERR_SEM_TYPE_COMPAT)) {
			return false;
		}
		init_expr_type = get_data_type_from_string(node->as.var_def.type_name);
	} else {
		// Implicit type - check not invalid
		if (node->as.var_def.init_expr->eval_type == TYPE_UNKNOWN) {
			print_error(ERR_SEM_TYPE_COMPAT, node->line_num,
						"Variable definition has unknown type and no explicit type.");
			return false;
		}
	}

	// Declare variable in the local scope
	symbol_t symbol = declare_variable(context, node->as.var_def.name, init_expr_type,
									   !node->as.var_def.is_val, true);
	if (!symbol) {
		return false;
	}

	// Store unique name
	if (!(node->as.var_def.unique_name = strdup(symbol->variable.unique_name))) {
		print_error(ERR_INTERNAL, node->line_num,
					"Failed to allocate memory for variable unique name.");
		return false;
	}

	// Success
	return true;
}

/*
Tool: Claude Sonnet 5.5
Prompt paraphrase: Najdi v tom chyby a oprav to
use same style, write clean code, dont break anything, explain what you did
+ other files as context
*/
bool sem_ensure_compatible_type(ast_node_t node, data_type_t expected_type,
								err_c_t potential_error_code) {
	assert(node && expected_type && "Called sem_ensure_compatible_type on nullptr.");

	// Check compatibility
	if (!(node->eval_type == expected_type || expected_type == TYPE_ANY ||
		  (expected_type == TYPE_DOUBLE && node->eval_type == TYPE_INT))) {
		print_error(potential_error_code, node->line_num, "Type mismatch");
		return false;
	}

	// Implicit conversion if needed
	if (expected_type == TYPE_DOUBLE && node->eval_type == TYPE_INT) {
		/*
		Tool: Claude Sonnet 5.5
		Prompt paraphrase: Please write for me the wrapping logic of node into cast node
		use same style, write clean code, dont break anything, explain what you did
		+ other files as context
		*/
		ast_node_t wrapper_node = sem_wrap_in_place(node, NODE_TYPE_CAST);
		if (!wrapper_node) {
			print_error(ERR_INTERNAL, node->line_num, "Failed to wrap node in cast node.");
			return false;
		}
		node->as.type_cast.target_type = TYPE_DOUBLE;
		node->as.type_cast.operand = wrapper_node;
		node->eval_type = TYPE_DOUBLE;
	}

	// Success
	return true;
}

/*
Tool: Claude Sonnet 5.5
Prompt paraphrase: Please write for me the wrapping logic of node into cast node
use same style, write clean code, dont break anything, explain what you did
+ other files as context
*/
ast_node_t sem_wrap_in_place(ast_node_t node, node_type_t wrapper) {
	assert(node && "Called sem_wrap_in_place on nullptr.");

	// Create node
	ast_node_t inner = ast_create_node(node->type, node->line_num);
	if (!inner) {
		return nullptr;
	}

	// Copy and connect nodes
	*inner = *node;
	inner->next = nullptr;
	ast_node_t next = node->next;
	*node = (struct ast_node){.type = wrapper, .line_num = inner->line_num, .next = next};
	return inner;
}

bool sem_second_assign(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_if(ast_node_t node, semantic_context_t *context, bool need_value) {
	// TODO
	return true;
}

bool sem_second_while(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_for(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_return(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_expression(ast_node_t node, semantic_context_t *context) {
	// TODO
	return true;
}

bool sem_second_return_type_compatibility(ast_node_t body, data_type_t expected_type) {
	assert(body && expected_type && "Called sem_second_return_type_compatibility on nullptr.");
	// TODO
	return true;
}

bool sem_second_insert_returns(ast_node_t body, semantic_context_t *context) {
	assert(body && context && "Called sem_second_insert_returns on nullptr.");
	// TODO
	return true;
}
