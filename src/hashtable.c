/**
 * @file hashtable.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of hashtable ADT
 */

#include "hashtable_private.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint64_t hash_function(const char *key) {
	uint64_t hash = 14695981039346656037ull;
	for (int i = 0; key[i] != '\0'; i++) {
		hash = (hash ^ (unsigned char)key[i]) * 1099511628211ull;
	}
	hash = hash ^ (hash >> 32);
	return hash;
}

size_t get_index(uint64_t hash, size_t htab_size) { return hash % htab_size; }

htable_t htable_init(void htable_value_dtor(void *item)) {
	htable_t htab = malloc(sizeof(struct htable));
	htable_box_t *boxes_memory_ptr = calloc(MIN_HTABLE_SIZE, sizeof(htable_box_t));
	if (htab && boxes_memory_ptr) {
		// Fill out default values
		htab->boxes = boxes_memory_ptr;
		htab->size = MIN_HTABLE_SIZE;
		htab->used = 0;
		htab->data_dtor = htable_value_dtor;
		return htab;
	} else {
		// Failed memory alloc
		free(htab);
		free(boxes_memory_ptr);
		return nullptr;
	}
}

size_t htable_record_count(htable_t htab) { return htab->used; }

bool htable_write(htable_t htab, const char *key, void *data) {
	if (htab->used + 1 > htab->size) { // Will it fit
		// TODO-resize can be implemented here
		return false;
	}

	// Search boxes
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) {
		if (!htab->boxes[current_idx].key) { // Free
			// Store key copy on heap
			char *key_memory = strdup(key);
			if (!key_memory) {
				return false;
			}
			// Store data there
			htab->boxes[current_idx].key = key_memory;
			htab->boxes[current_idx].data = data;
			htab->used++; // Increment used counter
			return true;
		} else if (!strcmp(htab->boxes[current_idx].key, key)) { // Same key
			// Store data there
			if (htab->data_dtor && htab->boxes[current_idx].data) {
				htab->data_dtor(htab->boxes[current_idx].data);
			}
			htab->boxes[current_idx].data = data;
			return true;
		}
		current_idx = get_index(current_idx + 1, htab->size);
	}
	return false;
}

void *htable_read(htable_t htab, const char *key) {
	// Search boxes
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) {
		if (!htab->boxes[current_idx].key) {
			break;
		}
		if (0 == strcmp(key, htab->boxes[current_idx].key)) {
			// Found
			return htab->boxes[current_idx].data;
		}
		current_idx = get_index(current_idx + 1, htab->size);
	}
	return nullptr;
}

bool htable_remove(htable_t htab, const char *key) {
	// Search boxes
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) {
		if (!htab->boxes[current_idx].key) {
			break;
		}
		if (0 == strcmp(key, htab->boxes[current_idx].key)) { // Found
			// Remove
			if (htab->data_dtor && htab->boxes[current_idx].data) {
				htab->data_dtor(htab->boxes[current_idx].data);
			}
			htab->boxes[current_idx].data = nullptr;
			free(htab->boxes[current_idx].key);
			htab->boxes[current_idx].key = nullptr;
			htab->used--; // Decrement used counter
			// Shift
			size_t source_idx = current_idx;
			while (42) {
				// Find movable entry
				while (42) {
					source_idx = get_index(source_idx + 1, htab->size);
					if (!htab->boxes[source_idx].key) {
						// None to move
						return true;
					}
					size_t source_ideal_idx =
						get_index(hash_function(htab->boxes[source_idx].key), htab->size);
					if (get_index(htab->size + source_idx - source_ideal_idx, htab->size) >=
						get_index(htab->size + source_idx - current_idx, htab->size)) {
						// Movable
						break;
					}
				}
				// Move
				htab->boxes[current_idx].key = htab->boxes[source_idx].key;
				htab->boxes[current_idx].data = htab->boxes[source_idx].data;
				htab->boxes[source_idx].key = nullptr;
				htab->boxes[source_idx].data = nullptr;
				current_idx = source_idx;
			}
		}
		current_idx = get_index(current_idx + 1, htab->size);
	}
	// Not found
	return false;
}

htable_t htable_dtor(htable_t htab) {
	for (size_t i = 0; i < htab->size; i++) {
		free(htab->boxes[i].key);
		if (htab->data_dtor && htab->boxes[i].data) {
			htab->data_dtor(htab->boxes[i].data);
		}
	}
	free(htab->boxes);
	htab->boxes = nullptr;
	free(htab);
	return (htable_t) nullptr;
}
