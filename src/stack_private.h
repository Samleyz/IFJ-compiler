/**
 * @file stack_private.h
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Private declarations of ADT linked list stack, and helper functions for LL items
 */

#ifndef STACK_PRIVATE_H
#define STACK_PRIVATE_H

#include "stack.h"
#include <stdlib.h>

/**
 * @brief Linked list item for stack
 */
typedef struct stack_item {
	void *data;				 // pointer to random data
	struct stack_item *next; // pointer to next item
} stack_item;

/**
 * @brief Linked list stack structure
 */
typedef struct stack {
	stack_item *top;			  // Item stored on top of the stack, NULL if empty
	void (*dstr_data_cb)(void *); // callback dstr function for data stored in items
} stack;

/**
 * @brief  Helper function: Initialises the stack item
 * @return pointer to initialised stack item
 * @return nullptr init failed
 */
static stack_item_t s_item_ctor(void);

/**
 * @brief Helper function: Frees the item struct
 * @param itm Target item
 * @param dstr_data_cb callback function of stack called on each items value
 * @return true Succesfully freed
 */
static bool s_item_dtor(stack_item_t itm, void (*dstr_data_cb)(void *));

#endif