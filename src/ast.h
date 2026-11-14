/**
 * @file ast.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Abstract syntax tree interface
 * @date 2026-10-20
 */

#ifndef AST_H
#define AST_H

// Tool: Gemini
// AST node definitions and interface designed with AI assistant support.
// Prompt: Design AST node structures for IFJ26 grammar rules, expressions, and selected
// extensions using a tagged union and linked-list chaining.
// Note: The architecture, memory representation, and node fields were interactively
// verified and refined to match project specifications.

#include "token.h"
#include <stddef.h>
#include <stdint.h>

typedef struct ast_node *ast_node_t;

/**
 * @brief linked list of ast nodes
 */
typedef struct ast_list {
	ast_node_t head; // first element of the linked list
	ast_node_t tail; // last element of the linked list
	size_t count;	 // count of the elements in the list
} ast_list_t;

/**
 * @brief Final data types (used in semantic analysis and code generation)
 */
typedef enum {
	TYPE_UNKNOWN, // Initial / unresolved type
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_UNIT,
} data_type_t;

/**
 * @brief AST node types
 */
typedef enum {
	// Program structure
	NODE_PROGRAM,
	NODE_BLOCK,
	NODE_EXTENSION, // extension definition

	// Statements
	NODE_FUNC_DEF,
	NODE_VAR_DEF,
	NODE_ASSIGN,
	NODE_IF,
	NODE_WHILE,
	NODE_FOR,
	NODE_RETURN,
	NODE_EXPR_STMT, // expression used as statement

	// Expressions
	NODE_BINARY_OP,
	NODE_UNARY_OP,
	NODE_FUNC_CALL,
	NODE_MEMBER_ACCESS, // member access or method call
	NODE_STRING_INTERP, // interpolated string
	NODE_TYPE_CAST,		// implicit / explicit conversion (this node adds semantic analysis)

	// Parameter lists and formal parameters
	NODE_PARAM_LIST, // group of parameters
	NODE_PARAM,		 // formal parameter (name: Type)
	NODE_ARG_LIST,	 // list of arguments

	// Leaves (atoms)
	NODE_IDENTIFIER,
	NODE_LIT_INT,
	NODE_LIT_DOUBLE,
	NODE_LIT_STRING,
	NODE_LIT_BOOL,
	NODE_LIT_UNIT,
} node_type_t;

struct ast_node {
	node_type_t type;
	ast_node_t next;	   // Chaining for sequences
	unsigned line_num;	   // Source line number for error reporting
	data_type_t eval_type; // Evaluated expression type
	void *sym_entry;	   // Pointer to symbol table entry

	union {
		// NODE_PROGRAM
		struct {
			ast_list_t items;
		} program;

		// NODE_BLOCK
		struct {
			ast_list_t statements;
		} block;

		// NODE_EXTENSION
		struct {
			char *receiver_name;
			char *unique_receiver_name;
			char *receiver_type;
			ast_list_t methods; // Linked list of NODE_FUNC_DEF
		} extension;

		// NODE_FUNC_DEF
		struct {
			char *name;
			bool is_main;			// Annotated with @main
			bool has_parens;		// False for parameterless def foo: Int = ...
			ast_list_t param_lists; // List of NODE_PARAM_LIST
			char *return_type;		// Explicit return type or NULL
			ast_node_t body;		// NODE_BLOCK or expression
		} func_def;

		// NODE_PARAM_LIST
		struct {
			ast_list_t params; // Linked list of NODE_PARAM
		} param_list;

		// NODE_PARAM
		struct {
			char *name;
			char *unique_name;
			char *type_name;
		} param;

		// NODE_VAR_DEF
		struct {
			char *name;
			char *unique_name;
			bool is_val;		  // true = val, false = var
			char *type_name;	  // Explicit type name or NULL
			ast_node_t init_expr; // Initializing expression
		} var_def;

		// NODE_ASSIGN
		struct {
			char *target_id;
			char *unique_target_id;
			ast_node_t expr;
		} assign;

		// NODE_IF
		struct {
			ast_node_t condition;
			// Branch: NODE_BLOCK or a simple statement (a one-line branch is NODE_EXPR_STMT).
			// In an if expression the value of a branch is the expr of its NODE_EXPR_STMT,
			// or of the last NODE_EXPR_STMT in its block.
			ast_node_t then_branch;
			// Like then_branch, or a nested NODE_IF for "else if", or NULL (never if is_expr).
			ast_node_t else_branch;
			// true if the if is used as an expression, set on every link of an "else if" chain
			bool is_expr;
		} if_stmt;

		// NODE_WHILE
		struct {
			ast_node_t condition;
			ast_node_t body; // NODE_BLOCK or expression
		} while_stmt;

		// NODE_FOR
		struct {
			char *var_name;
			char *unique_var_name;
			bool is_inclusive; // true for 'to' (inclusive), false for 'until' (exclusive)
			ast_node_t from_expr;
			ast_node_t to_expr;
			ast_node_t step_expr; // NULL for default step
			ast_list_t guards;	  // Linked list of guard expressions
			ast_node_t body;	  // NODE_BLOCK or expression
		} for_stmt;

		// NODE_RETURN
		struct {
			ast_node_t expr; // Return expression or NULL
		} return_stmt;

		// NODE_EXPR_STMT
		struct {
			ast_node_t expr;
		} expr_stmt;

		// NODE_FUNC_CALL
		struct {
			ast_node_t callee;	  // Expression being called (e.g. IDENTIFIER, FUNC_CALL, LAMBDA)
			bool has_parens;	  // False for parameterless calls without parens
			ast_list_t arg_lists; // Linked list of lists of arguments
		} func_call;

		// NODE_MEMBER_ACCESS
		struct {
			ast_node_t target;
			char *member_name;
		} member_access;

		// NODE_ARG_LIST
		struct {
			ast_list_t args;
		} arg_list;

		// NODE_STRING_INTERP
		struct {
			// Linked list of string literals and expressions, in order. Empty text between the
			// parts is dropped, so an empty list is the empty string.
			ast_list_t parts;
		} string_interp;

		// NODE_TYPE_CAST
		struct {
			data_type_t target_type;
			ast_node_t operand;
		} type_cast;

		// NODE_BINARY_OP
		struct {
			op_type_t operator_type;
			ast_node_t left;
			ast_node_t right;
		} binary_op;

		// NODE_UNARY_OP
		struct {
			op_type_t operator_type;
			ast_node_t operand;
		} unary_op;

		// NODE_IDENTIFIER
		struct {
			char *name;
			char *unique_name;
		} id;

		// Literals
		int32_t int_val;
		double double_val;
		char *string_val;
		bool bool_val;
	} as;
};

/**
 * @brief Allocates and initializes a new AST node of the specified type
 * @param type Type of node to create
 * @param line_num Line number in source code where node begins
 * @return Pointer to newly allocated AST node, or NULL on allocation failure
 */
ast_node_t ast_create_node(node_type_t type, unsigned line_num);

/**
 * @brief Initializes an empty AST list
 * @param list Pointer to list to initialize
 */
void ast_list_init(ast_list_t *list);

/**
 * @brief Appends an AST node to the list in O(1) time
 * @param list Pointer to list
 * @param item Node to append
 * @return true on success, false if list or item is NULL
 */
bool ast_list_append(ast_list_t *list, ast_node_t item);

/**
 * @brief Recursively frees an AST subtree and all its allocated resources
 * @param root Root node of the subtree to be freed
 * @return NULL (useful for resetting caller's pointer)
 */
ast_node_t ast_dtor(ast_node_t root);

#endif
