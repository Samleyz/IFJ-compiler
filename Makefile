BUILD_DIR ?= build
CMAKE ?= cmake
CTEST ?= ctest
VALGRIND ?= valgrind

TEST_EXECUTABLES := \
	$(BUILD_DIR)/dynamic_string_tests \
	$(BUILD_DIR)/hashtable_tests \
	$(BUILD_DIR)/stack_tests \
	$(BUILD_DIR)/lexer_tests \
	$(BUILD_DIR)/parse_expr_table_tests \
	$(BUILD_DIR)/parse_expr_stack_tests \
	$(BUILD_DIR)/parse_expr_tests \
	$(BUILD_DIR)/parser_integration_tests

.PHONY: all test valgrind clean

all:
	$(CMAKE) -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON
	$(CMAKE) --build $(BUILD_DIR) --parallel

test: all
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

valgrind: all
	@set -e; \
	for test in $(TEST_EXECUTABLES); do \
		$(VALGRIND) --quiet --leak-check=full --track-origins=yes \
			--show-leak-kinds=definite,indirect,possible \
			--errors-for-leak-kinds=definite,indirect,possible \
			--error-exitcode=1 "$$test"; \
	done

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)
