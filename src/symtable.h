/**
 * @file symtable.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Public interface for symtable data type
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

struct stable; // Ignore (for linking)

/**
 * @brief Type that is used to manipulate with symtable
 */
typedef struct stable *stable_t;

struct symbol {
	// TODO: what else will symbols need to store here
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
