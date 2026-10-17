/**
 * @file lexer_tests.cpp
 * @author Copilot and Cehlarik Samuel (xcehlas00@vutbr.cz)
 * @brief Specification-focused tests for the lexer.
 */

#include <gtest/gtest.h>

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

extern "C" {
#include "../src/error.h"
#include "../src/lexer.h"
#include "../src/token.h"
#define stack_t lexer_test_stack_t
#include "../src/stack.h"
#undef stack_t
}

namespace {

struct WireToken {
	int32_t type;
	uint32_t line_num;
	int32_t value;
	double double_value;
	uint32_t string_length;
};

struct ObservedToken {
	token_type type;
	unsigned line_num;
	int value;
	double double_value;
	std::string text;
};

struct LexerResult {
	std::vector<ObservedToken> tokens;
	int exit_code = -1;
	bool protocol_error = false;
};

struct ExpectedToken {
	token_type type;
	unsigned line_num = 1;
	int value = 0;
	double double_value = 0.0;
	std::string text;
};

ExpectedToken plain(token_type type, unsigned line = 1) {
	return ExpectedToken{.type = type, .line_num = line};
}

ExpectedToken identifier(const char *text, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_IDENTIFIER, .line_num = line, .text = text};
}

ExpectedToken stringToken(const std::string &text, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_STRING, .line_num = line, .text = text};
}

ExpectedToken integer(int value, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_INTEGER, .line_num = line, .value = value};
}

ExpectedToken doubleToken(double value, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_DOUBLE, .line_num = line, .double_value = value};
}

ExpectedToken keyword(kw_type_t value, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_KEYWORD, .line_num = line, .value = value};
}

ExpectedToken boolean(bool value, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_BOOLEAN, .line_num = line, .value = value};
}

ExpectedToken op(op_type_t value, unsigned line = 1) {
	return ExpectedToken{.type = TOKEN_OPERATOR, .line_num = line, .value = value};
}

bool writeAll(int fd, const void *buffer, size_t size) {
	const char *bytes = static_cast<const char *>(buffer);
	while (size > 0) {
		ssize_t written = write(fd, bytes, size);
		if (written < 0 && errno == EINTR) {
			continue;
		}
		if (written <= 0) {
			return false;
		}
		bytes += written;
		size -= static_cast<size_t>(written);
	}
	return true;
}

bool readExact(int fd, void *buffer, size_t size) {
	char *bytes = static_cast<char *>(buffer);
	size_t received = 0;
	while (received < size) {
		ssize_t count = read(fd, bytes + received, size - received);
		if (count < 0 && errno == EINTR) {
			continue;
		}
		if (count <= 0) {
			return false;
		}
		received += static_cast<size_t>(count);
	}
	return true;
}

