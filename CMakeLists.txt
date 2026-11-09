cmake_minimum_required(VERSION 3.21)
project(IFJCompilerLexerTests LANGUAGES C CXX)

include(CTest)

if(BUILD_TESTING)
	set(CMAKE_C_STANDARD 23)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	find_package(GTest 1.10 QUIET)
	if(NOT GTest_FOUND)
		include(FetchContent)
		FetchContent_Declare(
			googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG v1.15.2
			GIT_SHALLOW TRUE
		)
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
		FetchContent_MakeAvailable(googletest)
	endif()

	function(add_unit_test target)
		add_executable(${target} ${ARGN})
		target_include_directories(${target} PRIVATE src)
		target_link_libraries(${target} PRIVATE GTest::gtest_main)
		add_test(NAME ${target} COMMAND ${target})
	endfunction()

	add_unit_test(dynamic_string_tests
		tests/dynamic_string_tests.cpp
		src/dynamic_string.c
	)
	add_unit_test(hashtable_tests
		tests/hashtable_tests.cpp
		src/hashtable.c
	)
	add_unit_test(stack_tests
		tests/stack_tests.cpp
		src/stack.c
		src/error.c
	)
	add_unit_test(lexer_tests
		tests/lexer_tests.cpp
		src/dynamic_string.c
		src/error.c
		src/lexer.c
		src/lexer_helpers.c
		src/stack.c
		src/token.c
	)
	set(PARSE_EXPR_SOURCES
		src/parse_expr.c
		src/parser_helpers.c
		src/ast.c
		src/dynamic_string.c
		src/error.c
		src/lexer.c
		src/lexer_helpers.c
		src/stack.c
		src/token.c
	)
	add_unit_test(parse_expr_table_tests
		tests/parse_expr_table_tests.cpp
		${PARSE_EXPR_SOURCES}
		src/parser_grammar.c
	)
	add_unit_test(parse_expr_stack_tests
		tests/parse_expr_stack_tests.cpp
		${PARSE_EXPR_SOURCES}
		src/parser_grammar.c
	)
	# parse_expr_tests does not link parser_grammar.c: it defines its own stand-in for the if rule.
	add_unit_test(parse_expr_tests
		tests/parse_expr_tests.cpp
		${PARSE_EXPR_SOURCES}
	)
endif()
