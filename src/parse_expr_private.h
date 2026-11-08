/**
 * @file parse_expr_private.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz) , Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief Internal declarations for precedence syntactic analysis of expressions
 * @date 2026-10-22
 */

#ifndef PARSE_EXPR_PRIVATE_H
#define PARSE_EXPR_PRIVATE_H

#include <stddef.h>

#include "ast.h"
#include "stack.h"
#include "token.h"

/*
 * Terminal classes used to index the precedence matrix.
 * Keep this order in sync with the rows and columns in parse_expr.c.
 * END is the parser's pseudo-terminal ($), not a lexer token.
 *
 * LEFT_PAREN starts a grouped expression; CALL_OPEN starts a postfix function
 * call after an expression. Both are produced from TOKEN_LPAREN, distinguished
 * by whether the parser was expecting an operand.
 */
typedef enum {
	EXPR_SYMBOL_END,
	EXPR_SYMBOL_OR,
	EXPR_SYMBOL_AND,
	EXPR_SYMBOL_EQUALITY,
	EXPR_SYMBOL_RELATIONAL,
	EXPR_SYMBOL_ADDITIVE,
	EXPR_SYMBOL_MULTIPLICATIVE,
	EXPR_SYMBOL_PREFIX,
	EXPR_SYMBOL_OPERAND,
	EXPR_SYMBOL_LEFT_PAREN,
	EXPR_SYMBOL_CALL_OPEN,
	EXPR_SYMBOL_RIGHT_PAREN,
	EXPR_SYMBOL_COUNT,
	EXPR_SYMBOL_INVALID = EXPR_SYMBOL_COUNT
} expr_symbol_t;

typedef enum {
	EXPR_RELATION_ERROR,
	EXPR_RELATION_SHIFT,
	EXPR_RELATION_MATCH,
	EXPR_RELATION_REDUCE
} expr_relation_t;

expr_relation_t expr_precedence_relation(expr_symbol_t left, expr_symbol_t right);
expr_symbol_t expr_symbol_from_token(const token *tkn, bool expects_operand);

/**
 * @brief Upper bound on non-terminal entries above the nearest terminal on the expression stack
 * @note In valid input at most one reduced node sits there; exceeding the limit means the table or
 * the loop is broken, and it is reported as an internal error
 */
#define EXPR_STACK_SCAN_LIMIT 8

/**
 * @brief Kinds of expression stack entries
 */
typedef enum {
	EXPR_ENTRY_TERMINAL, // a symbol class plus the token it came from (or END)
	EXPR_ENTRY_NODE,	 // an already reduced AST subtree
	EXPR_ENTRY_MARKER	 // the '<' marker: start of the handle that will be reduced
} expr_entry_kind_t;

/**
 * @brief Expression stack entry, kind selects which member of the union is valid
 */
typedef struct {
	expr_entry_kind_t kind; // kind of the entry
	union {
		struct {
			expr_symbol_t symbol; // symbol class, indexes the precedence table
			token tkn;			  // source token, owns string_val of identifiers and strings
		} terminal;				  // valid for EXPR_ENTRY_TERMINAL
		ast_node_t node;		  // valid for EXPR_ENTRY_NODE, owned until moved into a bigger node
	} as;
} expr_entry_t;

/**
 * @brief Creates a terminal entry
 * @param symbol Symbol class of the terminal
 * @param tkn Source token, the entry takes ownership of its string_val
 * @return pointer to the new entry, terminates with ERR_INTERNAL if allocation fails
 */
expr_entry_t *expr_entry_new_terminal(expr_symbol_t symbol, token tkn);

/**
 * @brief Creates a node entry
 * @param node AST subtree, the entry takes ownership of it
 * @return pointer to the new entry, terminates with ERR_INTERNAL if allocation fails
 */
expr_entry_t *expr_entry_new_node(ast_node_t node);

/**
 * @brief Creates a '<' marker entry
 * @return pointer to the new entry, terminates with ERR_INTERNAL if allocation fails
 */
expr_entry_t *expr_entry_new_marker(void);

/**
 * @brief Frees an entry and what it owns (token string or AST subtree)
 * @param entry Entry to be freed, nullptr is allowed
 */
void expr_entry_free(expr_entry_t *entry);

/**
 * @brief Creates the expression stack with the END ($) terminal at the bottom
 * @note The stack is created with stack_init(NULL) and must stay that way: the helpers pop entries
 * temporarily and push the same pointers back, so a destroyer callback would free entries that are
 * still in use. Entries are freed only by expr_entry_free and expr_stack_destroy
 * @return pointer to the new stack, terminates with ERR_INTERNAL if creation fails
 */
stack_t expr_stack_create(void);

/**
 * @brief Frees every remaining entry (and the nodes they own) and the stack itself
 * @param stk Stack to be destroyed, nullptr is allowed
 */
void expr_stack_destroy(stack_t stk);

/**
 * @brief Pushes an entry to the top of the stack
 * @param stk Target stack
 * @param entry Entry to be pushed, freed if the push fails
 * @note Terminates with ERR_INTERNAL if the push fails
 */
void expr_stack_push(stack_t stk, expr_entry_t *entry);

/**
 * @brief Returns the top entry without removing it
 * @param stk Target stack, must not be empty
 * @return pointer to the top entry
 */
expr_entry_t *expr_stack_top(stack_t stk);

/**
 * @brief Finds the nearest terminal on the stack, skipping nodes
 * @param stk Target stack, the stack is left unchanged
 * @return symbol class of the nearest terminal, used as the row of the precedence table
 * @note Terminates with ERR_INTERNAL if more than EXPR_STACK_SCAN_LIMIT entries are skipped
 */
expr_symbol_t expr_stack_nearest_terminal(stack_t stk);

/**
 * @brief Inserts a '<' marker directly above the nearest terminal (below any node on top)
 * @param stk Target stack
 * @note Terminates with ERR_INTERNAL if more than EXPR_STACK_SCAN_LIMIT entries are skipped
 */
void expr_stack_insert_marker(stack_t stk);

/**
 * @brief Pops everything above the nearest '<' marker and removes the marker
 * @param stk Target stack
 * @param[out] handle Receives the popped entries ordered left to right, as in the source
 * @param capacity Size of the handle array
 * @return number of entries written to handle, ownership passes to the caller
 * @note Terminates with ERR_INTERNAL if there is no marker or the handle exceeds capacity
 */
size_t expr_stack_pop_handle(stack_t stk, expr_entry_t *handle[], size_t capacity);

#endif
