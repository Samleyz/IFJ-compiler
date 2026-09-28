/**
 * @file dynamic_string_interface.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Public interface for dynamic string ADT
 *
 * @todo As of now, contains only basic functionality. For further needs must be extended
 */

#ifndef DYNAMIC_STRING_INTERFACE_H
#define DYNAMIC_STRING_INTERFACE_H

struct dstring; // Ignore (for linking)

/**
 * @brief Type that is used to manipulate with dynamic string
 */
typedef struct dstring *dstring_t;

/**
 * @brief Initializes empty dynamic string
 *
 * @return dstring_t Initialized dynamic string (nullptr if error)
 */
dstring_t dstring_init();

/**
 * @brief Inserts single character at the end of dynamic string
 *
 * @param d_str Dynamic string to insert to
 * @param c Character, that should be inserted
 * @return true Success
 * @return false Failed
 */
bool dstring_append_char(dstring_t d_str, char c);

/**
 * @brief Translates the dynamic string content into heap allocated C library compatible string
 *
 * @param d_str Dynamic string to translate
 * @return char* Heap allocated compatible C string (nullptr if failed)
 */
char *dstring_get_string(dstring_t d_str);

/**
 * @brief Clears and frees dynamic string
 *
 * @param d_str Dynamic string to destruct
 * @return dstring_t Always nullptr (can be used to overide dstring_t variable after destruction)
 */
dstring_t dstring_dtor(dstring_t d_str);

#endif
