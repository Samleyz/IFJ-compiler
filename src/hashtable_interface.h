/**
 * @file hashtable_interface.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Public interface for hastable ADT
 */

#ifndef HASHTABLE_INTERFACE_H
#define HASHTABLE_INTERFACE_H

#include <stddef.h>

struct htable; // Ignore (for linking)

/**
 * @brief Type that is used to manipulate with hashtable
 */
typedef struct htable *htable_t;

/**
 * @brief Initializes empty hash table
 *
 * @param htable_value_dtor Function that is used to destruct htable values
 * @return htable_t Initialized hash table (nullptr if failed)
 */
htable_t htable_init(void htable_value_dtor(void *item));

/**
 * @brief Returns how many records are currently in hash table
 *
 * @param htab Hash table to get the count from
 * @return size_t Record count
 */
size_t htable_record_count(htable_t htab);

/**
 * @brief Stores pointer to specified heap allocated value in the hashtable
 *
 * @param htab Hash table to write to
 * @param key Key under which you want the data to be stored
 * @param data Pointer to data you want to store (must be heap allocated)
 * @return true Success
 * @return false Failed
 */
bool htable_write(htable_t htab, const char *key, void *data);

/**
 * @brief Gets you a pointer to data stored under specified key
 *
 * @param htab Hash table to search
 * @param key Searched key
 * @return void* Pointer to your specified value (nullptr if empty)
 */
void *htable_read(htable_t htab, const char *key);

/**
 * @brief Clears a record under specified key
 *
 * @param htab Hash table where to clear it
 * @param key Creared item key
 * @return true Item was cleared
 * @return false There already wasn't anything
 */
bool htable_remove(htable_t htab, const char *key);

/**
 * @brief Destructs specified hash table
 *
 * @param htab Hash table to destruct
 * @return htable_t Always nullptr (can be used to overide htable_t variable
 * after destruction)
 */
htable_t htable_dtor(htable_t htab);

#endif
