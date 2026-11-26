/**
 * Implementation of a compiler for the imperative language IFJ26
 *
 * @file symtable_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Private internal declarations for symtable data type
 */

#ifndef SYMTABLE_PRIVATE_H
#define SYMTABLE_PRIVATE_H

#include "hashtable.h"
#include "symtable.h"

/**
 * @brief Internal symbol table structure (scope stack of hastables)
 */
struct stable_layer {
	struct stable_layer *next;
	htable_t table;
};

/**
 * @brief Internal symbol table structure (scope stack of hastables)
 */
struct stable {
	struct stable_layer *top;
};

#endif
