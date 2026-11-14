/**
 * @file parse_expr_tests.cpp
 * @author Claude Code (Claude Sonnet 5.5) and Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief AI GENERATED TESTS of parse_expression (source text -> lexer -> precedence parser ->
 * AST), HUMAN APPROVED
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

extern "C" {
#include "error.h"
#include "lexer.h"
#include "parse_expr.h"
#include "parser_grammar.h"
#include "parser_private.h"
}

namespace {

struct ParseOutcome {
	bool ok = false;			 // an expression was returned
	std::string tree;			 // the tree in prefix form, e.g. "(+ a (* b c))"
	token_type next_type;		 // type of the token left in the parser (not consumed)
	kw_type_t next_keyword = {}; // keyword of that token, if it is one
	std::string next_identifier; // name of that token, if it is an identifier
};

std::string operatorText(op_type_t op) {
	switch (op) {
	case OPERATOR_PLUS:
		return "+";
	case OPERATOR_MINUS:
		return "-";
	case OPERATOR_STAR:
		return "*";
	case OPERATOR_SLASH:
		return "/";
	case OPERATOR_EX_MARK:
		return "!";
	case OPERATOR_AND:
		return "&&";
	case OPERATOR_OR:
		return "||";
	case OPERATOR_EQUAL:
		return "==";
	case OPERATOR_NOT_EQUAL:
		return "!=";
	case OPERATOR_LESS:
		return "<";
	case OPERATOR_LESS_EQUAL:
		return "<=";
	case OPERATOR_GREATER:
		return ">";
	case OPERATOR_GREATER_EQUAL:
		return ">=";
	default:
		return "?";
	}
}

std::string dump(ast_node_t node) {
	switch (node->type) {
	case NODE_IDENTIFIER:
		return node->as.id.name;
	case NODE_LIT_INT:
		return std::to_string(node->as.int_val);
	case NODE_LIT_DOUBLE: {
		char buffer[32];
		snprintf(buffer, sizeof buffer, "%g", node->as.double_val);
		return buffer;
	}
	case NODE_LIT_STRING:
		return std::string("\"") + node->as.string_val + "\"";
	case NODE_LIT_BOOL:
		return node->as.bool_val ? "true" : "false";
	case NODE_LIT_UNIT:
		return "()";
	case NODE_MEMBER_ACCESS: {
		std::string text = std::string(node->as.member_access.is_call ? "(method " : "(member ") +
						   dump(node->as.member_access.target) + " " +
						   node->as.member_access.member_name;
		for (ast_node_t arg = node->as.member_access.args.head; arg != nullptr; arg = arg->next) {
			text += " " + dump(arg);
		}
		return text + ")";
	}
	case NODE_FUNC_CALL: {
		std::string text = "(call " + dump(node->as.func_call.callee);
		for (ast_node_t arg = node->as.func_call.args.head; arg != nullptr; arg = arg->next) {
			text += " " + dump(arg);
		}
		return text + ")";
	}
	case NODE_UNARY_OP:
		return "(" + operatorText(node->as.unary_op.operator_type) + " " +
			   dump(node->as.unary_op.operand) + ")";
	case NODE_BINARY_OP:
		return "(" + operatorText(node->as.binary_op.operator_type) + " " +
			   dump(node->as.binary_op.left) + " " + dump(node->as.binary_op.right) + ")";
	default:
		return "?";
	}
}

// Runs the lexer and parse_expression on the source (a trailing newline is added) and cleans up.
ParseOutcome parse(const std::string &source) {
	ParseOutcome outcome;
	std::string text = source + "\n";

	FILE *input = tmpfile();
	EXPECT_NE(input, nullptr);
	fwrite(text.data(), 1, text.size(), input);
	rewind(input);
	EXPECT_TRUE(lexer_init(input));

	parser_t parser{};
	parser_advance(&parser);
	parser_advance(&parser);

	ast_node_t root = parse_expression(&parser);
	if (root != nullptr) {
		outcome.ok = true;
		outcome.tree = dump(root);
		ast_dtor(root);
	}
	outcome.next_type = parser.curr_tkn.type;
	if (parser.curr_tkn.type == TOKEN_KEYWORD) {
		outcome.next_keyword = parser.curr_tkn.attribute.keyword;
	}
	if (parser.curr_tkn.type == TOKEN_IDENTIFIER) {
		outcome.next_identifier = parser.curr_tkn.attribute.string_val;
	}

	free_token_attr(&parser.curr_tkn);
	free_token_attr(&parser.look_ahead_tkn);
	lexer_cleanup();
	fclose(input);
	return outcome;
}

struct TreeCase {
	const char *source;
	const char *tree;
};

TEST(ParseExpression, BuildsTreesWithCorrectPrecedenceAndAssociativity) {
	const TreeCase cases[] = {
		{"x", "x"},
		{"1 + 2", "(+ 1 2)"},
		{"a + b * c", "(+ a (* b c))"},
		{"a * b + c", "(+ (* a b) c)"},
		{"(a + b) * c", "(* (+ a b) c)"},
		{"2 * (3 + 4) - 5", "(- (* 2 (+ 3 4)) 5)"},
		{"a - b - c", "(- (- a b) c)"},
		{"a / b * c", "(* (/ a b) c)"},
		{"-a * b", "(* (- a) b)"},
		{"- - a", "(- (- a))"},
		{"a * -b", "(* a (- b))"},
		{"!a && b", "(&& (! a) b)"},
		{"a || b && c", "(|| a (&& b c))"},
		{"a && b && c", "(&& (&& a b) c)"},
		{"a < b == c", "(== (< a b) c)"},
		{"a + b < c", "(< (+ a b) c)"},
		{"a == b && c != d", "(&& (== a b) (!= c d))"},
		{"true && false", "(&& true false)"},
		{"\"s\" + t", "(+ \"s\" t)"},
		{"((a))", "a"},
		{"()", "()"},
		{"to + 1", "(+ to 1)"},
		{"1 + until", "(+ 1 until)"},
		{"f()", "(call f)"},
		{"f(a)", "(call f a)"},
		{"f(1, 2, 3)", "(call f 1 2 3)"},
		{"f(a + 1, b)", "(call f (+ a 1) b)"},
		{"f(g(x), h())", "(call f (call g x) (call h))"},
		{"f(a)(b)", "(call (call f a) b)"},
		{"a.length", "(member a length)"},
		{"a.b.c", "(member (member a b) c)"},
		{"\"abc\".length", "(member \"abc\" length)"},
		{"(1.5).toString", "(member 1.5 toString)"},
		{"5.toString", "(member 5 toString)"},
		{"(a + b).length", "(member (+ a b) length)"},
		{"-a.b", "(- (member a b))"},
		{"!a.b && c", "(&& (! (member a b)) c)"},
		{"a.b + c.d * 2", "(+ (member a b) (* (member c d) 2))"},
		{"a . b", "(member a b)"},
		{"a.foo()", "(method a foo)"},
		{"a.plus(12)", "(method a plus 12)"},
		{"a.plus(b + 1, c.d)", "(method a plus (+ b 1) (member c d))"},
		{"a.foo(x)(y)", "(call (method a foo x) y)"},
		{"f(x).y", "(member (call f x) y)"},
		{"a.b(c).d(e)", "(method (method a b c) d e)"},
		{"f(a.b, c.d)", "(call f (member a b) (member c d))"},
		{"add(x)(y)(1)", "(call (call (call add x) y) 1)"},
		{"(f)(x)", "(call f x)"},
		{"a + f(b) * c", "(+ a (* (call f b) c))"},
		{"2 * f(x) + 1", "(+ (* 2 (call f x)) 1)"},
		{"-f(x)", "(- (call f x))"},
		{"f(x) < g(y)", "(< (call f x) (call g y))"},
		{"f(a,\n  b,\n  )", "(call f a b)"},
		{"1.5 + 2.25", "(+ 1.5 2.25)"},
		{"0.5 * 4", "(* 0.5 4)"},
		{"-2.5", "(- 2.5)"},
		{"1e3", "1000"},
		{"3.14 * r * r", "(* (* 3.14 r) r)"},
		{"a - -b", "(- a (- b))"},
		{"a-b", "(- a b)"},
		{"+a", "(+ a)"},
		{"!a == b", "(== (! a) b)"},
		{"(a +\n  b)", "(+ a b)"},
		{"f(a,\n  b)", "(call f a b)"},
	};
	for (const TreeCase &c : cases) {
		ParseOutcome outcome = parse(c.source);
		EXPECT_TRUE(outcome.ok) << c.source;
		EXPECT_EQ(outcome.tree, c.tree) << c.source;
	}
}

TEST(ParseExpression, RejectsInvalidExpressions) {
	const char *sources[] = {
		"a < b < c",   // relational operators are not associative
		"a == b != c", // neither are equality operators
		"1 2",		   // two operands in a row (two literals)
		"* 3",		   // operator without a left operand
		"a +",		   // operator without a right operand
		"a + * b",	   // operator after an operator
		"a !b",		   // prefix operator after an operand
		"a.",		   // no member name
		"a.(b)",	   // a member name is an identifier
		"a.1",		   // a digit after the dot is a number, not a member
		".a",		   // nothing before the dot
		"a.b(1 2)",	   // an error inside the arguments of a method
		"f(a,)",	   // trailing comma on the same line as ")"
		"f(,a)",	   // missing argument
		"f(a b)",	   // two operands in one argument
		"f(a +)",	   // incomplete argument
		// An error in the middle of a call: everything built so far must be freed (Valgrind).
		"f(a, b +)",
		"f(a, b, c +)",
		"f(g(x, ), y)",
		"f(g(x +), y)",
		"a * f(b, c +)",
		"f(1)(2, 3 +)",
		"-f(x +)",
		"\"s\" + f(x, )",
		"x + y * f(z, w, )",
		// An unfinished expression cannot continue on the next line.
		"a +\nb",
		"a +\n  b",
	};
	for (const char *source : sources) {
		EXPECT_FALSE(parse(source).ok) << source;
	}
}

TEST(ParseExpression, LeavesTheTokenThatEndsTheExpressionUnconsumed) {
	ParseOutcome then_case = parse("a + b then");
	ASSERT_TRUE(then_case.ok);
	EXPECT_EQ(then_case.tree, "(+ a b)");
	EXPECT_EQ(then_case.next_type, TOKEN_KEYWORD);
	EXPECT_EQ(then_case.next_keyword, KW_THEN);

	ParseOutcome comma_case = parse("a, b");
	ASSERT_TRUE(comma_case.ok);
	EXPECT_EQ(comma_case.tree, "a");
	EXPECT_EQ(comma_case.next_type, TOKEN_OPERATOR);

	ParseOutcome eol_case = parse("a * b");
	ASSERT_TRUE(eol_case.ok);
	EXPECT_EQ(eol_case.next_type, TOKEN_EOL);
}

TEST(ParseExpression, AnIdentifierRightAfterAnOperandEndsTheExpression) {
	// "to", "until" and "by" are identifiers, the for rule of the parser checks for them.
	struct Case {
		const char *source;
		const char *tree;
		const char *next;
	};
	const Case cases[] = {
		{"1 to 10 by 2", "1", "to"},
		{"0 until limit", "0", "until"},
		{"a + b by 2", "(+ a b)", "by"},
		{"f(x) to 3", "(call f x)", "to"},
		{"(a) to 3", "a", "to"},
		{"-1 to 3", "(- 1)", "to"},
		{"a b", "a", "b"},
	};
	for (const Case &c : cases) {
		ParseOutcome outcome = parse(c.source);
		ASSERT_TRUE(outcome.ok) << c.source;
		EXPECT_EQ(outcome.tree, c.tree) << c.source;
		EXPECT_EQ(outcome.next_type, TOKEN_IDENTIFIER) << c.source;
		EXPECT_EQ(outcome.next_identifier, c.next) << c.source;
	}
}

TEST(ParseExpression, AGeneratorRangeIsParsedPieceByPiece) {
	// What the for rule does with "for i <- 1 to 10 by 2 do": parse_expression, check the word,
	// parse_expression again, and so on. Here the three pieces come from separate calls.
	EXPECT_EQ(parse("1 to 10 by 2").tree, "1");
	EXPECT_EQ(parse("10 by 2 do").tree, "10");
	ParseOutcome step = parse("2 do");
	EXPECT_EQ(step.tree, "2");
	EXPECT_EQ(step.next_type, TOKEN_KEYWORD);
	EXPECT_EQ(step.next_keyword, KW_DO);
}

TEST(ParseExpression, HandlesDeepNesting) {
	const int depth = 100;

	// ((((...a...)))) is just a
	EXPECT_EQ(parse(std::string(depth, '(') + "a" + std::string(depth, ')')).tree, "a");

	// - - - ... a: prefix operators are right-associative, so the stack grows
	std::string prefix_source;
	std::string prefix_tree = "a";
	for (int i = 0; i < depth / 2; i++) {
		prefix_source += "- ";
		prefix_tree = "(- " + prefix_tree + ")";
	}
	EXPECT_EQ(parse(prefix_source + "a").tree, prefix_tree);

	// a + a + a ...: left-associative, so each addition nests the previous tree
	std::string chain_source = "a";
	std::string chain_tree = "a";
	for (int i = 0; i < 2 * depth; i++) {
		chain_source += " + a";
		chain_tree = "(+ " + chain_tree + " a)";
	}
	EXPECT_EQ(parse(chain_source).tree, chain_tree);

	// f(f(f(...x...))): the parser recurses for every argument
	std::string call_source = "x";
	std::string call_tree = "x";
	for (int i = 0; i < depth / 2; i++) {
		call_source = "f(" + call_source + ")";
		call_tree = "(call f " + call_tree + ")";
	}
	EXPECT_EQ(parse(call_source).tree, call_tree);
}

TEST(ParseExpression, CallEndsAtTheNextTokenOfTheCaller) {
	ParseOutcome outcome = parse("f(x, y) then");
	ASSERT_TRUE(outcome.ok);
	EXPECT_EQ(outcome.tree, "(call f x y)");
	EXPECT_EQ(outcome.next_type, TOKEN_KEYWORD);
	EXPECT_EQ(outcome.next_keyword, KW_THEN);
}

} // namespace
