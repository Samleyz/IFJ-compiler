/**
 * @file symtable.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of symtable data type using hashtable
 */

#include "hashtable.h"
#include "symtable_private.h"
#include <stdlib.h>
#include <string.h>

void symbol_dtor(void *sym) {
	symbol_t *s = (symbol_t *)sym;
	if (s) {
		if (s->key) {
			free(s->key);
		}
		free(s);
	}
}

stable_t stable_init() { return htable_init(symbol_dtor); }

size_t stable_record_count(stable_t stab) { return htable_record_count((htable_t)stab); }

bool stable_write(stable_t stab, symbol_t entry) {
	symbol_t *tmp_sym = malloc(sizeof(symbol_t));
	char *tmp_key = strdup(entry.key);
	if (tmp_sym && tmp_key) {
		if (htable_write((htable_t)stab, entry.key, tmp_sym)) {
			*tmp_sym = entry;
			tmp_sym->key = tmp_key;
			return true;
		}
	}
	free(tmp_sym);
	free(tmp_key);
	return false;
}

symbol_t *stable_read(stable_t stab, const char *key) {
	return (symbol_t *)htable_read((htable_t)stab, key);
}

bool stable_remove(stable_t stab, const char *key) { return htable_remove((htable_t)stab, key); }

stable_t stable_dtor(stable_t stab) {
	htable_dtor((htable_t)stab);
	return nullptr;
}
