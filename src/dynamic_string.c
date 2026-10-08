/**
 * @file dynamic_string.c
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation for dynamic string ADT
 *
 * @todo As of now, contains only basic functionality. For further needs must be extended
 */

#include "dynamic_string_private.h"
#include <stdlib.h>
#include <string.h>

dstring_t dstring_init() {
	dstring_t d_str = malloc(sizeof(struct dstring));
	char *string_memory_ptr = malloc(sizeof(char));

	if (d_str && string_memory_ptr) {
		// If memory is allocated, initialize the values accordingly
		*string_memory_ptr = '\0';
		*d_str = (struct dstring){.length = 0, .size = 1, .string = string_memory_ptr};
		return d_str;
	} else {
		// Failed memory alloc
		free(d_str);
		free(string_memory_ptr);
		return nullptr;
	}
}

bool dstring_inflate(dstring_t d_str) {
	char *tmp = realloc(d_str->string, d_str->size * 2);
	if (tmp) {
		// If memory is reallocated, write it to the structure
		d_str->size *= 2;
		d_str->string = tmp;
		return true;
	} else {
		// Failed memory alloc
		return false;
	}
}

bool dstring_append_char(dstring_t d_str, char c) {
	if (d_str->length + 2 > d_str->size) // If won't fit, inflate needed
	{
		if (!dstring_inflate(d_str)) // If inflate fails
		{
			return false;
		}
	}
	// Insert character
	d_str->string[d_str->length] = c;
	d_str->length++;
	d_str->string[d_str->length] = '\0';
	return true;
}

char *dstring_get_string(dstring_t d_str) {
	char *str = malloc(d_str->length + 1);
	if (str) {
		strcpy(str, d_str->string); // NOLINT
	}
	return str;
}

dstring_t dstring_dtor(dstring_t d_str) {
	free(d_str->string);
	d_str->string = nullptr;
	d_str->length = 0;
	d_str->size = 0;
	free(d_str);
	return nullptr;
}

unsigned long dstring_get_len(dstring_t d_str) { return d_str->length; }
