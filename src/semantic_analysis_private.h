/**
 * @file semantic_analysis_private.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Header file for internal semantic analysis functions and data structures
 */

#ifndef SEMANTIC_ANALYSIS_PRIVATE_H
#define SEMANTIC_ANALYSIS_PRIVATE_H

#include "ast.h"
#include "semantic_analysis.h"
#include "symtable.h"

/**
 * @brief Prefills the symbol table with builtin functions and types
 *
 * @param symtable Symbol table to prefill
 * @return stable_t Prefilled symbol table (nullptr if failed)
 */
stable_t prefill_symbol_table();

/**
 * @brief Performs the first pass of semantic analysis on node
 *
 * @param node Node of AST to analyze
 * @param symtable Table of symbols
 * @return true Success
 * @return false Semantic error
 */
bool first_pass(ast_node_t node, stable_t symtable);

/**
 * @brief Performs the second pass of semantic analysis on node
 *
 * @param node Node of AST to analyze
 * @param symtable Table of symbols
 * @return true Success
 * @return false Semantic error
 */
bool second_pass_node(ast_node_t node, stable_t symtable);

#endif