LexerResult runLexer(const std::string &source) {
	int descriptors[2];
	if (pipe(descriptors) != 0) {
		throw std::runtime_error("pipe() failed");
	}

	pid_t child = fork();
	if (child < 0) {
		close(descriptors[0]);
		close(descriptors[1]);
		throw std::runtime_error("fork() failed");
	}

	if (child == 0) {
		close(descriptors[0]);
		int null_output = open("/dev/null", O_WRONLY);
		if (null_output >= 0) {
			dup2(null_output, STDERR_FILENO);
			close(null_output);
		}
		FILE *input = tmpfile();
		if (!input) {
			_exit(120);
		}
		if (fwrite(source.data(), 1, source.size(), input) != source.size()) {
			_exit(120);
		}
		rewind(input);
		if (!lexer_init(input)) {
			_exit(120);
		}

		while (true) {
			token current = lexer_get_token();
			WireToken wire{
				.type = static_cast<int32_t>(current.type),
				.line_num = current.line_num,
				.value = 0,
				.double_value = 0.0,
				.string_length = 0,
			};

			if (current.type == TOKEN_IDENTIFIER || current.type == TOKEN_STRING) {
				wire.string_length = static_cast<uint32_t>(strlen(current.attribute.string_val));
			} else if (current.type == TOKEN_INTEGER) {
				wire.value = current.attribute.int_val;
			} else if (current.type == TOKEN_DOUBLE) {
				wire.double_value = current.attribute.double_val;
			} else if (current.type == TOKEN_KEYWORD) {
				wire.value = current.attribute.keyword;
			} else if (current.type == TOKEN_BOOLEAN) {
				wire.value = current.attribute.bool_literal;
			} else if (current.type == TOKEN_OPERATOR) {
				wire.value = current.attribute.operator_type;
			}

			if (!writeAll(descriptors[1], &wire, sizeof(wire)) ||
				(wire.string_length > 0 &&
				 !writeAll(descriptors[1], current.attribute.string_val, wire.string_length))) {
				_exit(121);
			}
			if (wire.string_length > 0) {
				free(current.attribute.string_val);
			}
			if (current.type == TOKEN_EOF) {
				break;
			}
		}
		fclose(input);
		close(descriptors[1]);
		_exit(0);
	}

	close(descriptors[1]);
	LexerResult result;
	while (true) {
		WireToken wire{};
		ssize_t first_read;
		do {
			first_read = read(descriptors[0], &wire, sizeof(wire));
		} while (first_read < 0 && errno == EINTR);
		if (first_read == 0) {
			break;
		}
		if (first_read < 0) {
			result.protocol_error = true;
			break;
		}
		if (static_cast<size_t>(first_read) < sizeof(wire)) {
			size_t already_read = static_cast<size_t>(first_read);
			if (!readExact(descriptors[0], reinterpret_cast<char *>(&wire) + already_read,
						   sizeof(wire) - already_read)) {
				result.protocol_error = true;
				break;
			}
		}

		ObservedToken observed{
			.type = static_cast<token_type>(wire.type),
			.line_num = wire.line_num,
			.value = wire.value,
			.double_value = wire.double_value,
		};
		if (wire.string_length > 1024 * 1024) {
			result.protocol_error = true;
			break;
		}
		if (wire.string_length > 0) {
			observed.text.resize(wire.string_length);
			if (!readExact(descriptors[0], observed.text.data(), wire.string_length)) {
				result.protocol_error = true;
				break;
			}
		}
		result.tokens.push_back(std::move(observed));
	}
	close(descriptors[0]);

	int status = 0;
	pid_t waited;
	do {
		waited = waitpid(child, &status, 0);
	} while (waited < 0 && errno == EINTR);
	if (waited < 0) {
		result.protocol_error = true;
		return result;
	}
	if (WIFEXITED(status)) {
		result.exit_code = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		result.exit_code = 128 + WTERMSIG(status);
	}
	return result;
}

void expectTokens(const std::string &source, const std::vector<ExpectedToken> &expected) {
	LexerResult result = runLexer(source);
	ASSERT_FALSE(result.protocol_error);
	ASSERT_EQ(result.exit_code, 0);
	ASSERT_EQ(result.tokens.size(), expected.size());
	for (size_t i = 0; i < expected.size(); ++i) {
		EXPECT_EQ(result.tokens[i].type, expected[i].type)
			<< "token " << i << " from source: " << source;
		EXPECT_EQ(result.tokens[i].line_num, expected[i].line_num)
			<< "token " << i << " from source: " << source;
		if (expected[i].type == TOKEN_IDENTIFIER || expected[i].type == TOKEN_STRING) {
			EXPECT_EQ(result.tokens[i].text, expected[i].text)
				<< "token " << i << " from source: " << source;
		} else if (expected[i].type == TOKEN_DOUBLE) {
			EXPECT_NEAR(result.tokens[i].double_value, expected[i].double_value, 1e-12)
				<< "token " << i << " from source: " << source;
		} else if (expected[i].type == TOKEN_INTEGER || expected[i].type == TOKEN_KEYWORD ||
				   expected[i].type == TOKEN_BOOLEAN || expected[i].type == TOKEN_OPERATOR) {
			EXPECT_EQ(result.tokens[i].value, expected[i].value)
				<< "token " << i << " from source: " << source;
		}
	}
}

void expectLexicalError(const std::string &source) {
	LexerResult result = runLexer(source);
	EXPECT_FALSE(result.protocol_error);
	EXPECT_EQ(result.exit_code, ERR_LEXICAL) << "source: " << source;
}

} // namespace

TEST(LexerInitialization, RejectsNullInput) { EXPECT_FALSE(lexer_init(nullptr)); }

TEST(LexerEndOfInput, EmptySourceProducesEof) { expectTokens("", {plain(TOKEN_EOF)}); }

