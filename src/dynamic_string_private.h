/**
 * @file dynamic_string_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Internal private declarations for dynamic string ADT
 *
 * @todo As of now, contains only basic functionality. For further needs must be extended
 */

#ifndef DYNAMIC_STRING_PRIVATE_H
#define DYNAMIC_STRING_PRIVATE_H

#include "dynamic_string.h"

struct dstring {
	unsigned long size;	  /**< Allocated bite count */
	unsigned long length; /**< Current string length */
	char *string;		  /**< \0 terminated string content */
};

/**
 * @brief Allocates dynamic string to be twice as large
 *
 * @param d_str Dynamic string to inflate
 */
bool dstring_inflate(dstring_t d_str);

#endif
