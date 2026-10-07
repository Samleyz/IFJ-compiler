/**
 * @file symtable.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Public interface for symtable data type
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

struct htable;					 // Ignore (for linking)
typedef struct htable *htable_t; // Ignore (for linking)

/**
 * @brief Type that is used to manipulate with symtable
 */
typedef htable_t stable_t;

typedef struct {
	char *key;
	// TODO: what else will symbols need to store here
} symbol_t;

/**
 * @brief Initializes empty symbol table
 *
 * @return stable_t Initialized symbol table (nullptr if failed)
 */
stable_t stable_init();

/**
 * @brief Returns how many records are currently in symbol table
 *
 * @param stab Symbol table to get the count from
 * @return size_t Record count
 */
size_t stable_record_count(stable_t stab);

/**
 * @brief Stores saymbol under its key
 *
 * @param stab Symbol table to store it there
 * @param entry Symbol to store
 * @return true Success
 * @return false Failed
 */
bool stable_write(stable_t stab, symbol_t entry);

/**
 * @brief Gets you a symbol entry under specified key
 *
 * @param stab Symbol table to search
 * @param key Key under which you want to search
 * @return symbol_t* Pointer to your specified symbol (nullptr if not found)
 */
symbol_t *stable_read(stable_t stab, const char *key);

/**
 * @brief Clears a symbol under specified key
 *
 * @param stab Symbol table where to clear it
 * @param key Cleared symbol key
 * @return true Symbol was cleared
 * @return false There already wasn't anything
 */
bool stable_remove(stable_t stab, const char *key);

/**
 * @brief Destructs specified symbol table
 *
 * @param stab Symbol table to destruct
 * @return stable_t Always nullptr (can be used to overide stable_t variable
 * after destruction)
 */
stable_t stable_dtor(stable_t stab);

#endif
