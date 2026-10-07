/**
 * @file hashtable_tests.cpp
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Test cases for hastable ADT
 */

#include <gtest/gtest.h>
#include <string>

// for compatibility with C code
extern "C" {
#include "../src/hashtable.h"
#include <stdlib.h>
}

class Hashtable_test : public ::testing::Test {
  protected:
	htable_t htab = nullptr;

	static void dtor(void *p) {
		EXPECT_NE(p, nullptr);
		free(p);
	}

	void SetUp() override {
		htab = htable_init(dtor);
		ASSERT_NE(htab, nullptr);
	}

	void TearDown() override {
		if (htab != nullptr) {
			htab = htable_dtor(htab);
		}
	}
};

TEST_F(Hashtable_test, init_and_dtor) { EXPECT_EQ(htable_record_count(htab), 0); }

TEST_F(Hashtable_test, basic_write) {
	void *ptr = malloc(sizeof(int));

	EXPECT_TRUE(htable_write(htab, "testtest", ptr));
	EXPECT_EQ(htable_record_count(htab), 1);
}

TEST_F(Hashtable_test, multiple_write) {
	for (int i = 1; i <= 100; i++) {
		void *ptr = malloc(sizeof(int));
		std::string key = "test" + std::to_string(i);

		EXPECT_TRUE(htable_write(htab, key.c_str(), ptr));
		EXPECT_EQ(htable_record_count(htab), i);
	}
}

TEST_F(Hashtable_test, basic_read_write) {
	void *ptr = malloc(sizeof(int));

	EXPECT_TRUE(htable_write(htab, "testtest", ptr));
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_EQ(htable_read(htab, "testtest"), ptr);
	EXPECT_EQ(htable_record_count(htab), 1);
}

TEST_F(Hashtable_test, failed_read) {
	EXPECT_EQ(htable_read(htab, "testtest"), nullptr);
	EXPECT_EQ(htable_record_count(htab), 0);
}

TEST_F(Hashtable_test, rewrite) {
	void *ptr_1 = malloc(sizeof(int));
	void *ptr_2 = malloc(sizeof(int));
	ASSERT_NE(ptr_1, ptr_2);

	EXPECT_TRUE(htable_write(htab, "testtest", ptr_1));
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_TRUE(htable_write(htab, "testtest", ptr_2));
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_EQ(htable_read(htab, "testtest"), ptr_2);
}

TEST_F(Hashtable_test, basic_remove) {
	void *ptr = malloc(sizeof(int));

	EXPECT_TRUE(htable_write(htab, "testtest", ptr));
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_TRUE(htable_remove(htab, "testtest"));
	EXPECT_EQ(htable_record_count(htab), 0);
	EXPECT_EQ(htable_read(htab, "testtest"), nullptr);
}

TEST_F(Hashtable_test, multiple_write_remove) {
	for (int i = 1; i <= 100; i++) {
		void *ptr = malloc(sizeof(int));
		std::string key = "test" + std::to_string(i);

		EXPECT_TRUE(htable_write(htab, key.c_str(), ptr));
		EXPECT_EQ(htable_record_count(htab), i);
	}
	for (size_t i = 1; i <= 100; i++) {
		std::string key = "test" + std::to_string(i);

		EXPECT_TRUE(htable_remove(htab, key.c_str()));
		EXPECT_EQ(htable_record_count(htab), 100 - i);
	}
}

TEST_F(Hashtable_test, failed_remove) {
	EXPECT_FALSE(htable_remove(htab, "testtest"));
	EXPECT_EQ(htable_record_count(htab), 0);
}

TEST_F(Hashtable_test, empty_read_write_remove) {
	void *ptr = malloc(sizeof(int));

	EXPECT_TRUE(htable_write(htab, "", ptr));
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_EQ(htable_read(htab, ""), ptr);
	EXPECT_EQ(htable_record_count(htab), 1);

	EXPECT_TRUE(htable_remove(htab, ""));
	EXPECT_EQ(htable_record_count(htab), 0);
}

TEST(other, null_destruktor) {
	htable_t tab = htable_init(nullptr);
	ASSERT_NE(tab, nullptr);
	int num = 42;

	EXPECT_TRUE(htable_write(tab, "testtest", &num));
	EXPECT_EQ(htable_record_count(tab), 1);

	EXPECT_TRUE(htable_remove(tab, "testtest"));
	EXPECT_EQ(htable_record_count(tab), 0);

	EXPECT_EQ(htable_read(tab, "testtest"), nullptr);
	htable_dtor(tab);
}