TEST(LexerIdentifiers, RecognizesTheCompleteKeywordVocabulary) {
	expectTokens(
		"def do Double else if import Int return String then Unit val var while Boolean case "
		"class extension for List match type",
		{keyword(KW_DEF),	 keyword(KW_DO),		keyword(KW_DOUBLE),	 keyword(KW_ELSE),
		 keyword(KW_IF),	 keyword(KW_IMPORT),	keyword(KW_INT),	 keyword(KW_RETURN),
		 keyword(KW_STRING), keyword(KW_THEN),		keyword(KW_UNIT),	 keyword(KW_VAL),
		 keyword(KW_VAR),	 keyword(KW_WHILE),		keyword(KW_BOOLEAN), keyword(KW_CASE),
		 keyword(KW_CLASS),	 keyword(KW_EXTENSION), keyword(KW_FOR),	 keyword(KW_LIST),
		 keyword(KW_MATCH),	 keyword(KW_TYPE),		plain(TOKEN_EOF)});
}

TEST(LexerIdentifiers, KeywordMatchingIsCaseSensitiveAndRequiresWholeIdentifier) {
	expectTokens("Def Double double iffy Booleanx",
				 {identifier("Def"), keyword(KW_DOUBLE), identifier("double"), identifier("iffy"),
				  identifier("Booleanx"), plain(TOKEN_EOF)});
}

TEST(LexerIdentifiers, AcceptsLettersDigitsAndUnderscores) {
	expectTokens("a A z Z a0 A_1 _name name_ _ __ _1",
				 {identifier("a"), identifier("A"), identifier("z"), identifier("Z"),
				  identifier("a0"), identifier("A_1"), identifier("_name"), identifier("name_"),
				  plain(TOKEN_UNDERSCORE), identifier("__"), identifier("_1"), plain(TOKEN_EOF)});
}

TEST(LexerIdentifiers, RecognizesOnlyExactBooleanLiterals) {
	expectTokens("true false True TRUE falsehood",
				 {boolean(true), boolean(false), identifier("True"), identifier("TRUE"),
				  identifier("falsehood"), plain(TOKEN_EOF)});
}

TEST(LexerIdentifiers, RejectsUnsupportedIdentifierCharacters) {
	for (const std::string &source : {"`name`", "name'"}) {
		expectLexicalError(source);
	}
}

TEST(LexerNumbers, RecognizesZeroAndDecimalIntegers) {
	expectTokens("0 1 42 2147483647",
				 {integer(0), integer(1), integer(42), integer(2147483647), plain(TOKEN_EOF)});
}

TEST(LexerNumbers, RecognizesIntegerDigitSeparators) {
	expectTokens("1_000 12_34 9_8_7 1__2",
				 {integer(1000), integer(1234), integer(987), integer(12), plain(TOKEN_EOF)});
}

TEST(LexerNumbers, RecognizesDecimalAndExponentDoubles) {
	expectTokens(".5 1.0 12.34 1e3 1e+3 1e-3 1.5E-2",
				 {doubleToken(0.5), doubleToken(1.0), doubleToken(12.34), doubleToken(1000.0),
				  doubleToken(1000.0), doubleToken(0.001), doubleToken(0.015), plain(TOKEN_EOF)});
}

TEST(LexerNumbers, RecognizesDoubleSuffixes) {
	expectTokens("12d 1.5D 2e3d",
				 {doubleToken(12.0), doubleToken(1.5), doubleToken(2000.0), plain(TOKEN_EOF)});
}

TEST(LexerNumbers, RecognizesDigitSeparatorsInFractionAndExponent) {
	expectTokens("1_000.2_5 2.5e1_0",
				 {doubleToken(1000.25), doubleToken(2.5e10), plain(TOKEN_EOF)});
}

TEST(LexerNumbers, SignsAreSeparateOperators) {
	expectTokens("-12 +.5", {op(OPERATOR_MINUS), integer(12), op(OPERATOR_PLUS), doubleToken(0.5),
							 plain(TOKEN_EOF)});
}

TEST(LexerNumbers, RejectsLeadingZeroes) {
	for (const std::string &source : {"00", "01", "0123"}) {
		expectLexicalError(source);
	}
}

TEST(LexerNumbers, RejectsIncompleteOrMisplacedSeparators) {
	for (const std::string &source : {"1_", "1._2", "1.2_", "1e_2", "1e2_"}) {
		expectLexicalError(source);
	}
}

