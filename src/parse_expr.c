/**
 * @file parse_expr.c
 * @author Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief Implementation of precedence syntactic analysis for expressions
 * @date 2026-10-22
 */

#include "parse_expr.h"
#include "parse_expr_private.h"
#include "parser_grammar.h"

#include <stdlib.h>

static const expr_relation_t expr_precedence_table[EXPR_SYMBOL_COUNT][EXPR_SYMBOL_COUNT] = {
	// END
	{EXPR_RELATION_MATCH, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_ERROR},
	// OR
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// AND
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// equality
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_ERROR,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// relational
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_ERROR, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// additive
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// multiplicative
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// prefix
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_REDUCE},
	// operand
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_ERROR,
	 EXPR_RELATION_ERROR, EXPR_RELATION_ERROR, EXPR_RELATION_REDUCE},
	// left parenthesis
	{EXPR_RELATION_ERROR, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT,
	 EXPR_RELATION_SHIFT, EXPR_RELATION_SHIFT, EXPR_RELATION_MATCH},
	// right parenthesis
	{EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE,
	 EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_REDUCE, EXPR_RELATION_ERROR,
	 EXPR_RELATION_ERROR, EXPR_RELATION_ERROR, EXPR_RELATION_REDUCE},
};

expr_relation_t expr_precedence_relation(expr_symbol_t left, expr_symbol_t right) {
	// INVALID is deliberately outside the matrix; reject it before indexing.
	if ((unsigned)left >= EXPR_SYMBOL_COUNT || (unsigned)right >= EXPR_SYMBOL_COUNT) {
		return EXPR_RELATION_ERROR;
	}
	return expr_precedence_table[left][right];
}

expr_symbol_t expr_symbol_from_token(const token *tkn, bool expects_operand) {
	if (tkn == NULL) {
		return EXPR_SYMBOL_INVALID;
	}

	switch (tkn->type) {
	case TOKEN_IDENTIFIER:
		return EXPR_SYMBOL_OPERAND;
	case TOKEN_INTEGER:
		return EXPR_SYMBOL_OPERAND;
	case TOKEN_DOUBLE:
		return EXPR_SYMBOL_OPERAND;
	case TOKEN_STRING:
		return EXPR_SYMBOL_OPERAND;
	case TOKEN_BOOLEAN:
		return EXPR_SYMBOL_OPERAND;
	case TOKEN_LPAREN:
		return EXPR_SYMBOL_LEFT_PAREN;
	case TOKEN_RPAREN:
		return EXPR_SYMBOL_RIGHT_PAREN;
	case TOKEN_OPERATOR:
		switch (tkn->attribute.operator_type) {
		case OPERATOR_OR:
			return EXPR_SYMBOL_OR;
		case OPERATOR_AND:
			return EXPR_SYMBOL_AND;
		case OPERATOR_EQUAL:
			return EXPR_SYMBOL_EQUALITY;
		case OPERATOR_NOT_EQUAL:
			return EXPR_SYMBOL_EQUALITY;
		case OPERATOR_LESS:
			return EXPR_SYMBOL_RELATIONAL;
		case OPERATOR_LESS_EQUAL:
			return EXPR_SYMBOL_RELATIONAL;
		case OPERATOR_GREATER:
			return EXPR_SYMBOL_RELATIONAL;
		case OPERATOR_GREATER_EQUAL:
			return EXPR_SYMBOL_RELATIONAL;
		case OPERATOR_PLUS:
			// At expression start or after another operator/'(', these are unary.
			// After an operand or ')', they are binary additive operators.
			return expects_operand ? EXPR_SYMBOL_PREFIX : EXPR_SYMBOL_ADDITIVE;
		case OPERATOR_MINUS:
			return expects_operand ? EXPR_SYMBOL_PREFIX : EXPR_SYMBOL_ADDITIVE;
		case OPERATOR_STAR:
			return EXPR_SYMBOL_MULTIPLICATIVE;
		case OPERATOR_SLASH:
			return EXPR_SYMBOL_MULTIPLICATIVE;
		case OPERATOR_EX_MARK:
			return EXPR_SYMBOL_PREFIX;
		default:
			return EXPR_SYMBOL_INVALID;
		}
	default:
		return EXPR_SYMBOL_INVALID;
	}
}

/**
 * @brief Maximum number of entries in a handle: "( expression )" and "expression op expression"
 */
#define EXPR_HANDLE_MAX 3

