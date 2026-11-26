/**
 * Implementation of a compiler for the imperative language IFJ26
 *
 * @file parser.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Interface for parser, includes functions for work with parser
 * @date 2026-10-20
 */

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

/**
 * @brief Performs parser intialization
 */
void parser_init(void);

/**
 * @brief Performs parser cleanup, deletes ast
 */
void parser_cleanup(void);

/**
 * @brief Performs parsing of the whole program
 *
 * @return true if the parsing was successful, otherwise false
 */
ast_node_t parse_program(void);

#endif