TEST(LexerNumbers, RejectsIncompleteDecimalAndExponentParts) {
	for (const std::string &source : {"1.", "1e", "1e+", "1e-", "1.2e", "1.2e+"}) {
		expectLexicalError(source);
	}
}

TEST(LexerNumbers, RejectsDoubleOverflow) { expectLexicalError("1e9999"); }

TEST(LexerNumbers, RejectsUnsupportedBasesAndSuffixes) {
	for (const std::string &source : {"1.0f", "1.5F", "1f", "100L", "0x12", "0b101", "0o77"}) {
		expectLexicalError(source);
	}
}

TEST(LexerNumbers, RejectsMisplacedUnderscoresNearDotAndExp) {
	for (const std::string &source : {"1_.2", "._2", ".2_", "1_e2", "0_123", "0_"}) {
		expectLexicalError(source);
	}
}

TEST(LexerOperators, RecognizesEveryOperatorAndDelimiter) {
	expectTokens("&& || + + - * / ; , : ! != < <= <- > >= = == => | . ( ) [ ]",
				 {op(OPERATOR_AND),
				  op(OPERATOR_OR),
				  op(OPERATOR_PLUS),
				  op(OPERATOR_PLUS),
				  op(OPERATOR_MINUS),
				  op(OPERATOR_STAR),
				  op(OPERATOR_SLASH),
				  op(OPERATOR_SEMICOLON),
				  op(OPERATOR_COMMA),
				  op(OPERATOR_COLON),
				  op(OPERATOR_EX_MARK),
				  op(OPERATOR_NOT_EQUAL),
				  op(OPERATOR_LESS),
				  op(OPERATOR_LESS_EQUAL),
				  op(OPERATOR_LEFT_ARROW),
				  op(OPERATOR_GREATER),
				  op(OPERATOR_GREATER_EQUAL),
				  op(OPERATOR_ASSIGN),
				  op(OPERATOR_EQUAL),
				  op(OPERATOR_RIGHT_DOUBLE_ARROW),
				  op(OPERATOR_VERTICAL_BAR),
				  op(OPERATOR_DOT),
				  plain(TOKEN_LPAREN),
				  plain(TOKEN_RPAREN),
				  plain(TOKEN_LSQUARE),
				  plain(TOKEN_RSQUARE),
				  plain(TOKEN_EOF)});
}

TEST(LexerOperators, RecognizesDotAndProlog) {
	expectTokens("import ifj26.*", {keyword(KW_IMPORT), identifier("ifj26"), op(OPERATOR_DOT),
									op(OPERATOR_STAR), plain(TOKEN_EOF)});
}

TEST(LexerOperators, RecognizesCompoundExpressionsWithoutSpaces) {
	expectTokens("a+b<=c==d!=e=>f<-g++h",
				 {identifier("a"), op(OPERATOR_PLUS), identifier("b"), op(OPERATOR_LESS_EQUAL),
				  identifier("c"), op(OPERATOR_EQUAL), identifier("d"), op(OPERATOR_NOT_EQUAL),
				  identifier("e"), op(OPERATOR_RIGHT_DOUBLE_ARROW), identifier("f"),
				  op(OPERATOR_LEFT_ARROW), identifier("g"), op(OPERATOR_PLUS_PLUS), identifier("h"),
				  plain(TOKEN_EOF)});
}

TEST(LexerOperators, RecognizesListConcatenationOperator) {
	expectTokens("++ + +",
				 {op(OPERATOR_PLUS_PLUS), op(OPERATOR_PLUS), op(OPERATOR_PLUS), plain(TOKEN_EOF)});
}

TEST(LexerOperators, RecognizesMainAnnotation) {
	expectTokens("@main @main()", {plain(TOKEN_MAIN), plain(TOKEN_MAIN), plain(TOKEN_LPAREN),
								   plain(TOKEN_RPAREN), plain(TOKEN_EOF)});
}

TEST(LexerOperators, RejectsMalformedMainAnnotation) {
	for (const std::string &source : {"@", "@Main", "@mai", "@mx"}) {
		expectLexicalError(source);
	}
}

TEST(LexerStrings, RecognizesEmptyAndOrdinaryStrings) {
	expectTokens("\"\" \"plain text\"",
				 {stringToken(""), stringToken("plain text"), plain(TOKEN_EOF)});
}

