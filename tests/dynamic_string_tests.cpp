/**
 * @file dynamic_string_tests.cpp
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Test cases for dynamic string
 */

#include <gtest/gtest.h>
#include <string>

// for compatibility with C code
extern "C" {
#include "../src/dynamic_string.h"
#include <stdlib.h>
}

class Dynamic_string_test : public ::testing::Test {
  protected:
	dstring_t dstr = nullptr;

	void SetUp() override {
		dstr = dstring_init();
		ASSERT_NE(dstr, nullptr);
	}

	void TearDown() override { dstring_dtor(dstr); }
};

TEST_F(Dynamic_string_test, init_and_dtor) { EXPECT_EQ(dstring_get_len(dstr), 0); }

TEST_F(Dynamic_string_test, basic_append) {
	EXPECT_TRUE(dstring_append_char(dstr, 'a'));
	EXPECT_EQ(dstring_get_len(dstr), 1);
}

TEST_F(Dynamic_string_test, multiple_append) {
	for (int i = 0; i < 100; i++) {
		EXPECT_TRUE(dstring_append_char(dstr, 'a'));
		EXPECT_EQ(dstring_get_len(dstr), i + 1);
	}
}

TEST_F(Dynamic_string_test, basic_getstring) {
	EXPECT_TRUE(dstring_append_char(dstr, 'a'));

	char *tmp_str = dstring_get_string(dstr);
	EXPECT_STREQ(tmp_str, "a");
	free(tmp_str);
}

TEST_F(Dynamic_string_test, empty_getstring) {
	char *tmp_str = dstring_get_string(dstr);
	EXPECT_STREQ(tmp_str, "");
	free(tmp_str);
}

TEST_F(Dynamic_string_test, long_getstring) {
	for (int i = 0; i < 100; i++) {
		EXPECT_TRUE(dstring_append_char(dstr, 'a'));
	}
	std::string s(100, 'a');
	char *tmp_str = dstring_get_string(dstr);
	EXPECT_STREQ(tmp_str, s.c_str());
	free(tmp_str);
}
