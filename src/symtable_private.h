/**
 * @file symtable_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Private internal declarations for symtable data type
 */

#ifndef SYMTABLE_PRIVATE_H
#define SYMTABLE_PRIVATE_H

#include "symtable.h"

/**
 * @brief Frees specified symbol pointed to by void pointer
 *
 * helper function
 *
 * @param sym Symbol to free
 */
void symbol_dtor(void *sym);

#endif
