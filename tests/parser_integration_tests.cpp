/**
 * @file parser_integration_tests.cpp
 * @author Claude Code (Claude Sonnet 5.5) and Samuel Cehlarik (xcehlas00@vutbr.cz)
 * @author Claude Code (Claude Sonnet 5.5) and Jan Mejzlík (xmejzlj00@vutbr.cz)
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
			   (node->as.var_def.type_name != nullptr
					? std::string(":") + node->as.var_def.type_name
					: std::string()) +
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
	case NODE_FUNC_CALL: {
		// lists separated with " |"
		std::string text = "(call " + dump(node->as.func_call.callee);
		for (ast_node_t list = node->as.func_call.arg_lists.head; list != nullptr;
			 list = list->next) {
			text += (list == node->as.func_call.arg_lists.head ? "" : " |") +
					dumpList(list->as.arg_list.args);
		}
		return text + ")";
	}
	case NODE_STRING_INTERP:
		return "(interp" + dumpList(node->as.string_interp.parts) + ")";
	case NODE_MEMBER_ACCESS:
		return "(member " + dump(node->as.member_access.target) + " " +
			   node->as.member_access.member_name + ")";
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

// Prefix form of a function definition with its parameter lists and return type, for example
// "(def add @main (a:Int)(b:Int) -> Int (+ a b))". A function without parentheses is "noparens".
std::string dumpDefinition(ast_node_t node) {
	std::string text = std::string("(def ") + node->as.func_def.name;
	if (node->as.func_def.is_main) {
		text += " @main";
	}
	if (!node->as.func_def.has_parens) {
		text += " noparens";
	} else {
		text += " ";
		for (ast_node_t list = node->as.func_def.param_lists.head; list != nullptr;
			 list = list->next) {
			text += "(";
			for (ast_node_t param = list->as.param_list.params.head; param != nullptr;
				 param = param->next) {
				text += std::string(param == list->as.param_list.params.head ? "" : " ") +
						param->as.param.name + ":" + param->as.param.type_name;
			}
			text += ")";
		}
	}
	if (node->as.func_def.return_type != nullptr) {
		text += std::string(" -> ") + node->as.func_def.return_type;
	}
	return text + " " + dump(node->as.func_def.body) + ")";
}

// Prefix form of the top-level definitions of a program (functions and extensions).
std::string dumpDefinitions(ast_node_t root) {
	std::string text = "(program";
	for (ast_node_t item = root->as.program.items.head; item != nullptr; item = item->next) {
		if (item->type == NODE_EXTENSION) {
			text += std::string(" (extension ") + item->as.extension.receiver_name + " " +
					item->as.extension.receiver_type;
			for (ast_node_t method = item->as.extension.methods.head; method != nullptr;
				 method = method->next) {
				text += " " + dumpDefinition(method);
			}
			text += ")";
		} else {
			text += " " + dumpDefinition(item);
		}
	}
	return text + ")";
}

// Result of one run: the tree (empty when the parser reported an error).
struct Outcome {
	bool ok = false;
	std::string tree;
};

// Runs lexer, grammar and expression parser the way main does and cleans up.
Outcome compile(const std::string &body, bool definitions = false) {
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
		outcome.tree = definitions ? dumpDefinitions(root) : dump(root);
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

template <size_t N> void expectTrees(const Case (&cases)[N], bool definitions = false) {
	for (const Case &current : cases) {
		SCOPED_TRACE(current.source);
		Outcome outcome = compile(current.source, definitions);
		EXPECT_TRUE(outcome.ok);
		EXPECT_EQ(outcome.tree, current.tree);
	}
}

#define EXPECT_TREES(cases) expectTrees(cases)
#define EXPECT_TREES_OF_DEFINITIONS(cases) expectTrees(cases, true)

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
		{"def main() =\n    f(1)(2)\n", "(program (def main (block (stmt (call f 1 | 2)))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesFunctionDefinitionsAndParameterLists) {
	const Case cases[] = {
		{"def answer: Int = 42\n", "(program (def answer noparens -> Int 42))"},
		{"def answer = 42\n", "(program (def answer noparens 42))"},
		{"def f(): Int = 1\n", "(program (def f () -> Int 1))"},
		{"def f() = 1\n", "(program (def f () 1))"},
		{"def greet(name: String, times: Int) = name\n",
		 "(program (def greet (name:String times:Int) name))"},
		{"def add(a: Int)(b: Int): Int = a + b\n",
		 "(program (def add (a:Int)(b:Int) -> Int (+ a b)))"},
		{"def f(a: Double)(): Unit = a\n", "(program (def f (a:Double)() -> Unit a))"},
		// a trailing comma is allowed when a line break separates it from the closing parenthesis
		{"def f(a: Int,\n      ): Int = a\n", "(program (def f (a:Int) -> Int a))"},
		{"def f(\n    a: Int,\n    b: Int,\n    ): Int = a\n",
		 "(program (def f (a:Int b:Int) -> Int a))"},
		{"def ok(flag: Boolean): Boolean = flag\n",
		 "(program (def ok (flag:Boolean) -> Boolean flag))"},
		{"@main\ndef main() =\n    return 1\n", "(program (def main @main () (block (return 1))))"},
		{"@main\ndef main(): Unit =\n    val x = 1\n",
		 "(program (def main @main () -> Unit (block (val x 1))))"},
		{"def f() = 1\n\n// a comment\ndef g() = 2 // another one\n\ndef h() = 3\n",
		 "(program (def f () 1) (def g () 2) (def h () 3))"},
	};
	EXPECT_TREES_OF_DEFINITIONS(cases);
}

TEST(FrontEnd, ParsesExtensionDefinitions) {
	const Case cases[] = {
		{"extension (value: Int)\n    def twice: Int = value * 2\n",
		 "(program (extension value Int (def twice noparens -> Int (* value 2))))"},
		{"extension (value: Int)\n"
		 "    def twice: Int = value * 2\n"
		 "    def plus(n: Int): Int = value + n\n"
		 "    def both(a: Int)(b: Int): Int = a + b\n"
		 "    def nothing(): Unit = value\n",
		 "(program (extension value Int (def twice noparens -> Int (* value 2)) "
		 "(def plus (n:Int) -> Int (+ value n)) (def both (a:Int)(b:Int) -> Int (+ a b)) "
		 "(def nothing () -> Unit value)))"},
		{"extension (text: String)\n    def shout: String = text\n\ndef f() = 1\n",
		 "(program (extension text String (def shout noparens -> String text)) (def f () 1))"},
		{"def f() = 1\nextension (x: Double)\n    def half: Double = x / 2\n",
		 "(program (def f () 1) (extension x Double (def half noparens -> Double (/ x 2))))"},
	};
	EXPECT_TREES_OF_DEFINITIONS(cases);
}

TEST(FrontEnd, ParsesTypedVariableDefinitions) {
	const Case cases[] = {
		{"def main() =\n    val a: Int = 1\n    var b: Double = 2\n    val c: String = \"x\"\n"
		 "    var d: Boolean = true\n    val e: Unit = ()\n",
		 "(program (def main (block (val a:Int 1) (var b:Double 2) (val c:String \"x\") "
		 "(var d:Boolean true) (val e:Unit ()))))"},
		{"def main() =\n    val a = 1\n    var b = a\n",
		 "(program (def main (block (val a 1) (var b a))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesLinesAfterNestedBlocksAndSeparatedStatements) {
	// A line directly after a block has no end-of-line token (the block took it), it is still
	// valid.
	const Case cases[] = {
		{"def main() =\n    if a then\n        b = 1\n    c = 2\n",
		 "(program (def main (block (if a (block (= b 1)) -) (= c 2))))"},
		{"def main() =\n    while a do\n        if b then\n            c = 1\n    d = 2\n",
		 "(program (def main (block (while a (block (if b (block (= c 1)) -))) (= d 2))))"},
		{"def main() =\n    while a do\n        b = 1\n    for i <- 1 to 3 do\n        f(i)\n"
		 "    g = 2\n",
		 "(program (def main (block (while a (block (= b 1))) "
		 "(for i to 1 3 (block (stmt (call f i)))) (= g 2))))"},
		{"def main() =\n    val x = if a then\n        1\n    else\n        2\n    print(x)\n",
		 "(program (def main (block (val x (ifexpr a (block (stmt 1)) (block (stmt 2)))) "
		 "(stmt (call print x)))))"},
		{"def main() =\n    if a then\n        b = 1\n    else if c then\n        b = 2\n"
		 "    else\n        b = 3\n    d = 4\n",
		 "(program (def main (block (if a (block (= b 1)) (if c (block (= b 2)) (block (= b 3)))) "
		 "(= d 4))))"},
		// several statements on one line are separated by a semicolon
		{"def main() =\n    a = 1; b = 2\n    if c then\n        d = 1\n    e = 2; f = 3\n",
		 "(program (def main (block (= a 1) (= b 2) (if c (block (= d 1)) -) (= e 2) (= f 3))))"},
		{"def main() =\n    a = 1\n\n    // a comment\n    b = 2 // another one\n",
		 "(program (def main (block (= a 1) (= b 2))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, RejectsStatementsWithoutASeparator) {
	// Statements are separated by a new logical line or by a semicolon. An expression ends at an
	// identifier after an operand (the words to, until and by of a for loop), so these programs
	// must be rejected by the rules that call it.
	const char *sources[] = {
		"def main() =\n    a = 1 b = 2\n",
		"def main() =\n    x y\n",
		"def main() =\n    val x = 1 y\n",
		"def main() =\n    f(1) g(2)\n",
		"def main() =\n    return 1 x\n",
		"def main() =\n    if a then b c\n",
		"def main() =\n    while a do b c\n",
		"def main() =\n    if a then\n        b = 1\n    c = 2 d = 3\n",
		"def f() = 1 x\n",
	};
	for (const char *source : sources) {
		SCOPED_TRACE(source);
		EXPECT_FALSE(compile(source).ok);
	}

	// The same words still end the expressions of a for loop.
	EXPECT_TRUE(compile("def main() =\n    for i <- 1 to 10 by 2 do\n        f(i)\n").ok);
	EXPECT_TRUE(compile("def main() =\n    for i <- 0 until n do\n        f(i)\n").ok);
}

TEST(FrontEnd, RejectsInvalidDefinitions) {
	const char *sources[] = {
		"def f(a Int) = 1\n",					   // parameter without a colon
		"def f(a: Int,) = 1\n",					   // trailing comma on the same line
		"def f() 1\n",							   // no equals sign
		"def = 1\n",							   // no name
		"def f(): = 1\n",						   // return type without a name
		"def f(a: Foo) = 1\n",					   // not a type of the language
		"def f() =\n",							   // no body
		"extension (value: Int)\n",				   // an extension without a method
		"extension (value)\n    def f() = 1\n",	   // receiver without a type
		"extension value: Int\n    def f() = 1\n", // receiver without parentheses
		"def main() =\n    val x: = 1\n",		   // variable type without a name
		"def main() =\n    val = 1\n",			   // variable without a name
		"def main() =\n    val x 1\n",			   // no equals sign
	};
	for (const char *source : sources) {
		SCOPED_TRACE(source);
		EXPECT_FALSE(compile(source).ok);
	}
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

TEST(FrontEnd, ParsesMemberAccessAndMethodCalls) {
	const Case cases[] = {
		{"def main() =\n    val n = \"abc\".length\n",
		 "(program (def main (block (val n (member \"abc\" length)))))"},
		{"def main() =\n    val n = number.floor.toInt\n",
		 "(program (def main (block (val n (member (member number floor) toInt)))))"},
		{"def main() =\n    val s = (1.5).toString\n",
		 "(program (def main (block (val s (member 1.5 toString)))))"},
		{"def main() =\n    val s = 5.toString\n",
		 "(program (def main (block (val s (member 5 toString)))))"},
		{"def main() =\n    val v = -x.toString\n",
		 "(program (def main (block (val v (- (member x toString))))))"},
		{"def main() =\n    val v = a.length + b.length * 2\n",
		 "(program (def main (block (val v (+ (member a length) (* (member b length) 2))))))"},
		{"def main() =\n    val v = x.foo()\n",
		 "(program (def main (block (val v (call (member x foo))))))"},
		{"def main() =\n    val v = x.plus(12)\n",
		 "(program (def main (block (val v (call (member x plus) 12)))))"},
		{"def main() =\n    val v = x.plus(a + 1, b.length)\n",
		 "(program (def main (block (val v (call (member x plus) (+ a 1) (member b length))))))"},
		{"def main() =\n    val v = x.foo(a)(b)\n",
		 "(program (def main (block (val v (call (member x foo) a | b)))))"},
		{"def main() =\n    val v = f(1).x\n",
		 "(program (def main (block (val v (member (call f 1) x)))))"},
		{"def main() =\n    val v = a.b.c(1).d\n",
		 "(program (def main (block (val v (member (call (member (member a b) c) 1) d)))))"},
		{"def main() =\n    foo(a.b, c.d)\n",
		 "(program (def main (block (stmt (call foo (member a b) (member c d))))))"},
		{"def main() =\n    val v = if s.length > 3 then s.length else 0\n",
		 "(program (def main (block (val v (ifexpr (> (member s length) 3) "
		 "(stmt (member s length)) (stmt 0))))))"},
		{"def main() =\n    for i <- 0 until s.length do\n        foo(i)\n",
		 "(program (def main (block (for i until 0 (member s length) "
		 "(block (stmt (call foo i)))))))"},
	};
	EXPECT_TREES(cases);
}

TEST(FrontEnd, ParsesInterpolatedStrings) {
	// Empty text between the parts is dropped, an empty list is the empty string.
	const Case cases[] = {
		{"def main() =\n    val s = s\"hello\"\n",
		 "(program (def main (block (val s (interp \"hello\")))))"},
		{"def main() =\n    val s = s\"$name\"\n",
		 "(program (def main (block (val s (interp name)))))"},
		{"def main() =\n    val s = s\"\"\n", "(program (def main (block (val s (interp)))))"},
		{"def main() =\n    val s = s\"$name${n + 1} costs $$0\"\n",
		 "(program (def main (block (val s (interp name (+ n 1) \" costs $0\")))))"},
		{"def main() =\n    val s = s\"a ${f(1)} z\"\n",
		 "(program (def main (block (val s (interp \"a \" (call f 1) \" z\")))))"},
		{"def main() =\n    val s = s\"x ${f(s\"y\")} z\"\n",
		 "(program (def main (block (val s (interp \"x \" (call f (interp \"y\")) \" z\")))))"},
		{"def main() =\n    val s = s\"${x.length}\"\n",
		 "(program (def main (block (val s (interp (member x length))))))"},
		{"def main() =\n    val s = s\"a\".length\n",
		 "(program (def main (block (val s (member (interp \"a\") length)))))"},
		{"def main() =\n    val s = s\"a\" + s\"b\"\n",
		 "(program (def main (block (val s (+ (interp \"a\") (interp \"b\"))))))"},
		{"def main() =\n    print(s\"v=$v\", w)\n",
		 "(program (def main (block (stmt (call print (interp \"v=\" v) w)))))"},
		{"def main() =\n    return s\"x\"\n",
		 "(program (def main (block (return (interp \"x\")))))"},
		{"def f() = s\"x\"\n", "(program (def f (interp \"x\")))"},
		// a statement and the one-line branches of an if may start with an interpolated string
		{"def main() =\n    s\"x\"\n", "(program (def main (block (stmt (interp \"x\")))))"},
		{"def main() =\n    val s = if a then s\"1\" else s\"2\"\n",
		 "(program (def main (block (val s (ifexpr a (stmt (interp \"1\")) (stmt (interp "
		 "\"2\")))))))"},
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
		{"def main() =\n    val x = -if a then 1 else 2\n",
		 "(program (def main (block (val x (- (ifexpr a (stmt 1) (stmt 2)))))))"},
		{"def main() =\n    val x = (if a then 1 else 2) + 1\n",
		 "(program (def main (block (val x (+ (ifexpr a (stmt 1) (stmt 2)) 1)))))"},
		{"def main() =\n    val x = if a then 1 else 2 + 3\n",
		 "(program (def main (block (val x (ifexpr a (stmt 1) (stmt (+ 2 3)))))))"},
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
		"def main() =\n    val x = a.\n",	 // no member name
		"def main() =\n    val x = a.(b)\n", // a member name is an identifier
		"def main() =\n    val x = a.1\n",	 // a digit after the dot is a number
		"def main() =\n    val x = .a\n",	 // no expression before the dot
		"def main() =\n    val x = a.b(1 2)\n",
		"def main() =\n    val x = s\"${}\"\n",		// an empty expression in an interpolated string
		"def main() =\n    val x = s\"${1 +}\"\n",	// an incomplete expression in it
		"def main() =\n    val x = s\"${1 + 2\"\n", // the expression is not closed // an error
													// inside the arguments
		"def main() =\n    val x = if a then 1\n",
		"def main() =\n    val x = if a 1 else 2\n", // the condition is not a valid expression
		"def main() =\n    val x = if a then 1 else if b then 3\n",
		"def main() =\n    val x = 1 + if a then 1\n",
		"def main() =\n    if a then if b then foo()\n", // a one-line nested if is an expression
	};
	for (const char *source : sources) {
		SCOPED_TRACE(source);
		EXPECT_FALSE(compile(source).ok);
	}
}

// A lexical error does not end the process, the lexer returns TOKEN_ERR and the parser unwinds
// through its syntax error path. Each source fails in the middle of an unfinished construct, so
// valgrind (make valgrind) shows if anything the parser holds in local variables leaks. The bad
// token is two tokens ahead of the parser, because the parser keeps one token of look-ahead.
TEST(FrontEnd, RejectsLexicalErrorsInTheMiddleOfAConstruct) {
	const char *sources[] = {
		"def f = \"x",									   // function name
		"def f(a: \"x",									   // parameter name and list
		"extension(a: \"x",								   // extension receiver
		"def main() =\n    while a do\n        x = \"y\n", // nested blocks, target
		"def main() =\n    val x = \"y",				   // variable definition
		"def main() =\n    for i <- 1 to \"x\n",		   // for loop
		"def main() =\n    if a then \"y\n",			   // if statement
		"def main() =\n    val x = 1 + 2 * \"y",		   // expression stack
		"def main() =\n    f(1, \"y",					   // call arguments
		"def main() =\n    val x = if a then \"y",		   // if expression
		"def main() =\n    val x = 1\n  y\n",			   // invalid indentation
		"def main() =\n    return 1 + \"y",				   // return statement
		"def main() =\n    val x = 1 + $\n",			   // unrecognized character
		"def main() =\n    val x = 01\n",				   // integer starting with zero
		"def main() =\n    val x = \"a\\q\"\n",			   // invalid escape
		"def main() =\n    val x = 1 /* never closed\n",   // unterminated comment
		"\"x",											   // error right after the header
	};
	for (const char *source : sources) {
		SCOPED_TRACE(source);
		EXPECT_FALSE(compile(source).ok);
	}
}

} // namespace
