/**
 * @file code_gen.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Code generation interface
 * @date 2026-11-08
 */

#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "ast.h"
#include <stdio.h>

/**
 * @brief Initializes the code generator, sets output file and sets counters to zero
 *
 * @param[in] output_file The output file to be set
 */
void code_gen_setup(FILE *output_file);

/**
 * @brief Generates the IFJcode26 program to the set output file
 *
 * @param[in] root Root of the abstract syntax tree which contains the code to be printed
 * @return true if success, false otherwise
 */
bool generate_program(ast_node_t root);

#endif