/**
 * @brief Helper function: Classifies the current token of the parser for the precedence table
 * @param p Parser whose current token is classified
 * @param expects_operand True if an operand (and not an operator) may come next
 * @param paren_depth Number of '(' of this expression that are not closed yet
 * @return symbol class of the token, END if the token does not belong to the expression
 */
static expr_symbol_t expr_input_symbol(const parser_t *p, bool expects_operand,
									   size_t paren_depth) {
	expr_symbol_t symbol = expr_symbol_from_token(&p->curr_tkn, expects_operand);
	// Tokens outside of the expression (EOL, ',', 'then', ...) end it, and so does a ')' that does
	// not close a '(' of this expression: that one belongs to the caller.
	if (symbol == EXPR_SYMBOL_INVALID || (symbol == EXPR_SYMBOL_RIGHT_PAREN && paren_depth == 0)) {
		return EXPR_SYMBOL_END;
	}
	// An identifier right after an operand cannot continue the expression, so it ends it. These
	// are the words 'to', 'until' and 'by' of a for generator, they are identifiers by design.
	if (p->curr_tkn.type == TOKEN_IDENTIFIER && !expects_operand) {
		return EXPR_SYMBOL_END;
	}
	return symbol;
}

/**
 * @brief Helper function: Moves the current token of the parser onto the stack and advances
 * @param stk Expression stack
 * @param p Parser whose current token is consumed
 * @param symbol Symbol class of the token
 * @return true on success, false if the entry could not be pushed (the error is printed)
 */
static bool expr_push_token(stack_t stk, parser_t *p, expr_symbol_t symbol) {
	token tkn = p->curr_tkn;
	// The entry takes over the string of the token, the parser must not free it when it advances.
	if (tkn.type == TOKEN_IDENTIFIER || tkn.type == TOKEN_STRING) {
		p->curr_tkn.attribute.string_val = nullptr;
	}
	if (!expr_stack_push(stk, expr_entry_new_terminal(symbol, tkn))) {
		return false;
	}
	parser_advance(p);
	return true;
}

/**
 * @brief Helper function: Creates an AST node
 * @param type Type of the node
 * @param line_num Source line of the node
 * @return pointer to the new node, nullptr if allocation fails (ERR_INTERNAL is printed)
 */
static ast_node_t expr_create_node(node_type_t type, unsigned line_num) {
	ast_node_t node = ast_create_node(type, line_num);
	if (node == nullptr) {
		print_error(ERR_INTERNAL, 0, "Failed to allocate AST node");
	}
	return node;
}

/**
 * @brief Helper function: Takes the AST node out of a node entry, the entry stops owning it
 * @param entry Node entry
 * @return the AST node of the entry
 */
static ast_node_t expr_take_node(expr_entry_t *entry) {
	ast_node_t node = entry->as.node;
	entry->as.node = nullptr;
	return node;
}

/**
 * @brief Helper function: Creates a leaf node from an operand token
 * @param tkn Operand token, its string (if any) is moved into the node
 * @return pointer to the new leaf node, nullptr if the token is not an operand or the allocation
 * failed (the token keeps its string then)
 */
static ast_node_t expr_make_leaf(token *tkn) {
	ast_node_t node = nullptr;
	switch (tkn->type) {
	case TOKEN_IDENTIFIER:
		node = expr_create_node(NODE_IDENTIFIER, tkn->line_num);
		if (node != nullptr) {
			node->as.id.name = tkn->attribute.string_val;
			tkn->attribute.string_val = nullptr;
		}
		break;
	case TOKEN_STRING:
		node = expr_create_node(NODE_LIT_STRING, tkn->line_num);
		if (node != nullptr) {
			node->as.string_val = tkn->attribute.string_val;
			tkn->attribute.string_val = nullptr;
		}
		break;
	case TOKEN_INTEGER:
		node = expr_create_node(NODE_LIT_INT, tkn->line_num);
		if (node != nullptr) {
			node->as.int_val = tkn->attribute.int_val;
		}
		break;
	case TOKEN_DOUBLE:
		node = expr_create_node(NODE_LIT_DOUBLE, tkn->line_num);
		if (node != nullptr) {
			node->as.double_val = tkn->attribute.double_val;
		}
		break;
	case TOKEN_BOOLEAN:
		node = expr_create_node(NODE_LIT_BOOL, tkn->line_num);
		if (node != nullptr) {
			node->as.bool_val = tkn->attribute.bool_literal;
		}
		break;
	default:
		break;
	}
	return node;
}