TEST(LexerStrings, DecodesAllSpecifiedEscapes) {
	expectTokens("\"slash\\\\quote\\\"newline\\nreturn\\rtab\\t\"",
				 {stringToken("slash\\quote\"newline\nreturn\rtab\t"), plain(TOKEN_EOF)});
}

TEST(LexerStrings, StringContentsDoNotStartComments) {
	expectTokens("\"// not a comment /* still text */\"",
				 {stringToken("// not a comment /* still text */"), plain(TOKEN_EOF)});
}

TEST(LexerStrings, RecognizesMultilineTripleQuotedStrings) {
	expectTokens("\"\"\"first line\nsecond line\"\"\"\nnext",
				 {stringToken("first line\nsecond line"), plain(TOKEN_EOL, 2),
				  identifier("next", 3), plain(TOKEN_EOF, 3)});
}

TEST(LexerStrings, PreservesQuotesInsideTripleQuotedStrings) {
	expectTokens("\"\"\"say \"\"hello\"\"\"", {stringToken("say \"\"hello"), plain(TOKEN_EOF)});
}

TEST(LexerStrings, DoesNotInterpretEscapesInTripleQuotedStrings) {
	expectTokens("\"\"\"back\\nslash\\\\\"\"\"",
				 {stringToken("back\\nslash\\\\"), plain(TOKEN_EOF)});
}

TEST(LexerStrings, RecognizesEmptyMultilineString) {
	expectTokens("\"\"\"\"\"\"", {stringToken(""), plain(TOKEN_EOF)});
}

TEST(LexerStrings, GrowsStorageForLongStrings) {
	std::string contents(2048, 'x');
	expectTokens("\"" + contents + "\"", {stringToken(contents), plain(TOKEN_EOF)});
}

TEST(LexerStrings, RejectsInvalidEscapesAndUnterminatedStrings) {
	for (const std::string &source :
		 {"\"\\x\"", "\"\\0\"", "\"unterminated", "\"unterminated\\", "\"line\nbreak\""}) {
		expectLexicalError(source);
	}
}

TEST(LexerStrings, RejectsUnterminatedTripleQuotedStrings) {
	expectLexicalError("\"\"\"unterminated");
}

TEST(LexerComments, SkipsLineCommentsAndKeepsFollowingTokens) {
	expectTokens("first // ignored text\nsecond", {identifier("first"), plain(TOKEN_EOL, 1),
												   identifier("second", 2), plain(TOKEN_EOF, 2)});
}

TEST(LexerComments, SkipsNestedBlockComments) {
	expectTokens("before /* outer /* inner */ outer */ after",
				 {identifier("before"), identifier("after"), plain(TOKEN_EOF)});
}

TEST(LexerComments, TreatsMultilineBlockCommentAsOneLogicalLineEnd) {
	expectTokens(
		"before /* first\n/* nested */\nlast */\nafter",
		{identifier("before"), plain(TOKEN_EOL, 1), identifier("after", 4), plain(TOKEN_EOF, 4)});
}

TEST(LexerComments, CommentMarkersInStringsAreNotComments) {
	expectTokens("\"/* text */ // text\" next",
				 {stringToken("/* text */ // text"), identifier("next"), plain(TOKEN_EOF)});
}

TEST(LexerComments, RejectsUnterminatedBlockComments) {
	for (const std::string &source : {"/*", "/* outer /* inner */", "/* comment\n"}) {
		expectLexicalError(source);
	}
}

TEST(LexerComments, MultilineBlockCommentEndsItsLogicalLine) {
	expectLexicalError("value /* block\ncomment */ next");
}

TEST(LexerComments, SingleLineBlockCommentSeparatesTokensWithoutSpaces) {
	expectTokens("first/*comment*/second",
				 {identifier("first"), identifier("second"), plain(TOKEN_EOF)});
}

TEST(LexerComments, MultilineBlockCommentAllowsTrailingSpaces) {
	expectTokens(
		"first /* block\ncomment */   \nsecond",
		{identifier("first"), plain(TOKEN_EOL, 1), identifier("second", 3), plain(TOKEN_EOF, 3)});
}

TEST(LexerComments, OnlyCommentsProduceEof) {
	expectTokens("// only a line comment\n", {plain(TOKEN_EOF, 2)});
	expectTokens("/* only a\nblock comment */", {plain(TOKEN_EOF, 2)});
}

