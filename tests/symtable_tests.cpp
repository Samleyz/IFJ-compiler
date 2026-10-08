/**
 * @file symtable_tests.cpp
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Test cases for symtable ADT
 */

#include <gtest/gtest.h>
#include <string>

// for compatibility with C code
extern "C" {
#include "../src/symtable_private.h"
#include <stdlib.h>
}

class Symtable_test : public ::testing::Test {
  protected:
	stable_t stab = nullptr;

	void SetUp() override {
		stab = stable_init();
		ASSERT_NE(stab, nullptr);
	}

	void TearDown() override {
		if (stab != nullptr) {
			stab = stable_dtor(stab);
		}
	}
};

TEST_F(Symtable_test, init_and_dtor) { EXPECT_EQ(stable_record_count(stab), 0); }

TEST_F(Symtable_test, basic_write) {
	symbol_t s = {.key = (char *)"testtest"};

	EXPECT_TRUE(stable_write(stab, s));
	EXPECT_EQ(stable_record_count(stab), 1);
}

TEST_F(Symtable_test, multiple_write) {
	for (int i = 1; i <= 100; i++) {
		std::string key = "test" + std::to_string(i);
		symbol_t s = {.key = (char *)key.c_str()};

		EXPECT_TRUE(stable_write(stab, s));
		EXPECT_EQ(stable_record_count(stab), i);
	}
}

TEST_F(Symtable_test, basic_read_write) {
	symbol_t s = {.key = (char *)"testtest"};

	EXPECT_TRUE(stable_write(stab, s));
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_STREQ(stable_read(stab, "testtest")->key, "testtest");
	EXPECT_EQ(stable_record_count(stab), 1);
}

TEST_F(Symtable_test, failed_read) {
	EXPECT_EQ(stable_read(stab, "testtest"), nullptr);
	EXPECT_EQ(stable_record_count(stab), 0);
}

TEST_F(Symtable_test, rewrite) {
	symbol_t s1 = {.key = (char *)"test1"};
	symbol_t s2 = {.key = (char *)"test1"};

	EXPECT_TRUE(stable_write(stab, s1));
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_TRUE(stable_write(stab, s2));
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_STREQ(stable_read(stab, "test1")->key, "test1");
	// Later can be added check for other rewriten data (to really check for replacing)
}

TEST_F(Symtable_test, basic_remove) {
	symbol_t s = {.key = (char *)"testtest"};

	EXPECT_TRUE(stable_write(stab, s));
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_TRUE(stable_remove(stab, "testtest"));
	EXPECT_EQ(stable_record_count(stab), 0);
	EXPECT_EQ(stable_read(stab, "testtest"), nullptr);
}

TEST_F(Symtable_test, multiple_write_remove) {
	for (int i = 1; i <= 100; i++) {
		std::string key = "test" + std::to_string(i);
		symbol_t s = {.key = (char *)key.c_str()};

		EXPECT_TRUE(stable_write(stab, s));
		EXPECT_EQ(stable_record_count(stab), i);
	}
	for (size_t i = 1; i <= 100; i++) {
		std::string key = "test" + std::to_string(i);

		EXPECT_TRUE(stable_remove(stab, key.c_str()));
		EXPECT_EQ(stable_record_count(stab), 100 - i);
	}
}

TEST_F(Symtable_test, failed_remove) {
	EXPECT_FALSE(stable_remove(stab, "testtest"));
	EXPECT_EQ(stable_record_count(stab), 0);
}

TEST_F(Symtable_test, empty_read_write_remove) {
	symbol_t s = {.key = (char *)""};

	EXPECT_TRUE(stable_write(stab, s));
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_STREQ(stable_read(stab, "")->key, "");
	EXPECT_EQ(stable_record_count(stab), 1);

	EXPECT_TRUE(stable_remove(stab, ""));
	EXPECT_EQ(stable_record_count(stab), 0);
}
