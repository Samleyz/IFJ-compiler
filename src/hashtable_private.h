/**
 * Implementation of a compiler for the imperative language IFJ26
 *
 * @file hashtable_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Internal private declarations for hastable ADT
 */

#ifndef HASHTABLE_PRIVATE_H
#define HASHTABLE_PRIVATE_H

#include "hashtable.h"
#include <stdint.h>

#define MIN_HTABLE_SIZE 32 /**< Minimum amount of boxes in hashtable. Power of 2 */

/**
 * @brief Calculates hash value of string
 *
 * 64-bit FNV-1a (Fowler–Noll–Vo) hash function with XOR-folding
 *
 * @param key String to hash
 * @return size_t Computed hash value
 */
uint64_t hash_function(const char *key);

/**
 * @brief Computes index of entry
 *
 * @param hash Hash of entry key
 * @param htab_size Size of the internal hash table array
 * @return size_t Index of the entry
 */
size_t get_index(uint64_t hash, size_t htab_size);

/**
 * @brief Structure that represents single hash table box
 */
typedef struct {
	char *key;	/**< Key of the stored data (invalid when status is not set to OCCUPIED) */
	void *data; /**< Pointer to stored data under specified key, when status is OCCUPIED */
} htable_box_t;

struct htable {
	size_t size;			   /**< Hash table box count */
	size_t used;			   /**< Hash table used box count */
	void (*data_dtor)(void *); /**< Value destructor function */
	htable_box_t *boxes;	   /**< Array of hash table boxes */
};

/**
 * @brief Makes hastable two times larger
 *
 * @param htab Hastable to resize
 * @return true Success
 * @return false Failed
 */
bool upscale_htab(htable_t htab);

#endif
