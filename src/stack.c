/**
 * Implementation of a compiler for the imperative language IFJ26
 *
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

static bool s_item_dtor(stack_item_t itm, void (*dstr_data_cb)(void *)) {
	if (!itm) {
		return false;
	}

	if (dstr_data_cb != NULL && itm->data != NULL) {
		dstr_data_cb(itm->data); // destorying the data stored in item using CallBack
	}

	free(itm);
	return true;
}

stack_t stack_init(void (*dstr_data_cb)(void *)) {
	stack_t stck = malloc(sizeof(stack));
	if (!stck) {
		return NULL;
	}
	stck->top = NULL;
	stck->dstr_data_cb = dstr_data_cb;
	return stck;
}

bool stack_is_empty(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, 0, "Called stack_is_empty on NULL");
	}
	if (stk->top == NULL) {
		return true;
	}
	return false;
}

bool stack_push(stack_t stk, void *data) {
	if (!stk) {
		throw_err(ERR_INTERNAL, 0, "Called stack_push on NULL");
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
		throw_err(ERR_INTERNAL, 0, "Called stack_top on NULL");
	}
	if (stack_is_empty(stk)) {
		throw_err(ERR_INTERNAL, 0, "Called stack_top on empty stack");
	}
	return stk->top->data;
}

bool stack_pop(stack_t stk) {
	if (!stk) {
		throw_err(ERR_INTERNAL, 0, "Called stack_pop on NULL");
	}
	if (stack_is_empty(stk)) {
		return false;
	}

	stack_item_t top_temp = stk->top;
	stk->top = stk->top->next;						 // popping
	return s_item_dtor(top_temp, stk->dstr_data_cb); // cleaning
}

bool stack_dtor(stack_t stk) {
	if (!stk) {
		return false;
	}
	while (!stack_is_empty(stk)) {
		stack_pop(stk);
	}
	free(stk);
	return true;
}