/**
 * @brief Helper function: Checks that an entry is a terminal of the given symbol class
 * @param entry Entry to be checked
 * @param symbol Expected symbol class
 * @return true if the entry is a terminal of that class
 */
static bool expr_is_terminal(const expr_entry_t *entry, expr_symbol_t symbol) {
	return entry->kind == EXPR_ENTRY_TERMINAL && entry->as.terminal.symbol == symbol;
}

/**
 * @brief Helper function: Checks that an entry is a reduced AST node
 * @param entry Entry to be checked
 * @return true if the entry is a node
 */
static bool expr_is_node(const expr_entry_t *entry) { return entry->kind == EXPR_ENTRY_NODE; }

/**
 * @brief Helper function: Checks that an entry is a terminal of a binary operator class
 * @param entry Entry to be checked
 * @return true if the entry is a binary operator
 */
static bool expr_is_binary_operator(const expr_entry_t *entry) {
	if (entry->kind != EXPR_ENTRY_TERMINAL) {
		return false;
	}
	switch (entry->as.terminal.symbol) {
	case EXPR_SYMBOL_OR:
	case EXPR_SYMBOL_AND:
	case EXPR_SYMBOL_EQUALITY:
	case EXPR_SYMBOL_RELATIONAL:
	case EXPR_SYMBOL_ADDITIVE:
	case EXPR_SYMBOL_MULTIPLICATIVE:
		return true;
	default:
		return false;
	}
}

/**
 * @brief Helper function: Builds the AST node for a handle, validating the shape of the handle
 * @param handle Entries of the handle ordered left to right, what is used is moved into the node
 * @param count Number of entries in the handle
 * @return pointer to the new node, nullptr if the handle has no valid shape or the allocation
 * failed (in that case nothing was moved out of the handle)
 */
static ast_node_t expr_build_node(expr_entry_t *handle[], size_t count) {
	// operand
	if (count == 1 && expr_is_terminal(handle[0], EXPR_SYMBOL_OPERAND)) {
		return expr_make_leaf(&handle[0]->as.terminal.tkn);
	}
	// prefix expression
	if (count == 2 && expr_is_terminal(handle[0], EXPR_SYMBOL_PREFIX) && expr_is_node(handle[1])) {
		token *op = &handle[0]->as.terminal.tkn;
		ast_node_t node = expr_create_node(NODE_UNARY_OP, op->line_num);
		if (node == nullptr) {
			return nullptr;
		}
		node->as.unary_op.operator_type = op->attribute.operator_type;
		node->as.unary_op.operand = expr_take_node(handle[1]);
		return node;
	}
	// ( ) is the Unit literal
	if (count == 2 && expr_is_terminal(handle[0], EXPR_SYMBOL_LEFT_PAREN) &&
		expr_is_terminal(handle[1], EXPR_SYMBOL_RIGHT_PAREN)) {
		return expr_create_node(NODE_LIT_UNIT, handle[0]->as.terminal.tkn.line_num);
	}
	// expression binary_operator expression
	if (count == 3 && expr_is_node(handle[0]) && expr_is_binary_operator(handle[1]) &&
		expr_is_node(handle[2])) {
		token *op = &handle[1]->as.terminal.tkn;
		ast_node_t node = expr_create_node(NODE_BINARY_OP, op->line_num);
		if (node == nullptr) {
			return nullptr;
		}
		node->as.binary_op.operator_type = op->attribute.operator_type;
		node->as.binary_op.left = expr_take_node(handle[0]);
		node->as.binary_op.right = expr_take_node(handle[2]);
		return node;
	}
	// ( expression ) is just the inner expression
	if (count == 3 && expr_is_terminal(handle[0], EXPR_SYMBOL_LEFT_PAREN) &&
		expr_is_node(handle[1]) && expr_is_terminal(handle[2], EXPR_SYMBOL_RIGHT_PAREN)) {
		return expr_take_node(handle[1]);
	}
	return nullptr;
}

/**
 * @brief Helper function: Reduces the handle above the nearest marker to a single node
 * @param stk Expression stack
 * @param p Parser, used for the line of an error
 * @return true on success, false if the handle has no valid shape (syntax error is printed) or
 * a step failed (internal error is printed)
 */
static bool expr_reduce(stack_t stk, const parser_t *p) {
	expr_entry_t *handle[EXPR_HANDLE_MAX];
	size_t count = expr_stack_pop_handle(stk, handle, EXPR_HANDLE_MAX);
	if (count == EXPR_STACK_FAILED) {
		return false;
	}
	ast_node_t node = expr_build_node(handle, count);
	for (size_t i = 0; i < count; i++) {
		expr_entry_free(handle[i]);
	}
	if (node == nullptr) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Invalid expression");
		return false;
	}
	return expr_stack_push(stk, expr_entry_new_node(node));
}

