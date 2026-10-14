/**
 * @file lexer.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Interface for lexer, provides tokens on demand
 * @date 2026-10-04
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdio.h>

/**
 * @brief Performs lexical analysis and gets new token from source
 *
 * @return pointer to the new loaded token
 */
token lexer_get_token(void);

/**
 * @brief Performs cleanup after using lexer
 */
void lexer_cleanup(void);

/**
 * @brief Performs init of lexer, sets counter to zero, prepares stack
 *
 * @param [in] source file from which the lexer will read
 */
bool lexer_init(FILE *source);

#endif
