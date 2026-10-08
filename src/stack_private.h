/**
 * @file stack_private.h
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Private declarations of ADT linked list stack
 */

#ifndef STACK_PRIVATE_H
#define STACK_PRIVATE_H

#include "stack.h"

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
	stack_item *top; // Item stored on top of the stack, NULL if empty
} stack;

#endif