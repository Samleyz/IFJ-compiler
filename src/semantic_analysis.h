/**
 * Implementation of a compiler for the imperative language IFJ26
 *
 * @file semantic_analysis.h
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Header file for semantic analysis functions
 */

#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "ast.h"

/**
 * @brief Performs semantic analysis on the given AST tree.
 *
 * @param root Abstract syntax tree to perform semantic analysis on
 * @return true Success
 * @return false Semantic error
 */
bool perform_semantic_analysis(ast_node_t root);

#endif
