/**
 * @file semantic_analysis_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Header file for internal semantic analysis functions and data structures
 */

#ifndef SEMANTIC_ANALYSIS_PRIVATE_H
#define SEMANTIC_ANALYSIS_PRIVATE_H

#include "ast.h"
#include "semantic_analysis.h"
#include "symtable.h"

/** @brief Digits of size_t, space for the counter in the unique names of variables */
#define SIZE_T_MAX_DIGITS 20

/**
 * @brief Stack level struct that is used for keeping track of function call stack, that is
 * currently analyzed in second pass
 */
typedef struct function_stack {
	symbol_t symbol;			/**< Symbol of the analysed function */
	ast_node_t definition_node; /**< Node NODE_FUNC_DEF of the analysed function */
	stable_t local_scope;		/**< Current local sccope symbol table */
	struct function_stack
		*caller_function_stack; /**< Stack of the function that was calling this function */
} function_stack_t;

/**
 * @brief Semantic context struct that holds all the necessary information for the semantic analysis
 * so functions can share that between each other
 */
typedef struct {
	stable_t symtable_local;			   /**< Table of local symbols */
	stable_t symtable_functions;		   /**< Table of function symbols */
	stable_t symtable_extension_functions; /**< Table of extension function symbols */

	size_t main_functions_count; /**< Count of main functions found */
	size_t rename_counter;		 /**< Counter for generating unique names*/

	function_stack_t
		*function_stack; /**< Function currently analysed in the second pass (nullptr if none) */
} semantic_context_t;

/**
 * @brief Initializes a semantic context structure
 *
 * @param context Pointer to the semantic context structure to initialize (in situ)
 * @return true Success
 * @return false Failed to initialize
 */
bool semantic_context_init(semantic_context_t *context);

/**
 * @brief Destroys a semantic context structure and frees it's resources
 *
 * @param context Pointer to the semantic context structure to destroy
 */
void semantic_context_dtor(semantic_context_t *context);

/**
 * @brief Prefills the symbol table with builtin functions
 *
 * @param symtable Symbol table to prefill
 * @return true Success
 * @return false Failed to prefill
 */
bool prefill_builtin_global_functions(stable_t stab);

/**
 * @brief Prefills the symbol table with builtin extension functions
 *
 * @param stab Symbol table to prefill
 * @return true Success
 * @return false Failed to prefill
 */
bool prefill_builtin_extension_functions(stable_t stab);

/**
 * @brief Registers a builtin function in the semantic context
 *
 * @param stab Symbol table to register the function in
 * @param name Name of the function (string literal)
 * @param return_type Return type of the function
 * @param param_types Array of parameter types as data_type_t (nullptr if no parameter list)
 * @param param_count How long is param_types array
 * @return true Success
 * @return false Failed to register
 */
bool register_builtin_function(stable_t stab, const char *name, data_type_t return_type,
							   data_type_t *param_types, size_t param_count);

/**
 * @brief Registers a builtin extension function in the semantic context
 *
 * @param stab Symbol table to register the function in
 * @param name Name of the function (string literal)
 * @param return_type Return type of the function
 * @param param_types Array of parameter types as data_type_t (nullptr if no parameter list)
 * @param param_count How long is param_types array
 * @param receiver_type Type of the value before the dot
 * @return true Success
 * @return false Failed to register
 */
bool register_builtin_extension_function(stable_t stab, const char *name, data_type_t return_type,
										 data_type_t *param_types, size_t param_count,
										 data_type_t receiver_type);

/**
 * @brief Create a parameter list for builtin function
 *
 * @param param_lists List of parameter lists to append the new parameter list to (in situ)
 * @param param_types Array of parameter types as data_type_t
 * @param param_count How long is param_types array
 * @return true Success
 * @return false Failed to create
 */
bool create_parameter_list_for_builtin_function(ast_list_t *param_lists, data_type_t *param_types,
												size_t param_count);

/**
 * @brief Array that can be used to convert data_type_t to strings (use strdup to copy the string)
 * Quick fix
 */
static const char *const data_type_string_forms[] = {
	[TYPE_UNKNOWN] = nullptr, [TYPE_INT] = "Int",	   [TYPE_DOUBLE] = "Double",
	[TYPE_STRING] = "String", [TYPE_BOOL] = "Boolean", [TYPE_UNIT] = "Unit",
	[TYPE_ANY] = "Any",
};

/**
 * @brief Get the data type from string
 * Quick fix
 *
 * @param type_str String representing the data type
 * @return data_type_t Corresponding data type enum value (UNKNOWN if not recognized)
 */
data_type_t get_data_type_from_string(const char *type_str);

/*
####################################################################################################
# First pass functions
####################################################################################################
*/

