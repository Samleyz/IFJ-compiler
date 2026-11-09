/**
 * @file parse_expr_stack_tests.cpp
 * @author Claude Code (Claude Sonnet 5.5) and Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief AI GENERATED TESTS for the expression stack helpers, HUMAN APPROVED
 */

#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>

// stack.h declares "stack_t", which clashes with the POSIX type pulled in by gtest in C++.
#define stack_t my_stack_t
extern "C" {
#include "error.h"
#include "parse_expr_private.h"
}
#undef stack_t

namespace {

token operandToken(const char *name) {
	token result{};
	result.type = TOKEN_IDENTIFIER;
	result.attribute.string_val = strdup(name); // owned by the entry it is pushed into
	return result;
}

expr_entry_t *newOperator(expr_symbol_t symbol) {
	return expr_entry_new_terminal(symbol, token{});
}

expr_entry_t *newNode() {
	return expr_entry_new_node(ast_create_node(NODE_LIT_INT, 1));
}

class ExprStackTest : public ::testing::Test {
  protected:
	my_stack_t stk = nullptr;

	void SetUp() override { stk = expr_stack_create(); }
	void TearDown() override { expr_stack_destroy(stk); }

	// Pops the top entry and returns it (the test now owns it).
	expr_entry_t *popTop() {
		expr_entry_t *entry = expr_stack_top(stk);
		stack_pop(stk);
		return entry;
	}
};

TEST_F(ExprStackTest, StartsWithEndTerminalOnly) {
	EXPECT_EQ(expr_stack_nearest_terminal(stk), EXPR_SYMBOL_END);

	expr_entry_t *bottom = popTop();
	EXPECT_EQ(bottom->kind, EXPR_ENTRY_TERMINAL);
	EXPECT_EQ(bottom->as.terminal.symbol, EXPR_SYMBOL_END);
	EXPECT_TRUE(stack_is_empty(stk));
	expr_entry_free(bottom);
	// TearDown destroys an empty stack, which must be fine too.
}

TEST_F(ExprStackTest, NearestTerminalIsTheTopWhenTopIsTerminal) {
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_ADDITIVE));
	EXPECT_EQ(expr_stack_nearest_terminal(stk), EXPR_SYMBOL_ADDITIVE);
}

TEST_F(ExprStackTest, NearestTerminalSkipsNodesAndLeavesStackUnchanged) {
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_ADDITIVE));
	expr_entry_t *node = newNode();
	expr_stack_push(stk, node);

	EXPECT_EQ(expr_stack_nearest_terminal(stk), EXPR_SYMBOL_ADDITIVE);

	// Same entry still on top, then the operator, then END.
	EXPECT_EQ(expr_stack_top(stk), node);
	expr_entry_t *first = popTop();
	expr_entry_t *second = popTop();
	expr_entry_t *third = popTop();
	EXPECT_EQ(first->kind, EXPR_ENTRY_NODE);
	EXPECT_EQ(second->as.terminal.symbol, EXPR_SYMBOL_ADDITIVE);
	EXPECT_EQ(third->as.terminal.symbol, EXPR_SYMBOL_END);
	expr_entry_free(first);
	expr_entry_free(second);
	expr_entry_free(third);
}

TEST_F(ExprStackTest, MarkerGoesAboveNearestTerminalAndBelowTheNode) {
	// $ + [n]  ->  $ + < [n]
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_ADDITIVE));
	expr_stack_push(stk, newNode());

	expr_stack_insert_marker(stk);

	expr_entry_t *first = popTop();
	expr_entry_t *second = popTop();
	expr_entry_t *third = popTop();
	expr_entry_t *fourth = popTop();
	EXPECT_EQ(first->kind, EXPR_ENTRY_NODE);
	EXPECT_EQ(second->kind, EXPR_ENTRY_MARKER);
	EXPECT_EQ(third->as.terminal.symbol, EXPR_SYMBOL_ADDITIVE);
	EXPECT_EQ(fourth->as.terminal.symbol, EXPR_SYMBOL_END);
	for (expr_entry_t *entry : {first, second, third, fourth}) {
		expr_entry_free(entry);
	}
}

TEST_F(ExprStackTest, MarkerOnTopOfTerminalWhenNoNodeIsAbove) {
	expr_stack_insert_marker(stk);

	expr_entry_t *first = popTop();
	expr_entry_t *second = popTop();
	EXPECT_EQ(first->kind, EXPR_ENTRY_MARKER);
	EXPECT_EQ(second->as.terminal.symbol, EXPR_SYMBOL_END);
	expr_entry_free(first);
	expr_entry_free(second);
}

TEST_F(ExprStackTest, PopHandleReturnsEntriesLeftToRightAndRemovesMarker) {
	// $ < [a] * [b]  : handle is [a] * [b]
	expr_stack_insert_marker(stk);
	expr_entry_t *left = newNode();
	expr_entry_t *op = newOperator(EXPR_SYMBOL_MULTIPLICATIVE);
	expr_entry_t *right = newNode();
	expr_stack_push(stk, left);
	expr_stack_push(stk, op);
	expr_stack_push(stk, right);

	expr_entry_t *handle[3];
	size_t count = expr_stack_pop_handle(stk, handle, 3);

	ASSERT_EQ(count, 3u);
	EXPECT_EQ(handle[0], left);
	EXPECT_EQ(handle[1], op);
	EXPECT_EQ(handle[2], right);
	for (size_t i = 0; i < count; i++) {
		expr_entry_free(handle[i]);
	}

	// Marker is gone: only END is left.
	EXPECT_EQ(expr_stack_top(stk)->kind, EXPR_ENTRY_TERMINAL);
	EXPECT_EQ(expr_stack_nearest_terminal(stk), EXPR_SYMBOL_END);
}

TEST_F(ExprStackTest, PopHandleStopsAtTheNearestMarkerOnly) {
	// $ + < [b] * < c   : popping the inner handle leaves "$ + < [b] *"
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_ADDITIVE));
	expr_stack_push(stk, newNode());
	expr_stack_insert_marker(stk); // above '+', below [b]
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_MULTIPLICATIVE));
	expr_stack_insert_marker(stk); // above '*'
	expr_stack_push(stk, expr_entry_new_terminal(EXPR_SYMBOL_OPERAND, operandToken("c")));

	expr_entry_t *handle[4];
	size_t count = expr_stack_pop_handle(stk, handle, 4);

	ASSERT_EQ(count, 1u);
	EXPECT_EQ(handle[0]->as.terminal.symbol, EXPR_SYMBOL_OPERAND);
	expr_entry_free(handle[0]);
	EXPECT_EQ(expr_stack_nearest_terminal(stk), EXPR_SYMBOL_MULTIPLICATIVE);
}

TEST_F(ExprStackTest, DestroyFreesNodesAndTokenStrings) {
	// Nothing to assert directly: leaks here are caught by Valgrind (make valgrind).
	expr_stack_push(stk, expr_entry_new_terminal(EXPR_SYMBOL_OPERAND, operandToken("name")));
	expr_stack_push(stk, newNode());
	expr_stack_insert_marker(stk);
	expr_stack_push(stk, newOperator(EXPR_SYMBOL_ADDITIVE));
}

TEST_F(ExprStackTest, PopHandleWithoutMarkerIsAnInternalError) {
	EXPECT_EXIT(
		{
			expr_entry_t *handle[2];
			expr_stack_pop_handle(stk, handle, 2);
		},
		::testing::ExitedWithCode(ERR_INTERNAL), "No '<' marker");
}

} // namespace