/**
 * @brief Helper function: Takes the finished expression off the stack
 * @param stk Expression stack, expected to hold only END and the finished expression
 * @param p Parser, used for the line of an error
 * @return the root of the expression, nullptr if there is none (syntax error is printed)
 */
static ast_node_t expr_take_result(stack_t stk, const parser_t *p) {
	expr_entry_t *top = expr_stack_top(stk);
	if (!expr_is_node(top)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected expression");
		return nullptr;
	}
	stack_pop(stk);
	ast_node_t node = expr_take_node(top);
	expr_entry_free(top);
	return node;
}

/**
 * @brief Helper function: Parses the arguments of a call, the "(" is already consumed
 * @param p Parser, its current token is the first token after "("
 * @param[out] args Receives the argument expressions in order
 * @return true on success, the closing ")" is consumed, false on a syntax error (it is printed)
 */
static bool expr_parse_arguments(parser_t *p, ast_list_t *args) {
	while (!peek_token_type(p, TOKEN_RPAREN)) {
		// Each argument is an expression of its own, it ends at "," or ")".
		ast_node_t arg = parse_expression(p);
		if (arg == nullptr) {
			return false;
		}
		if (!ast_list_append(args, arg)) {
			ast_dtor(arg);
			print_error(ERR_INTERNAL, 0, "Failed to append a call argument");
			return false;
		}
		if (!peek_operator(p, OPERATOR_COMMA)) {
			break;
		}
		unsigned comma_line = p->curr_tkn.line_num;
		parser_advance(p);
		// A trailing comma is allowed only if a line break separates it from ")".
		if (peek_token_type(p, TOKEN_RPAREN) && p->curr_tkn.line_num <= comma_line) {
			print_error(ERR_SYNTACTIC, p->curr_tkn.line_num,
						"A trailing comma must be followed by a line break");
			return false;
		}
	}
	if (!peek_token_type(p, TOKEN_RPAREN)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected ')' after call arguments");
		return false;
	}
	parser_advance(p);
	return true;
}

/**
 * @brief Helper function: Turns the expression on top of the stack into the callee of a call
 * @param stk Expression stack, a node entry must be on top
 * @param p Parser, its current token is the "(" that opens the argument list
 * @return true on success, false on a syntax error (it is printed)
 */
static bool expr_parse_call(stack_t stk, parser_t *p) {
	expr_entry_t *entry = expr_stack_top(stk);
	stack_pop(stk);
	ast_node_t callee = expr_take_node(entry);
	expr_entry_free(entry);

	// From here on the call node owns the callee, so freeing it on an error frees both.
	ast_node_t call = expr_create_node(NODE_FUNC_CALL, callee->line_num);
	if (call == nullptr) {
		ast_dtor(callee);
		return false;
	}
	call->as.func_call.callee = callee;
	call->as.func_call.has_parens = true;

	parser_advance(p); // "("
	if (!expr_parse_arguments(p, &call->as.func_call.args)) {
		ast_dtor(call);
		return false;
	}
	return expr_stack_push(stk, expr_entry_new_node(call));
}

/**
 * @brief Helper function: Turns the expression on top of the stack into the target of a member
 * access
 * @param stk Expression stack, a node entry must be on top
 * @param p Parser, its current token is the "." that follows the expression
 * @return true on success, false on a syntax error (it is printed)
 * @note "x.name" and "x.name(arguments)" both build a NODE_MEMBER_ACCESS, is_call tells them apart
 */
static bool expr_parse_member(stack_t stk, parser_t *p) {
	parser_advance(p); // "."
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected a member name after '.'");
		return false;
	}

	// The target stays on the stack until the node exists, so an allocation failure frees it there.
	ast_node_t member = expr_create_node(NODE_MEMBER_ACCESS, p->curr_tkn.line_num);
	if (member == nullptr) {
		return false;
	}
	member->as.member_access.member_name = p->curr_tkn.attribute.string_val;
	p->curr_tkn.attribute.string_val = nullptr;
	expr_entry_t *entry = expr_stack_top(stk);
	stack_pop(stk);
	member->as.member_access.target = expr_take_node(entry);
	expr_entry_free(entry);
	parser_advance(p); // the member name

	// From here on the member node owns the target, so freeing it on an error frees both.
	if (peek_token_type(p, TOKEN_LPAREN)) {
		member->as.member_access.is_call = true;
		parser_advance(p); // "("
		if (!expr_parse_arguments(p, &member->as.member_access.args)) {
			ast_dtor(member);
			return false;
		}
	}
	return expr_stack_push(stk, expr_entry_new_node(member));
}

