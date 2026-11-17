/**
 * @file symtable.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Public interface for symtable data type
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"
#include <stddef.h>

struct stable; // Ignore (for linking)

/**
 * @brief Type that is used to manipulate with symtable
 */
typedef struct stable *stable_t;

/**
 * @brief Determines whether symbol is a function or a variable
 */
typedef enum {
	SYMBOL_UNSET,	 /**< Symbol variant is not set */
	SYMBOL_FUNCTION, /**< Symbol is a function */
	SYMBOL_VARIABLE, /**< Symbol is a variable */
} symbol_variant_t;

typedef enum {
	FUNCTION_UNVISITED,
	FUNCTION_VISITING,
	FUNCTION_VISITED,
} function_state_t;

/**
 * @brief Structure that represents a symbol in the symbol table
 */
struct symbol {
	symbol_variant_t symbol_variant;
	union {
		struct {
			data_type_t type;  /**< Data type of the variable */
			bool is_mutable;   /**< Indicates if the variable is mutable (var) or immutable (val) */
			char *unique_name; /**< Unique name for the variable (for code generation) */
		} variable;
		struct {
			function_state_t state; /**< State of the function visit */

			bool is_main;	 /**< Main function flag */
			bool is_builtin; /**< Builtin function flag (needs to be have param list dealocated) */
			bool has_explicit_return_type; /**< Indicates if the function has an explicit return
											  type */

			data_type_t return_type;	/**< Return type of the function */
			ast_list_t parameter_lists; /**< List of parameter lists (NODE_PARAM_LIST) for the
										   function (nullptr if no parameter lists) */

			data_type_t
				extension_parameter_type;	/**< Type of the variable used in extension function */
			char *extension_parameter_name; /**< Name of the variable used in extension function
										   (nullptr if not extension) */
		} function;
	};
};

typedef struct symbol *symbol_t;

/**
 * @brief Allocates empty symbol
 *
 * @return symbol_t Allocated symbol set to 0 (nulltptr if failed)
 */
symbol_t stable_init_symbol();

/**
 * @brief Frees symbol
 *
 * @param sym Symbolt to free
 */
void stable_destroy_symbol(symbol_t sym);

/**
 * @brief Initializes symbol table stack with first empty scope level prepared
 *
 * @return stable_t Initialized table of symbols (nullptr if failed)
 */
stable_t stable_init();

/**
 * @brief Creates a deeper scope level in symbol table
 *
 * @param stab Symbol table to create the scope level in
 * @return true
 * @return false
 */
bool stable_enter_scope(stable_t stab);

/**
 * @brief Pops the current scope level
 *
 * @param stab Symbol table to end scope level
 */
void stable_leave_scope(stable_t stab);

/**
 * @brief Stores symbol under its key in the currect scope
 *
 * @param stab Symbol table to store it there
 * @param key Key under which the symbol should be stored
 * @param entry Symbol to store
 * @return true Success
 * @return false Failed
 */
bool stable_write(stable_t stab, const char *key, symbol_t entry);

/**
 * @brief Removes symbol under specified key in the currect scope
 *
 * @param stab Symbol table to remove it from
 * @param key Key of the symbol that should be removed
 * @return true Removed
 * @return false Not found
 */
bool stable_remove_symbol_curlevel(stable_t stab, const char *key);

/**
 * @brief Gets you the first symbol entry under specified key from the scope stack
 *
 * @param stab Symbol table to search
 * @param key Key under which you want to search
 * @return symbol_t Pointer to your specified symbol (nullptr if not found)
 */
symbol_t stable_find_alllevel(stable_t stab, const char *key);

/**
 * @brief Gets you a symbol entry under specified key (fails if not in current scope)
 *
 * @param stab Symbol table to search
 * @param key Key under which you want to search
 * @return symbol_t Pointer to your specified symbol (nullptr if not found)
 */
symbol_t stable_find_curlevel(stable_t stab, const char *key);

/**
 * @brief Destructs specified symbol table
 *
 * @param stab Symbol table to destruct
 * @return stable_t Always nullptr (can be used to overide stable_t variable
 * after destruction)
 */
stable_t stable_dtor(stable_t stab);

#endif
