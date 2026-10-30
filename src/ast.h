/**
 * @file ast.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm0000@vutbr.cz)
 * @brief Abstract syntax tree interface
 * @date 2026-10-20
 */

#ifndef AST_H
#define AST_H

#include "token.h"

typedef struct ast_node *ast_node_t;

typedef enum {

	NODE_BINARY_OP,
	NODE_UNARY_OP,

	NODE_INT_LITERAL,
	NODE_DOUBLE_LITERAL,
	NODE_STRING_LITERAL,
	NODE_IDENTIFIER,

	NODE_IF,
	NODE_CASE,

	NODE_FUNC_DEF,
	NODE_FUNC_CALL,
	NODE_RETURN,

	NODE_WHILE,
	NODE_FOR,

	// many more to go right? :(
	// will be added continously as we do the grammar

} node_type;

typedef struct ast_node {
	node_type type;

	union {
		op_type_t operator_type; // operator type
		kw_type_t keyword;		 // keyword type
		int32_t int_val;		 // integer value
		double double_val;		 // double value
		bool bool_literal;		 // bool literal value
		char *string_val;		 // string literal value
	} data;

	ast_node_t child_node_1;
	ast_node_t child_node_2;
	ast_node_t child_node_3; // if...else...then...

} ast_node;

/**
 * @brief Creates and allocates a new AST node
 * @param n_type type of node, check node_type for full list
 * @return Pointer to newly allocated AST node, or NULL on failure
 */
ast_node_t create_node(node_type n_type);

/**
 * @brief Destroys the abstract syntax tree
 */
ast_node_t ast_dtor(void);

#endif
