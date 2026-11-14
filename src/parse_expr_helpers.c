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
 * @return pointer to the new entry, nullptr if allocation fails (the error is printed)
 */
static expr_entry_t *expr_entry_alloc(expr_entry_kind_t kind) {
	expr_entry_t *entry = calloc(1, sizeof(*entry));
	if (entry == nullptr) {
		print_error(ERR_INTERNAL, 0, "Failed to allocate expression stack entry");
		return nullptr;
	}
	entry->kind = kind;
	return entry;
}

expr_entry_t *expr_entry_new_terminal(expr_symbol_t symbol, token tkn) {
	expr_entry_t *entry = expr_entry_alloc(EXPR_ENTRY_TERMINAL);
	if (entry == nullptr) {
		return nullptr;
	}
	entry->as.terminal.symbol = symbol;
	entry->as.terminal.tkn = tkn;
	return entry;
}

expr_entry_t *expr_entry_new_node(ast_node_t node) {
	expr_entry_t *entry = expr_entry_alloc(EXPR_ENTRY_NODE);
	if (entry == nullptr) {
		return nullptr;
	}
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

bool expr_stack_push(stack_t stk, expr_entry_t *entry) {
	if (entry == nullptr) {
		return false; // the failed allocation of the entry is already reported
	}
	if (!stack_push(stk, entry)) {
		expr_entry_free(entry);
		print_error(ERR_INTERNAL, 0, "Failed to push onto the expression stack");
		return false;
	}
	return true;
}

expr_entry_t *expr_stack_top(stack_t stk) { return (expr_entry_t *)stack_top(stk); }

stack_t expr_stack_create(void) {
	// NULL destroyer on purpose: entries are freed only by expr_entry_free (see the header).
	stack_t stk = stack_init(nullptr);
	if (stk == nullptr) {
		print_error(ERR_INTERNAL, 0, "Failed to create the expression stack");
		return nullptr;
	}
	if (!expr_stack_push(stk, expr_entry_new_terminal(EXPR_SYMBOL_END, (token){}))) {
		expr_stack_destroy(stk);
		return nullptr;
	}
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
 * @brief Helper function: Pushes the entries popped by expr_stack_expose_terminal back
 * @param stk Target stack
 * @param above Entries popped by expr_stack_expose_terminal, topmost first
 * @param count Number of entries in the above array
 * @return true on success, false if a push failed (the error is printed)
 */
static bool expr_stack_restore(stack_t stk, expr_entry_t *above[], size_t count) {
	while (count > 0) {
		if (!expr_stack_push(stk, above[--count])) {
			return false;
		}
	}
	return true;
}

/**
 * @brief Helper function: Pops the nodes off the top until a terminal is on top
 * @param stk Target stack
 * @param[out] above Receives the popped entries, topmost first
 * @param capacity Size of the above array
 * @return number of popped entries, pair with expr_stack_restore, EXPR_STACK_FAILED if there is
 * no terminal or more than capacity entries above it (the error is printed)
 */
static size_t expr_stack_expose_terminal(stack_t stk, expr_entry_t *above[], size_t capacity) {
	size_t count = 0;
	while (true) {
		if (stack_is_empty(stk)) {
			print_error(ERR_INTERNAL, 0, "Expression stack has no terminal");
			return EXPR_STACK_FAILED;
		}
		expr_entry_t *top = expr_stack_top(stk);
		if (top->kind == EXPR_ENTRY_TERMINAL) {
			return count;
		}
		if (count == capacity) {
			print_error(ERR_INTERNAL, 0, "Too many entries above the nearest terminal");
			return EXPR_STACK_FAILED;
		}
		above[count++] = top;
		stack_pop(stk);
	}
}

expr_symbol_t expr_stack_nearest_terminal(stack_t stk) {
	expr_entry_t *above[EXPR_STACK_SCAN_LIMIT];
	size_t count = expr_stack_expose_terminal(stk, above, EXPR_STACK_SCAN_LIMIT);
	if (count == EXPR_STACK_FAILED) {
		return EXPR_SYMBOL_INVALID;
	}
	expr_symbol_t symbol = expr_stack_top(stk)->as.terminal.symbol;
	if (!expr_stack_restore(stk, above, count)) {
		return EXPR_SYMBOL_INVALID;
	}
	return symbol;
}

bool expr_stack_insert_marker(stack_t stk) {
	expr_entry_t *above[EXPR_STACK_SCAN_LIMIT];
	size_t count = expr_stack_expose_terminal(stk, above, EXPR_STACK_SCAN_LIMIT);
	if (count == EXPR_STACK_FAILED) {
		return false;
	}
	return expr_stack_push(stk, expr_entry_new_marker()) && expr_stack_restore(stk, above, count);
}

size_t expr_stack_pop_handle(stack_t stk, expr_entry_t *handle[], size_t capacity) {
	size_t count = 0;
	while (true) {
		if (stack_is_empty(stk)) {
			print_error(ERR_INTERNAL, 0, "No '<' marker on the expression stack");
			goto failed;
		}
		expr_entry_t *top = expr_stack_top(stk);
		stack_pop(stk);
		if (top->kind == EXPR_ENTRY_MARKER) {
			expr_entry_free(top);
			break;
		}
		if (count == capacity) {
			expr_entry_free(top);
			print_error(ERR_INTERNAL, 0, "Handle on the expression stack is too long");
			goto failed;
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

failed:
	// The entries popped so far are not on the stack any more, so nobody else would free them.
	for (size_t i = 0; i < count; i++) {
		expr_entry_free(handle[i]);
	}
	return EXPR_STACK_FAILED;
}