/**
 * @brief Helper function: Appends a part to the interpolated string, frees the part on failure
 * @param interp NODE_STRING_INTERP node
 * @param part Node to be appended, nullptr (an earlier failure) makes the call fail
 * @return true on success, false on failure (the error is printed)
 */
static bool expr_interp_append(ast_node_t interp, ast_node_t part) {
	if (part == nullptr) {
		return false;
	}
	if (!ast_list_append(&interp->as.string_interp.parts, part)) {
		ast_dtor(part);
		print_error(ERR_INTERNAL, 0, "Failed to append a part of an interpolated string");
		return false;
	}
	return true;
}

/**
 * @brief Helper function: Parses one part of an interpolated string and appends it
 * @param p Parser, its current token is the first token of the part
 * @param interp NODE_STRING_INTERP node that receives the part
 * @return true on success, false on a syntax error (it is printed)
 * @note The lexer sends the text between the parts as STRING tokens, an empty one carries no text
 * and is dropped
 */
static bool expr_parse_interp_part(parser_t *p, ast_node_t interp) {
	if (peek_token_type(p, TOKEN_STRING)) {
		const char *text = p->curr_tkn.attribute.string_val;
		if (text == nullptr || text[0] == '\0') {
			parser_advance(p);
			return true;
		}
		ast_node_t part = expr_create_node(NODE_LIT_STRING, p->curr_tkn.line_num);
		if (part == nullptr) {
			return false;
		}
		part->as.string_val = p->curr_tkn.attribute.string_val;
		p->curr_tkn.attribute.string_val = nullptr;
		parser_advance(p);
		return expr_interp_append(interp, part);
	}
	if (peek_token_type(p, TOKEN_IDENTIFIER)) {
		// "$name" is the value of a variable
		ast_node_t part = expr_create_node(NODE_IDENTIFIER, p->curr_tkn.line_num);
		if (part == nullptr) {
			return false;
		}
		part->as.id.name = p->curr_tkn.attribute.string_val;
		p->curr_tkn.attribute.string_val = nullptr;
		parser_advance(p);
		return expr_interp_append(interp, part);
	}
	if (peek_token_type(p, TOKEN_INTRPLTDS_EXP_START)) {
		// "${expression}", the expression ends by itself at the closing brace
		parser_advance(p);
		ast_node_t part = parse_expression(p);
		if (part == nullptr) {
			return false;
		}
		if (!peek_token_type(p, TOKEN_INTRPLTDS_EXP_END)) {
			ast_dtor(part);
			print_error(ERR_SYNTACTIC, p->curr_tkn.line_num,
						"Expected the end of the expression in an interpolated string");
			return false;
		}
		parser_advance(p);
		return expr_interp_append(interp, part);
	}
	print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Unexpected token in an interpolated string");
	return false;
}

/**
 * @brief Helper function: Parses an interpolated string
 * @param p Parser, its current token is the start of the interpolated string
 * @return pointer to the NODE_STRING_INTERP node, nullptr on a syntax error (it is printed)
 */
static ast_node_t expr_parse_interpolated(parser_t *p) {
	ast_node_t interp = expr_create_node(NODE_STRING_INTERP, p->curr_tkn.line_num);
	if (interp == nullptr) {
		return nullptr;
	}
	parser_advance(p); // the start of the interpolated string
	while (!peek_token_type(p, TOKEN_INTRPLTDS_END)) {
		if (!expr_parse_interp_part(p, interp)) {
			ast_dtor(interp);
			return nullptr;
		}
	}
	parser_advance(p); // the end of the interpolated string
	return interp;
}

/**
 * @brief Helper function: Parses an if expression with the grammar rule for if
 * @param p Parser, its current token is "if"
 * @return pointer to the NODE_IF node, nullptr on a syntax error (it is printed)
 */
