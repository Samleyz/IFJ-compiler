/**
 * @file stack_private.h
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Private declarations for the linked-list stack ADT.
 */

#ifndef STACK_PRIVATE_H
#define STACK_PRIVATE_H

#include "stack.h"

typedef struct stack_item {
	void *data;
	struct stack_item *next;
} stack_item;

typedef struct stack {
	stack_item *top;
} stack;

#endif
