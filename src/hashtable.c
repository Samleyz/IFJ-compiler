/**
 * @file hashtable.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of hashtable ADT
 */

#include "hashtable_private.h"
#include <assert.h>
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
	// Allocate memory
	htable_t htab = malloc(sizeof(struct htable));
	htable_box_t *boxes_memory_ptr = calloc(MIN_HTABLE_SIZE, sizeof(htable_box_t));
	if (htab && boxes_memory_ptr) { // Successful alloc
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

size_t htable_record_count(htable_t htab) {
	assert(htab && "Called htable_record_count on nullptr.");
	return htab->used;
}

bool htable_write(htable_t htab, const char *key, void *data) {
	assert(htab && "Called htable_write on nullptr.");
	if (htab->used + 1 > htab->size * 3 / 4) { // Will occupancy be over 75%
		// Try to make the table larger
		bool successful_resize = upscale_htab(htab);
		if (!successful_resize) {
			return false;
		}
	}

	// Store
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) { // Search boxes
		if (!htab->boxes[current_idx].key) {  // Is free
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
		} else if (!strcmp(htab->boxes[current_idx].key, key)) { // Has same key
			// Update data there
			if (htab->data_dtor && htab->boxes[current_idx].data) {
				htab->data_dtor(htab->boxes[current_idx].data);
			}
			htab->boxes[current_idx].data = data;
			return true;
		}
		// Occupied -> move to next
		current_idx = get_index(current_idx + 1, htab->size);
	}
	return false;
}

void *htable_read(htable_t htab, const char *key) {
	assert(htab && "Called htable_read on nullptr.");
	// Search boxes
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) {
		if (!htab->boxes[current_idx].key) { // Found empty
			break;
		}
		if (0 == strcmp(key, htab->boxes[current_idx].key)) { // Found searched key
			// Found
			return htab->boxes[current_idx].data;
		}
		// Different one -> move to next
		current_idx = get_index(current_idx + 1, htab->size);
	}
	return nullptr;
}

bool htable_remove(htable_t htab, const char *key) {
	assert(htab && "Called htable_remove on nullptr.");
	// Search boxes
	size_t current_idx = get_index(hash_function(key), htab->size);
	for (size_t i = 0; i < htab->size; i++) {
		if (!htab->boxes[current_idx].key) { // Found empty
			break;
		}
		if (0 == strcmp(key, htab->boxes[current_idx].key)) { // Found searched key
			// Remove
			if (htab->data_dtor && htab->boxes[current_idx].data) {
				htab->data_dtor(htab->boxes[current_idx].data);
			}
			htab->boxes[current_idx].data = nullptr;
			free(htab->boxes[current_idx].key);
			htab->boxes[current_idx].key = nullptr;
			htab->used--; // Decrement used counter
			// Shift chained entries to fill gap
			size_t source_idx = current_idx;
			while (42) {
				// Find movable entry
				while (42) {
					source_idx = get_index(source_idx + 1, htab->size);
					if (!htab->boxes[source_idx].key) { // Found empty
						// None other to move
						return true;
					}
					// Calculate if source can be moved to current
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
		// Different entry -> try next
		current_idx = get_index(current_idx + 1, htab->size);
	}
	// Not found
	return false;
}

htable_t htable_dtor(htable_t htab) {
	if (htab) {
		// Clear entries
		for (size_t i = 0; i < htab->size; i++) {
			free(htab->boxes[i].key);
			if (htab->data_dtor && htab->boxes[i].data) {
				htab->data_dtor(htab->boxes[i].data);
			}
		}
		// Clear array
		free(htab->boxes);
		htab->boxes = nullptr;
		// Clear structure
		free(htab);
	}
	return (htable_t) nullptr;
}

bool upscale_htab(htable_t htab) {
	// Create larger
	htable_box_t *boxes_memory_ptr = calloc(htab->size * 2, sizeof(htable_box_t));
	if (boxes_memory_ptr) {
		// Fill out new data
		struct htable new_htab = {.boxes = boxes_memory_ptr,
								  .data_dtor = htab->data_dtor,
								  .size = htab->size * 2,
								  .used = 0};
		// Transfer entries
		for (size_t i = 0; i < htab->size; i++) { // For all in old htab
			if (htab->boxes[i].key) {			  // If it is entry
				// Move to new one
				size_t current_idx = get_index(hash_function(htab->boxes[i].key), new_htab.size);
				for (size_t j = 0; j < new_htab.size; j++) { // Search for free
					if (!new_htab.boxes[current_idx].key) {	 // Free
						// Move data there
						new_htab.boxes[current_idx].key = htab->boxes[i].key;
						new_htab.boxes[current_idx].data = htab->boxes[i].data;
						new_htab.used++; // Increment used counter
						break;
					}
					// Not free -> move to next
					current_idx = get_index(current_idx + 1, new_htab.size);
				}
			}
		}
		// Clear old and exchange it for new
		free(htab->boxes);
		*htab = new_htab;
		return true;
	} else {
		// Failed memory alloc
		return false;
	}
}
