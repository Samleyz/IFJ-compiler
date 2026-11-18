/**
 * @file semantic_analysis_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Header file for internal semantic analysis functions and data structures
 */

#ifndef SEMANTIC_ANALYSIS_PRIVATE_H
#define SEMANTIC_ANALYSIS_PRIVATE_H

#include "ast.h"
#include "semantic_analysis.h"
#include "symtable.h"

typedef struct {
	stable_t symtable_local;			   /**< Table of local symbols */
	stable_t symtable_functions;		   /**< Table of function symbols */
	stable_t symtable_extension_functions; /**< Table of extension function symbols */

	size_t main_functions_count; /**< Count of main functions found */
	size_t rename_counter;		 /**< Counter for generating unique names*/

	char *current_function_name; /**< Name of the function currently being analyzed (nullptr if not
									in function) */

	char *extension_variable_name; /**< Name of the variable used in extension function calls to be
									  stored in function symbol (nullptr if not in extension)*/

	bool found_branch_without_return; /**< Flag indicating if a branch without return was found */
	bool is_in_expression_if; /**< Flag indicating if currently analyzing an if as expression */

	// TODO: Add more fields if needed for semantic context
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
 * @brief Checks the semantic correctness of a statement
 *
 * @param node Statement node to analyze
 * @param context Semantic analysis context
 * @return true	Success
 * @return false Semantic error
 */
bool sem_second_stmt(ast_node_t node, semantic_context_t *context);

/**
 * @brief Checks the semantic correctness of an expression
 *
 * @param node Expression node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_expr(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_PROGRAM
 *
 * @param node Program node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_program(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_BLOCK
 *
 * @param node Block node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_block(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_EXTENSION
 *
 * @param node Extension node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_extension(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_FUNC_DEF
 *
 * @param node Function definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_func_def(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_VAR_DEF
 *
 * @param node Variable definition node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_var_def(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_ASSIGN
 *
 * @param node Assignment node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_assign(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_IF (statement)
 *
 * @param node If statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_if_stmt(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_IF (expression)
 *
 * @param node If expression node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_if_expr(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_WHILE
 *
 * @param node While statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_while(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_FOR
 *
 * @param node For statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_for(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_RETURN
 *
 * @param node Return statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_return(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_EXPR_STMT
 *
 * @param node Expression statement node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_expr_stmt(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_BINARY_OP
 *
 * @param node Binary operation node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_binary_op(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_UNARY_OP
 *
 * @param node Unary operation node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_unary_op(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_FUNC_CALL
 *
 * @param node Function call node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_func_call(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_MEMBER_ACCESS
 *
 * @param node Member access node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_member_access(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_STRING_INTERP
 *
 * @param node String interpolation node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_string_interp(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_TYPE_CAST
 *
 * @param node Type cast node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_type_cast(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_PARAM_LIST
 *
 * @param node Node with a parameter list to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_param_list(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_PARAM
 *
 * @param node Parameter node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_param(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_IDENTIFIER
 *
 * @param node Identifier node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_identifier(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_LIT_INT
 *
 * @param node Integer literal node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_lit_int(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_LIT_DOUBLE
 *
 * @param node Double literal node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_lit_double(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_LIT_STRING
 *
 * @param node String literal node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_lit_string(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_LIT_BOOL
 *
 * @param node Boolean literal node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_lit_bool(ast_node_t node, semantic_context_t *context);

/**
 * @brief Performs the second pass of semantic analysis on a NODE_LIT_UNIT
 *
 * @param node Unit literal node to analyze
 * @param context Semantic analysis context
 * @return true Success
 * @return false Semantic error
 */
bool sem_second_node_lit_unit(ast_node_t node, semantic_context_t *context);

#endif
