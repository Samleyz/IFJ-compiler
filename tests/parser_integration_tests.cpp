/**
 * @file parser_integration_tests.cpp
 * @author Claude Code (Claude Sonnet 5.5) and Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @brief AI GENERATED TESTS of the whole front end (source text -> lexer -> grammar rules ->
 * precedence parser -> AST) with the real grammar, HUMAN APPROVED
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

extern "C" {
#include "ast.h"
#include "lexer.h"
#include "parser.h"
}

namespace {

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

std::string dump(ast_node_t node);

std::string dumpList(const ast_list_t &list) {
	std::string result;
	for (ast_node_t item = list.head; item != nullptr; item = item->next) {
		result += " " + dump(item);
	}
	return result;
}

// Prefix form of the tree, e.g. "(+ a (* b c))". A missing else branch is written as "-".
std::string dump(ast_node_t node) {
	if (node == nullptr) {
		return "-";
	}
	switch (node->type) {
	case NODE_PROGRAM:
		return "(program" + dumpList(node->as.program.items) + ")";
	case NODE_BLOCK:
		return "(block" + dumpList(node->as.block.statements) + ")";
	case NODE_FUNC_DEF:
		return std::string("(def ") + node->as.func_def.name + " " + dump(node->as.func_def.body) +
			   ")";
	case NODE_VAR_DEF:
		return std::string(node->as.var_def.is_val ? "(val " : "(var ") + node->as.var_def.name +
			   " " + dump(node->as.var_def.init_expr) + ")";
	case NODE_ASSIGN:
		return std::string("(= ") + node->as.assign.target_id + " " + dump(node->as.assign.expr) +
			   ")";
	case NODE_IF:
		return std::string(node->as.if_stmt.is_expr ? "(ifexpr " : "(if ") +
			   dump(node->as.if_stmt.condition) + " " + dump(node->as.if_stmt.then_branch) + " " +
			   dump(node->as.if_stmt.else_branch) + ")";
	case NODE_WHILE:
		return "(while " + dump(node->as.while_stmt.condition) + " " +
			   dump(node->as.while_stmt.body) + ")";
	case NODE_FOR: {
		std::string result = std::string("(for ") + node->as.for_stmt.var_name +
							 (node->as.for_stmt.is_inclusive ? " to " : " until ") +
							 dump(node->as.for_stmt.from_expr) + " " +
							 dump(node->as.for_stmt.to_expr);
		if (node->as.for_stmt.step_expr != nullptr) {
			result += " by " + dump(node->as.for_stmt.step_expr);
		}
		for (ast_node_t guard = node->as.for_stmt.guards.head; guard != nullptr;
			 guard = guard->next) {
			result += " if " + dump(guard);
		}
		return result + " " + dump(node->as.for_stmt.body) + ")";
	}
	case NODE_RETURN:
		return node->as.return_stmt.expr == nullptr
				   ? "(return)"
				   : "(return " + dump(node->as.return_stmt.expr) + ")";
	case NODE_EXPR_STMT:
		return "(stmt " + dump(node->as.expr_stmt.expr) + ")";
	case NODE_FUNC_CALL:
		return "(call " + dump(node->as.func_call.callee) + dumpList(node->as.func_call.args) + ")";
	case NODE_BINARY_OP:
		return "(" + operatorText(node->as.binary_op.operator_type) + " " +
			   dump(node->as.binary_op.left) + " " + dump(node->as.binary_op.right) + ")";
	case NODE_UNARY_OP:
		return "(" + operatorText(node->as.unary_op.operator_type) + " " +
			   dump(node->as.unary_op.operand) + ")";
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
	default:
		return "?";
	}
}

// Result of one run: the tree (empty when the parser reported an error).
struct Outcome {
	bool ok = false;
	std::string tree;
};

// Runs lexer, grammar and expression parser the way main does and cleans up.
Outcome compile(const std::string &body) {
	Outcome outcome;
	std::string text = "import ifj26.*\n" + body;

	FILE *input = tmpfile();
	EXPECT_NE(input, nullptr);
	fwrite(text.data(), 1, text.size(), input);
	rewind(input);
	EXPECT_TRUE(lexer_init(input));
	parser_init();

	ast_node_t root = parse_program(); // owned by the parser, freed by parser_cleanup
	if (root != nullptr) {
		outcome.ok = true;
		outcome.tree = dump(root);
	}

	parser_cleanup();
	lexer_cleanup();
	fclose(input);
	return outcome;
}

struct Case {
	const char *source;
	const char *tree;
};

template <size_t N> void expectTrees(const Case (&cases)[N]) {
	for (const Case &current : cases) {
		SCOPED_TRACE(current.source);
		Outcome outcome = compile(current.source);
		EXPECT_TRUE(outcome.ok);
		EXPECT_EQ(outcome.tree, current.tree);
	}
}

#define EXPECT_TREES(cases) expectTrees(cases)

TEST(FrontEnd, ParsesFunctionsAndStatements) {
	const Case cases[] = {
		{"def f(a: Int, b: Int): Int = a + b * 2\n", "(program (def f (+ a (* b 2))))"},
		{"def f() = 1\ndef g() = 2\n", "(program (def f 1) (def g 2))"},
		{"@main\ndef main() =\n"
		 "    val x = 1 + 2 * 3\n"
		 "    var y = x\n"
		 "    y = y - 1\n"
		 "    return y\n",
		 "(program (def main (block (val x (+ 1 (* 2 3))) (var y x) (= y (- y 1)) (return y))))"},
		{"def main() =\n    val s = \"hi\"\n    val d = 1.5\n    val u = ()\n",
		 "(program (def main (block (val s \"hi\") (val d 1.5) (val u ()))))"},
		{"def main() =\n    print(f(1, 2) + 3)\n",
		 "(program (def main (block (stmt (call print (+ (call f 1 2) 3))))))"},
		{"def main() =\n    f(1)(2)\n", "(program (def main (block (stmt (call (call f 1) 2)))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesOperatorPrecedenceInsideStatements) {
	const Case cases[] = {
		{"def main() =\n    val b = a < 1 && !(c == 2) || d\n",
		 "(program (def main (block (val b (|| (&& (< a 1) (! (== c 2))) d)))))"},
		{"def main() =\n    val v = (a + b) * c - d / e\n",
		 "(program (def main (block (val v (- (* (+ a b) c) (/ d e))))))"},
		{"def main() =\n    val v = -a * -b\n",
		 "(program (def main (block (val v (* (- a) (- b))))))"},
		{"def main() =\n    x = a - b - c\n", "(program (def main (block (= x (- (- a b) c)))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesConditionalStatementsAndLoops) {
	const Case cases[] = {
		{"def main() =\n    if x < 3 then\n        y = 1\n    else\n        y = 2\n",
		 "(program (def main (block (if (< x 3) (block (= y 1)) (block (= y 2))))))"},
		{"def main() =\n    if x < 0 then\n        a = 1\n    else if x == 0 then\n"
		 "        a = 2\n    else\n        a = 3\n",
		 "(program (def main (block (if (< x 0) (block (= a 1)) "
		 "(if (== x 0) (block (= a 2)) (block (= a 3)))))))"},
		{"def main() =\n    if x then y = 1\n", "(program (def main (block (if x (= y 1) -))))"},
		{"def main() =\n    while i < 10 do\n        i = i + 1\n",
		 "(program (def main (block (while (< i 10) (block (= i (+ i 1)))))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesForLoopsWithRangesAndGuards) {
	const Case cases[] = {
		{"def main() =\n    for i <- 1 to n do\n        foo(i)\n",
		 "(program (def main (block (for i to 1 n (block (stmt (call foo i)))))))"},
		{"def main() =\n    for i <- 0 until n + 1 do\n        foo(i)\n",
		 "(program (def main (block (for i until 0 (+ n 1) (block (stmt (call foo i)))))))"},
		{"def main() =\n    for i <- 0 to n by 2 do\n        foo(i)\n",
		 "(program (def main (block (for i to 0 n by 2 (block (stmt (call foo i)))))))"},
		{"def main() =\n    for i <- 1 to n if i > 2 if i < 9 do\n        foo(i)\n",
		 "(program (def main (block (for i to 1 n if (> i 2) if (< i 9) "
		 "(block (stmt (call foo i)))))))"},
		{"def main() =\n    for\n        i <- 0 until n\n        if i != 3\n        if i < 8\n"
		 "    do\n        foo(i)\n",
		 "(program (def main (block (for i until 0 n if (!= i 3) if (< i 8) "
		 "(block (stmt (call foo i)))))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesIfExpressionsWithTheRealGrammar) {
	// Branches of an if expression are NODE_EXPR_STMT (see the comment of if_stmt in ast.h).
	const Case cases[] = {
		{"def main() =\n    val x = if a then 1 else 2\n",
		 "(program (def main (block (val x (ifexpr a (stmt 1) (stmt 2))))))"},
		{"def main() =\n    val x = if a < 0 then 0 - a else a\n",
		 "(program (def main (block (val x (ifexpr (< a 0) (stmt (- 0 a)) (stmt a))))))"},
		{"def main() =\n    val x = 1 + if a then 1 else 2\n",
		 "(program (def main (block (val x (+ 1 (ifexpr a (stmt 1) (stmt 2)))))))"},
		{"def main() =\n    foo(if a then 1 else 2, 3)\n",
		 "(program (def main (block (stmt (call foo (ifexpr a (stmt 1) (stmt 2)) 3)))))"},
		{"def main() =\n    return if a then 1 else 2\n",
		 "(program (def main (block (return (ifexpr a (stmt 1) (stmt 2))))))"},
		{"def f(a: Int): Int = if a < 1 then 1 else 2\n",
		 "(program (def f (ifexpr (< a 1) (stmt 1) (stmt 2))))"},
		// the layouts shown in the specification (CONTROL)
		{"def main() =\n    val abs2 = if x < 0 then\n        0 - x\n    else\n        x\n",
		 "(program (def main (block (val abs2 (ifexpr (< x 0) (block (stmt (- 0 x))) "
		 "(block (stmt x)))))))"},
		{"def main() =\n    val abs3 = if x < 0 then 0 - x else\n        x\n",
		 "(program (def main (block (val abs3 (ifexpr (< x 0) (stmt (- 0 x)) "
		 "(block (stmt x)))))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, MarksEveryLinkOfAnElseIfChainAsAnExpression) {
	const Case cases[] = {
		{"def main() =\n    val x = if a then 1 else if b then 3 else 4\n",
		 "(program (def main (block (val x (ifexpr a (stmt 1) (ifexpr b (stmt 3) (stmt 4)))))))"},
		{"def main() =\n    val x = if a then 1 else if b then 2 else if c then 3 else 4\n",
		 "(program (def main (block (val x (ifexpr a (stmt 1) (ifexpr b (stmt 2) "
		 "(ifexpr c (stmt 3) (stmt 4))))))))"},
		{"def main() =\n    val v = if x < 0 then\n        1\n    else if x == 0 then\n"
		 "        2\n    else\n        3\n",
		 "(program (def main (block (val v (ifexpr (< x 0) (block (stmt 1)) "
		 "(ifexpr (== x 0) (block (stmt 2)) (block (stmt 3))))))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesAnIfExpressionAsAOneLineBranchOfAnotherIf) {
	const Case cases[] = {
		{"def main() =\n    val x = if a then if b then 1 else 2 else 3\n",
		 "(program (def main (block (val x (ifexpr a (stmt (ifexpr b (stmt 1) (stmt 2))) "
		 "(stmt 3))))))"},
		{"def main() =\n    if a then\n        if b then foo()\n",
		 "(program (def main (block (if a (block (if b (stmt (call foo)) -)) -))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, RejectsInvalidExpressions) {
	const char *sources[] = {
		"def main() =\n    val x = 1 +\n",
		"def main() =\n    val x = 1 < 2 < 3\n",
		"def main() =\n    val x = 1 2\n",
		"def main() =\n    val x = f(1 2)\n",
		"def main() =\n    val x = if a then 1\n",
		"def main() =\n    val x = if a then 1 else if b then 3\n",
		"def main() =\n    val x = 1 + if a then 1\n",
		"def main() =\n    if a then if b then foo()\n", // a one-line nested if is an expression
	};
	for (const char *source : sources) {
		SCOPED_TRACE(source);
		EXPECT_FALSE(compile(source).ok);
	}
}

} // namespace