static ast_node_t expr_parse_if(parser_t *p) {
	unsigned line = p->curr_tkn.line_num;
	ast_node_t node = parse_cond_stmt(p);
	if (node == nullptr) {
		return nullptr;
	}
	if (node->type != NODE_IF) {
		ast_dtor(node);
		print_error(ERR_SYNTACTIC, line, "Expected an if expression");
		return nullptr;
	}
	// "else if" arrives as a nested NODE_IF in the else branch. Every link of the chain is part of
	// the expression, and the last one must end with a plain else (spec, CONTROL).
	for (ast_node_t link = node;;) {
		link->as.if_stmt.is_expr = true;
		ast_node_t next = link->as.if_stmt.else_branch;
		if (next == nullptr) {
			unsigned link_line = link->line_num;
			ast_dtor(node);
			print_error(ERR_SYNTACTIC, link_line, "An if expression requires an else branch");
			return nullptr;
		}
		if (next->type != NODE_IF) {
			break;
		}
		link = next;
	}
	return node;
}

ast_node_t parse_expression(parser_t *p) {
	stack_t stk = expr_stack_create();
	if (stk == nullptr) {
		return nullptr;
	}
	bool expects_operand = true;
	size_t paren_depth = 0;
	ast_node_t result = nullptr;

	// Shift/reduce loop, see lecture slides 31-33.
	while (true) {
		expr_symbol_t input = expr_input_symbol(p, expects_operand, paren_depth);

		if (expects_operand && peek_token_type(p, TOKEN_INTRPLTDS_START)) {
			// An interpolated string is an operand like any other.
			ast_node_t node = expr_parse_interpolated(p);
			if (node == nullptr) {
				goto done;
			}
			if (!expr_stack_push(stk, expr_entry_new_node(node))) {
				goto done;
			}
			expects_operand = false;
			continue;
		}

		if (expects_operand && peek_keyword(p, KW_IF)) {
			// "if" used as an expression is parsed by the grammar rule for if, and the node it
			// returns is an operand like any other.
			ast_node_t node = expr_parse_if(p);
			if (node == nullptr) {
				goto done;
			}
			if (!expr_stack_push(stk, expr_entry_new_node(node))) {
				goto done;
			}
			expects_operand = false;
			continue;
		}

		if (input == EXPR_SYMBOL_LEFT_PAREN && !expects_operand) {
			// A '(' right after an operand or ')' opens a call. A call binds tighter than every
			// operator, so it is applied straight to the finished expression on top of the stack
			// and the table is not consulted.
			if (!expr_is_node(expr_stack_top(stk))) {
				// The callee is still an operand or a ')' on the stack: reduce it to a node first.
				if (!expr_reduce(stk, p)) {
					goto done;
				}
			} else if (!expr_parse_call(stk, p)) {
				goto done;
			}
			continue;
		}

		if (peek_operator(p, OPERATOR_DOT) && !expects_operand) {
			// A '.' right after an operand or ')' starts a member access. Like a call it binds
			// tighter than every operator and is applied straight to the finished expression on
			// top of the stack, so the table is not consulted and "-x.y" is "-(x.y)".
			if (!expr_is_node(expr_stack_top(stk))) {
				if (!expr_reduce(stk, p)) {
					goto done;
				}
			} else if (!expr_parse_member(stk, p)) {
				goto done;
			}
			continue;
		}

		expr_symbol_t top = expr_stack_nearest_terminal(stk);
		if (top == EXPR_SYMBOL_INVALID) {
			goto done;
		}

		switch (expr_precedence_relation(top, input)) {
		case EXPR_RELATION_SHIFT:
			if (!expr_stack_insert_marker(stk) || !expr_push_token(stk, p, input)) {
				goto done;
			}
			if (input == EXPR_SYMBOL_LEFT_PAREN) {
				paren_depth++;
			}
			expects_operand = input != EXPR_SYMBOL_OPERAND && input != EXPR_SYMBOL_RIGHT_PAREN;
			break;
		case EXPR_RELATION_MATCH:
			if (top == EXPR_SYMBOL_END) {
				// END against END: the expression is complete, the caller's token stays unconsumed
				result = expr_take_result(stk, p);
				goto done;
			}
			// ')' closes a '(' of a group, it is reduced together with it
			if (!expr_push_token(stk, p, input)) {
				goto done;
			}
			paren_depth--;
			expects_operand = false;
			break;
		case EXPR_RELATION_REDUCE:
			// the input is not consumed, it is compared again with the new nearest terminal
			if (!expr_reduce(stk, p)) {
				goto done;
			}
			break;
		case EXPR_RELATION_ERROR:
			print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Unexpected token in expression");
			goto done;
		}
	}

done:
	expr_stack_destroy(stk);
	return result;
}
