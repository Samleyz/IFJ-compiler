/**
 * @file stack_tests.cpp
 * @author Gemini 3.1 Pro & Michal Sobek (xsobekm00vutbr.cz)
 * @brief AI GENERATED TESTS for stack, HUMAN APROOVED
 */

#include <gtest/gtest.h>
#define stack_t my_stack_t
extern "C" {
#include "error.h"
#include "stack.h"
}
#undef stack_t

class StackTest : public ::testing::Test {
  protected:
	my_stack_t stk; // cpp keyword confilict with "stack_t"

	void SetUp() override { stk = stack_init(); }

	void TearDown() override {
		if (stk != nullptr) {
			stack_dtor(stk);
		}
	}
};

TEST_F(StackTest, Initialization) {
	ASSERT_NE(stk, nullptr);
	EXPECT_TRUE(stack_is_empty(stk));
}

TEST_F(StackTest, PushAndTop) {
	int val1 = 42;
	int val2 = 84;

	EXPECT_TRUE(stack_push(stk, &val1));
	EXPECT_FALSE(stack_is_empty(stk));
	EXPECT_EQ(*static_cast<int *>(stack_top(stk)), 42);

	EXPECT_TRUE(stack_push(stk, &val2));
	EXPECT_EQ(*static_cast<int *>(stack_top(stk)), 84);
}

TEST_F(StackTest, Pop) {
	int val = 10;
	stack_push(stk, &val);

	EXPECT_TRUE(stack_pop(stk));
	EXPECT_TRUE(stack_is_empty(stk));

	// pop on a empty stack sould do nothing and return false
	EXPECT_FALSE(stack_pop(stk));
}

TEST_F(StackTest, NullPointers) {
	EXPECT_EXIT(stack_is_empty(nullptr), ::testing::ExitedWithCode(ERR_INTERNAL),
				"Called stack_is_empty on NULL");
	EXPECT_EXIT(stack_push(nullptr, nullptr), ::testing::ExitedWithCode(ERR_INTERNAL),
				"Called stack_push on NULL");
	EXPECT_EXIT(stack_pop(nullptr), ::testing::ExitedWithCode(ERR_INTERNAL),
				"Called stack_pop on NULL");
	EXPECT_FALSE(stack_dtor(nullptr));
}

TEST_F(StackTest, HeapAllocatedString) {
	char *heap_str = strdup("dynamic_string");
	ASSERT_NE(heap_str, nullptr);

	EXPECT_TRUE(stack_push(stk, heap_str));
	EXPECT_FALSE(stack_is_empty(stk));

	char *top_str = static_cast<char *>(stack_top(stk));
	EXPECT_STREQ(top_str, "dynamic_string");

	// data from heap need to be freed before popping
	free(top_str);
	EXPECT_TRUE(stack_pop(stk));
	EXPECT_TRUE(stack_is_empty(stk));
}
