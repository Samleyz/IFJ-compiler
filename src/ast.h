/**
 * @file ast.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Abstract syntax tree interface
 * @date 2026-10-20
 */

#ifndef AST_H
#define AST_H

typedef struct ast_node *ast_node_t;

/**
 * @brief Frees all nodes of the AST recursively
 *
 * @param[in] root Root node of the AST to be freed
 * @return NULL - can be used to override variable after destruction
 */
ast_node_t ast_dtor(ast_node_t root);

/**
 * @brief Creates and allocates a new AST node
 *
 * @return Pointer to newly allocated AST node, or NULL on failure
 */
ast_node_t create_node(void);

#endif
