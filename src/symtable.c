/**
 * @file symtable.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of symtable data type using hashtable
 */

#include "hashtable.h"
#include "symtable_private.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

symbol_t stable_init_symbol() { return calloc(1, sizeof(struct symbol)); }

void stable_destroy_symbol(symbol_t sym) {
	if (sym)
		free(sym);
}

stable_t stable_init() {
	// Alloc
	stable_t stab = malloc(sizeof(struct stable));
	struct stable_layer *slayer = malloc(sizeof(struct stable_layer));
	htable_t htab = htable_init(stable_destroy_symbol);
	if (stab && slayer && htab) {
		// Success
		stab->top = slayer;
		stab->top->table = htab;
		stab->top->next = nullptr;
		return stab;
	} else {
		// Failed
		free(stab);
		free(slayer);
		htable_dtor(htab);
		return nullptr;
	}
}

bool stable_enter_scope(stable_t stab) {
	assert(stab && "Called stable_enter_scope on nullptr.");
	// Alloc
	struct stable_layer *slayer = malloc(sizeof(struct stable_layer));
	htable_t htab = htable_init((void (*)(void *))stable_destroy_symbol);
	if (slayer && htab) {
		// Success
		slayer->table = htab;
		slayer->next = stab->top;
		stab->top = slayer;
		return true;
	} else {
		// Failed
		free(slayer);
		htable_dtor(htab);
		return false;
	}
}

void stable_leave_scope(stable_t stab) {
	assert(stab && "Called stable_leave_scope on nullptr.");
	if (stab->top) { // If there is something
		// Pop
		struct stable_layer *tmp = stab->top;
		stab->top = stab->top->next;
		htable_dtor(tmp->table);
		free(tmp);
	}
}

bool stable_write(stable_t stab, const char *key, symbol_t entry) {
	assert(stab && "Called stable_write on nullptr.");
	if (stab->top) {
		if (htable_write(stab->top->table, key, entry)) {
			return true;
		}
	}
	return false;
}

symbol_t stable_find_alllevel(stable_t stab, const char *key) {
	assert(stab && "Called stable_find_alllevel on nullptr.");
	struct stable_layer *current_layer = stab->top;
	while (current_layer) { // Go though layers
		// Search each
		symbol_t sym = htable_read(current_layer->table, key);
		if (sym) {
			return sym;
		}
		// Move
		current_layer = current_layer->next;
	}
	// Not found
	return nullptr;
}

symbol_t stable_find_curlevel(stable_t stab, const char *key) {
	assert(stab && "Called stable_find_curlevel on nullptr.");
	if (stab->top) {
		return htable_read(stab->top->table, key);
	}
	return nullptr;
}

stable_t stable_dtor(stable_t stab) {
	if (stab) {
		while (stab->top) { // Pop all layers
			stable_leave_scope(stab);
		}
		// Free head
		free(stab);
	}
	return nullptr;
}
