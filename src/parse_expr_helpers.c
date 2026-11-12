/**
 * @file parse_expr_helpers.c
 * @author Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief Expression stack helpers for precedence syntactic analysis of expressions
 * @date 2026-10-22
 */

#include "parse_expr_private.h"

#include "error.h"

#include <stdlib.h>

/**
 * @brief Helper function: Allocates a zero-initialised entry of the given kind
 * @param kind Kind of the entry
 * @return pointer to the new entry, terminates with ERR_INTERNAL if allocation fails
 */
static expr_entry_t *expr_entry_alloc(expr_entry_kind_t kind) {
	expr_entry_t *entry = calloc(1, sizeof(*entry));
	if (entry == nullptr) {
		throw_err(ERR_INTERNAL, 0, "Failed to allocate expression stack entry");
	}
	entry->kind = kind;
	return entry;
}

expr_entry_t *expr_entry_new_terminal(expr_symbol_t symbol, token tkn) {
	expr_entry_t *entry = expr_entry_alloc(EXPR_ENTRY_TERMINAL);
	entry->as.terminal.symbol = symbol;
	entry->as.terminal.tkn = tkn;
	return entry;
}

expr_entry_t *expr_entry_new_node(ast_node_t node) {
	expr_entry_t *entry = expr_entry_alloc(EXPR_ENTRY_NODE);
	entry->as.node = node;
	return entry;
}

expr_entry_t *expr_entry_new_marker(void) { return expr_entry_alloc(EXPR_ENTRY_MARKER); }

void expr_entry_free(expr_entry_t *entry) {
	if (entry == nullptr) {
		return;
	}
	switch (entry->kind) {
	case EXPR_ENTRY_TERMINAL:
		free_token_attr(&entry->as.terminal.tkn);
		break;
	case EXPR_ENTRY_NODE:
		ast_dtor(entry->as.node);
		break;
	case EXPR_ENTRY_MARKER:
		break;
	}
	free(entry);
}

void expr_stack_push(stack_t stk, expr_entry_t *entry) {
	if (!stack_push(stk, entry)) {
		expr_entry_free(entry);
		throw_err(ERR_INTERNAL, 0, "Failed to push onto the expression stack");
	}
}

expr_entry_t *expr_stack_top(stack_t stk) { return (expr_entry_t *)stack_top(stk); }

stack_t expr_stack_create(void) {
	// NULL destroyer on purpose: entries are freed only by expr_entry_free (see the header).
	stack_t stk = stack_init(nullptr);
	if (stk == nullptr) {
		throw_err(ERR_INTERNAL, 0, "Failed to create the expression stack");
	}
	expr_stack_push(stk, expr_entry_new_terminal(EXPR_SYMBOL_END, (token){}));
	return stk;
}

void expr_stack_destroy(stack_t stk) {
	if (stk == nullptr) {
		return;
	}
	while (!stack_is_empty(stk)) {
		expr_entry_t *entry = expr_stack_top(stk);
		stack_pop(stk);
		expr_entry_free(entry);
	}
	stack_dtor(stk);
}

/**
 * @brief Helper function: Pops the nodes off the top until a terminal is on top
 * @param stk Target stack
 * @param[out] above Receives the popped entries, topmost first
 * @param capacity Size of the above array
 * @return number of popped entries, pair with expr_stack_restore
 */
static size_t expr_stack_expose_terminal(stack_t stk, expr_entry_t *above[], size_t capacity) {
	size_t count = 0;
	while (true) {
		if (stack_is_empty(stk)) {
			throw_err(ERR_INTERNAL, 0, "Expression stack has no terminal");
		}
		expr_entry_t *top = expr_stack_top(stk);
		if (top->kind == EXPR_ENTRY_TERMINAL) {
			return count;
		}
		if (count == capacity) {
			throw_err(ERR_INTERNAL, 0, "Too many entries above the nearest terminal");
		}
		above[count++] = top;
		stack_pop(stk);
	}
}

/**
 * @brief Helper function: Pushes the entries popped by expr_stack_expose_terminal back
 * @param stk Target stack
 * @param above Entries popped by expr_stack_expose_terminal, topmost first
 * @param count Number of entries in the above array
 */
static void expr_stack_restore(stack_t stk, expr_entry_t *above[], size_t count) {
	while (count > 0) {
		expr_stack_push(stk, above[--count]);
	}
}

expr_symbol_t expr_stack_nearest_terminal(stack_t stk) {
	expr_entry_t *above[EXPR_STACK_SCAN_LIMIT];
	size_t count = expr_stack_expose_terminal(stk, above, EXPR_STACK_SCAN_LIMIT);
	expr_symbol_t symbol = expr_stack_top(stk)->as.terminal.symbol;
	expr_stack_restore(stk, above, count);
	return symbol;
}

void expr_stack_insert_marker(stack_t stk) {
	expr_entry_t *above[EXPR_STACK_SCAN_LIMIT];
	size_t count = expr_stack_expose_terminal(stk, above, EXPR_STACK_SCAN_LIMIT);
	expr_stack_push(stk, expr_entry_new_marker());
	expr_stack_restore(stk, above, count);
}

size_t expr_stack_pop_handle(stack_t stk, expr_entry_t *handle[], size_t capacity) {
	size_t count = 0;
	while (true) {
		if (stack_is_empty(stk)) {
			throw_err(ERR_INTERNAL, 0, "No '<' marker on the expression stack");
		}
		expr_entry_t *top = expr_stack_top(stk);
		stack_pop(stk);
		if (top->kind == EXPR_ENTRY_MARKER) {
			expr_entry_free(top);
			break;
		}
		if (count == capacity) {
			throw_err(ERR_INTERNAL, 0, "Handle on the expression stack is too long");
		}
		handle[count++] = top;
	}

	// Entries were popped right to left; reverse them so handle[0] is the leftmost.
	for (size_t i = 0; i < count / 2; i++) {
		expr_entry_t *tmp = handle[i];
		handle[i] = handle[count - 1 - i];
		handle[count - 1 - i] = tmp;
	}
	return count;
}