TEST(LexerLines, EmitsEndOfLineForNonblankLinesOnly) {
	expectTokens("first\n\nsecond\n",
				 {identifier("first"), plain(TOKEN_EOL, 1), identifier("second", 3),
				  plain(TOKEN_EOL, 3), plain(TOKEN_EOF, 4)});
}

TEST(LexerLines, TracksLinesAcrossBlankLinesAndComments) {
	expectTokens(
		"// first line\n\nvalue // trailing comment\nnext",
		{identifier("value", 3), plain(TOKEN_EOL, 3), identifier("next", 4), plain(TOKEN_EOF, 4)});
}

TEST(LexerIndentation, EmitsIndentAndDedentForNestedBlocks) {
	expectTokens("outer\n    middle\n        inner\n    after\nouter_again\n",
				 {identifier("outer"), plain(TOKEN_EOL, 1), plain(TOKEN_INDENT, 2),
				  identifier("middle", 2), plain(TOKEN_EOL, 2), plain(TOKEN_INDENT, 3),
				  identifier("inner", 3), plain(TOKEN_EOL, 3), plain(TOKEN_DEDENT, 4),
				  identifier("after", 4), plain(TOKEN_EOL, 4), plain(TOKEN_DEDENT, 5),
				  identifier("outer_again", 5), plain(TOKEN_EOL, 5), plain(TOKEN_EOF, 6)});
}

TEST(LexerIndentation, RejectsDedentToAnIndentLevelNeverOpened) {
	expectLexicalError("outer\n    inner\n  invalid\n");
}

TEST(LexerIndentation, UsesSpacesAndRejectsLeadingTabsOnNonemptyLines) {
	expectLexicalError("\tvalue");
}

TEST(LexerWhitespace, AllowsTabsBetweenTokens) {
	expectTokens("left\tright", {identifier("left"), identifier("right"), plain(TOKEN_EOF)});
}

TEST(LexerIndentation, ClosesOpenBlocksAtEndOfInput) {
	expectTokens("root\n    child",
				 {identifier("root"), plain(TOKEN_EOL, 1), plain(TOKEN_INDENT, 2),
				  identifier("child", 2), plain(TOKEN_DEDENT, 2), plain(TOKEN_EOF, 2)});
}

TEST(LexerIndentation, EmitsMultipleDedentsInSingleStep) {
	expectTokens("a\n    b\n        c\nd\n",
				 {identifier("a"), plain(TOKEN_EOL, 1), plain(TOKEN_INDENT, 2),
				  identifier("b", 2), plain(TOKEN_EOL, 2), plain(TOKEN_INDENT, 3),
				  identifier("c", 3), plain(TOKEN_EOL, 3), plain(TOKEN_DEDENT, 4),
				  plain(TOKEN_DEDENT, 4), identifier("d", 4), plain(TOKEN_EOL, 4),
				  plain(TOKEN_EOF, 5)});
}

TEST(LexerBrackets, RecognizesParenthesesAndSquareBrackets) {
	expectTokens("f([x])",
				 {identifier("f"), plain(TOKEN_LPAREN), plain(TOKEN_LSQUARE), identifier("x"),
				  plain(TOKEN_RSQUARE), plain(TOKEN_RPAREN), plain(TOKEN_EOF)});
}

TEST(LexerBrackets, AllowsLineBreaksInsideParentheses) {
	expectTokens("f(\n  value\n  )", {identifier("f"), plain(TOKEN_LPAREN), identifier("value", 2),
									  plain(TOKEN_RPAREN, 3), plain(TOKEN_EOF, 3)});
}

TEST(LexerBrackets, RejectsInsufficientIndentationInsideBrackets) {
	expectLexicalError("  foo(\nbar\n  )");
}

TEST(LexerErrors, RejectsCharactersOutsideTheLanguageAlphabet) {
	for (const std::string &source : {"#", "$", "\\", "`", "&", "&&&"}) {
		expectLexicalError(source);
	}
}

TEST(LexerErrors, RejectsOtherUnknownAsciiCharacters) {
	for (const std::string &source : {"?", "~", "^", "%"}) {
		expectLexicalError(source);
	}
}

TEST(LexerErrors, RejectsNonAsciiSourceAndStringCharacters) {
	const std::string non_ascii("\xC3\xA9", 2);
	expectLexicalError(non_ascii);
	expectLexicalError("\"" + non_ascii + "\"");
}