/**
 * @brief Performs the first pass of semantic analysis on node
 *
 * @param node Node of AST to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_first(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the first pass of semantic analysis on a node in program node
 *
 * @param node Node of AST to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_first_sub_program(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the first pass of semantic analysis on a function definition node
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_first_node_func_def(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the first pass of semantic analysis on an extension node
 *
 * @param node Extension node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_first_node_extension(ast_node_t node, semantic_context_t *context);

/**
 * @brief Checks the uniqueness of all parameter names
 *
 * @param param_lists List of parameter lists to check
 * @param receiver_name Name of the parameter before dot (nullptr if not in extension)
 * @param context Semantic analysis context
 * @return true Valid
 * @return false Error
 */
bool sem_first_validate_parameter_uniqueness(ast_list_t *param_lists, const char *receiver_name,
											 semantic_context_t *context);

/*
####################################################################################################
# Second pass functions
####################################################################################################
*/

/**
 * @brief Performs the second pass of semantic analysis on program root node
 *
 * @param node Root program node of AST to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second(ast_node_t node, semantic_context_t *context);

/**
 * @brief Ensures that extension functions are passed in second pass
 *
 * @param node Extension function node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_want_extension(ast_node_t node, semantic_context_t *context);

/**
 * @brief Ensures that function are passed in second pass
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_want_function(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a function definition node
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a function definition body
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_node_func_def_body(ast_node_t node, semantic_context_t *context);

/**
 * @brief Puts the parameters of a function definition to its local scope
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_put_params_to_local_scope(ast_node_t node, semantic_context_t *context);

/**
 * @brief Ensures that a function definition body is enclosed in a block
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_ensure_enclosed_body(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types with separate scope according to if return value is expected
 *
 * @param node Node to analyze
 * @param context Semantic analysis context
 * @param need_value Whether the body is expected to return a value
 * @return true Success
 * @return false Error
 */
bool sem_second_scoped_resolve(ast_node_t node, semantic_context_t *context, bool need_value);

/**
 * @brief Checks compatibility of a body return type with an expected type (inserts casts if needed)
 *
 * @param body Body node to analyze
 * @param expected_type Expected return type
 * @return true Success
 * @return false Error
 */
bool sem_second_return_type_compatibility(ast_node_t body, data_type_t expected_type);

/**
 * @brief Inserts return statements in a body if needed (for code gen)
 *
 * @param body Body node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_insert_returns(ast_node_t body, semantic_context_t *context);

/**
 * @brief Declares a new variable in the current scope
 *
 * @param context Semantic analysis context
 * @param name Name of the variable
 * @param type Type of the variable
 * @param is_mutable Whether the variable is mutable
 * @param should_be_unique Whether the variable should be renamed
 * @return true Success
 * @return false Error
 */
symbol_t declare_variable(semantic_context_t *context, const char *name, data_type_t type,
						  bool is_mutable, bool should_be_unique);

/**
 * @brief Creates name for a variable (unique if needed)
 *
 * @param context Semantic analysis context
 * @param name Name of the variable
 * @param should_be_unique Whether the variable should be renamed
 * @return Pointer to the unique name, or nullptr on error
 */
char *make_name(semantic_context_t *context, const char *name, bool should_be_unique);

/**
 * @brief Resolves the types of a statement
 *
 * @param node Statement node to analyze
 * @param context Semantic analysis context
 * @param need_value Whether the statement is expected to return a value
 * @return true Success
 * @return false Error
 */
bool sem_second_statement(ast_node_t node, semantic_context_t *context, bool need_value);

/**
 * @brief Resolves the types of a block
 *
 * @param node Block node to analyze
 * @param context Semantic analysis context
 * @param need_value Whether the block is expected to return a value
 * @return true Success
 * @return false Error
 */
bool sem_second_block(ast_node_t node, semantic_context_t *context, bool need_value);

/**
 * @brief Resolves the types of a variable definition
 *
 * @param node Variable definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_var_def(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types of an assignment
 *
 * @param node Assignment node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_assign(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types of an if statement
 *
 * @param node If statement node to analyze
 * @param context Semantic analysis context
 * @param need_value Whether the if statement is expected to return a value
 * @return true Success
 * @return false Error
 */
bool sem_second_if(ast_node_t node, semantic_context_t *context, bool need_value);

/**
 * @brief Resolves the types of a while statement
 *
 * @param node While statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_while(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types of a for statement
 *
 * @param node For statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_for(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types of a return statement
 *
 * @param node Return statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_return(ast_node_t node, semantic_context_t *context);

/**
 * @brief Resolves the types of an expression
 *
 * @param node Expression node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Error
 */
bool sem_second_expression(ast_node_t node, semantic_context_t *context);

/**
 * @brief Ensures that a node's type is compatible with an expected type
 *
 * @param node Node to check
 * @param expected_type Expected type
 * @param potential_error_code Error code to use if the types are not compatible
 * @return true If the types are compatible
 * @return false If the types are not compatible
 */
bool sem_ensure_compatible_type(ast_node_t node, data_type_t expected_type,
								err_c_t potential_error_code);

/**
 * @brief Wraps a node in a other node
 *
 * @param node Node to wrap
 * @param wrapper Type of the wrapper node
 * @return The wrapped node, or nullptr on error
 */
ast_node_t sem_wrap_in_place(ast_node_t node, node_type_t wrapper);
