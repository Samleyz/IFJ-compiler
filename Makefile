BUILD_DIR ?= build
CMAKE ?= cmake
CTEST ?= ctest
VALGRIND ?= valgrind
CC := gcc
LD := ld
CFLAGS := -std=c23 -Wall -Wextra -Wjump-misses-init
#disable jump over declaration in lexer switch
lexer.o: CFLAGS += -Wno-jump-misses-init
semantic_analysis.o: CFLAGS += -Wno-jump-misses-init
TARGETS := \
	compiler \
	semantic_analysis.o \
	main.o compiler.o \
	symtable.o ast.o stack.o dynamic_string.o hashtable.o error.o \
	lexer.o lexer_helpers.o token.o lexer_final.o \
	parser.o parser_grammar.o parser_helpers.o parse_expr.o parse_expr_helpers.o parser_final.o \
	code_gen.o code_gen_expr.o code_gen_stmt.o code_gen_builtins.o instruction_set.o code_gen_final.o
HEADERS := \
	src/ast.h \
	src/code_gen.h \
	src/code_gen_private.h \
	src/dynamic_string.h \
	src/dynamic_string_private.h \
	src/error.h \
	src/hashtable.h \
	src/hashtable_private.h \
	src/instruction_set.h \
	src/lexer.h \
	src/lexer_private.h \
	src/parse_expr.h \
	src/parse_expr_private.h \
	src/parser.h \
	src/parser_grammar.h \
	src/parser_private.h \
	src/stack.h \
	src/stack_private.h \
	src/token.h \
	src/semantic_analysis.h \
	src/semantic_analysis_private.h \
	src/symtable.h \
	src/symtable_private.h

TEST_EXECUTABLES := \
	$(BUILD_DIR)/dynamic_string_tests \
	$(BUILD_DIR)/hashtable_tests \
	$(BUILD_DIR)/stack_tests \
	$(BUILD_DIR)/lexer_tests \
	$(BUILD_DIR)/parse_expr_table_tests \
	$(BUILD_DIR)/parse_expr_stack_tests \
	$(BUILD_DIR)/parse_expr_tests \
	$(BUILD_DIR)/parser_integration_tests

.PHONY: build unit-tests test valgrind e2e clean

build: compiler

compiler: ast.o code_gen_final.o dynamic_string.o hashtable.o error.o lexer_final.o stack.o parser_final.o compiler.o semantic_analysis.o symtable.o

#main
compiler.o: main.o
	$(LD) -r $^ -o $@

#main modules
lexer_final.o: lexer.o lexer_helpers.o token.o
	$(LD) -r $^ -o $@

parser_final.o: parser.o parser_grammar.o parser_helpers.o parse_expr.o parse_expr_helpers.o
	$(LD) -r $^ -o $@

semantic_analysis.o : src/semantic_analysis.c src/semantic_analysis.h src/semantic_analysis_private.h src/symtable.h
	$(CC) -c $< -o $@ $(CFLAGS)

code_gen_final.o: code_gen.o code_gen_expr.o code_gen_stmt.o code_gen_builtins.o instruction_set.o
	$(LD) -r $^ -o $@

#data structures
ast.o: src/ast.c src/ast.h src/token.h src/error.h

stack.o: src/stack.c src/stack.h src/stack_private.h src/error.h

dynamic_string.o: src/dynamic_string.c src/dynamic_string.h src/dynamic_string_private.h
	$(CC) -c $< -o $@ $(CFLAGS)

hashtable.o: src/hashtable.c src/hashtable.h src/hashtable_private.h
	$(CC) -c $< -o $@ $(CFLAGS)

#helper modules
error.o: src/error.c src/error.h
	$(CC) -c $< -o $@ $(CFLAGS)

#general rule for all .c
%.o: src/%.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

unit-tests:
	$(CMAKE) -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON
	$(CMAKE) --build $(BUILD_DIR) --parallel

test: unit-tests
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

valgrind: unit-tests
	@set -e; \
	for test in $(TEST_EXECUTABLES); do \
		$(VALGRIND) --quiet --leak-check=full --track-origins=yes \
			--show-leak-kinds=definite,indirect,possible \
			--errors-for-leak-kinds=definite,indirect,possible \
			--error-exitcode=1 "$$test"; \
	done

# end-to-end tests from the specification (see README.md)
e2e:
	tests/e2e/run.sh

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)
	rm -f $(TARGETS)
