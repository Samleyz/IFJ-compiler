/**
 * @file stack.c
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief stack ADTs function implementations
 */

#include "error.h"
#include "stack_private.h"
#include <stdlib.h>

static stack_item_t s_item_ctor(void) {
	stack_item_t itm = malloc(sizeof(stack_item));
	if (!itm) {
		return NULL;
	}
	itm->data = NULL;
	itm->next = NULL;
	return itm;
}

static bool s_item_dtor(stack_item_t itm) {
	if (!itm) {
		return false;
	}
	free(itm);
	return true;
}

stack_t stack_init(void) {
	stack_t stck = malloc(sizeof(stack));
	if (!stck) {
		return NULL;
	}
	stck->top = NULL;
	return stck;
}

bool stack_is_empty(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, -1, "Called stack_is_empty on NULL");
	}
	if (stk->top == NULL) {
		return true;
	}
	return false;
}

bool stack_push(stack_t stk, void *data) {
	if (!stk) {
		throw_err(ERR_INTERNAL, -1, "Called stack_push on NULL");
	}

	stack_item_t item = s_item_ctor(); // initialising the stack item
	if (!item) {
		return false;
	}

	item->data = data;
	item->next = stk->top; // linking previous item
	stk->top = item;	   // setting current item as stack top
	return true;
}

void *stack_top(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, -1, "Called stack_top on NULL");
	}
	if (stack_is_empty(stk)) {
		throw_err(ERR_INTERNAL, -1, "Called stack_top on empty stack");
	}
	return stk->top->data;
}

bool stack_pop(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, -1, "Called stack_pop on NULL");
	}
	if (stack_is_empty(stk)) {
		return false;
	}

	stack_item_t top_temp = stk->top;
	stk->top = stk->top->next;	  // popping
	return s_item_dtor(top_temp); // cleaning
}

bool stack_dtor(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, -1, "Called stack_dtor on NULL");
	}
	while (!stack_is_empty(stk)) {
		stack_pop(stk);
	}
	free(stk);
	return true;
}