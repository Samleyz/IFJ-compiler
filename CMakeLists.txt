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

	add_executable(
		lexer_tests
		tests/lexer_tests.cpp
		src/dynamic_string.c
		src/error.c
		src/lexer.c
		src/lexer_helpers.c
		src/stack.c
	)
	target_include_directories(lexer_tests PRIVATE src)
	target_link_libraries(lexer_tests PRIVATE GTest::gtest_main)

	add_test(NAME lexer_tests COMMAND lexer_tests)
endif()
