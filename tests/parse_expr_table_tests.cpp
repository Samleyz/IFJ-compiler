#include <gtest/gtest.h>

extern "C" {
#include "parse_expr_private.h"
}

namespace {

token operatorToken(op_type_t op) {
	token result{};
	result.type = TOKEN_OPERATOR;
	result.attribute.operator_type = op;
	return result;
}

TEST(ExprSymbolMapping, ClassifiesOperands) {
	for (token_type type :
		 {TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_DOUBLE, TOKEN_STRING, TOKEN_BOOLEAN}) {
		token current{};
		current.type = type;
		EXPECT_EQ(expr_symbol_from_token(&current, true), EXPR_SYMBOL_OPERAND);
	}
}

TEST(ExprSymbolMapping, DistinguishesPrefixAndBinaryPlusMinus) {
	token plus = operatorToken(OPERATOR_PLUS);
	token minus = operatorToken(OPERATOR_MINUS);

	EXPECT_EQ(expr_symbol_from_token(&plus, true), EXPR_SYMBOL_PREFIX);
	EXPECT_EQ(expr_symbol_from_token(&minus, true), EXPR_SYMBOL_PREFIX);
	EXPECT_EQ(expr_symbol_from_token(&plus, false), EXPR_SYMBOL_ADDITIVE);
	EXPECT_EQ(expr_symbol_from_token(&minus, false), EXPR_SYMBOL_ADDITIVE);
}

TEST(ExprSymbolMapping, DistinguishesGroupingAndCallParentheses) {
	token left_parenthesis{};
	left_parenthesis.type = TOKEN_LPAREN;

	EXPECT_EQ(expr_symbol_from_token(&left_parenthesis, true), EXPR_SYMBOL_LEFT_PAREN);
	EXPECT_EQ(expr_symbol_from_token(&left_parenthesis, false), EXPR_SYMBOL_CALL_OPEN);
}

TEST(ExprSymbolMapping, ClassifiesSelectedExtensionOperators) {
	token logical_not = operatorToken(OPERATOR_EX_MARK);
	token conjunction = operatorToken(OPERATOR_AND);
	token disjunction = operatorToken(OPERATOR_OR);

	EXPECT_EQ(expr_symbol_from_token(&logical_not, true), EXPR_SYMBOL_PREFIX);
	EXPECT_EQ(expr_symbol_from_token(&conjunction, false), EXPR_SYMBOL_AND);
	EXPECT_EQ(expr_symbol_from_token(&disjunction, false), EXPR_SYMBOL_OR);
}

TEST(ExprSymbolMapping, LeavesNonExpressionTokensToCaller) {
	token comma = operatorToken(OPERATOR_COMMA);
	token assignment = operatorToken(OPERATOR_ASSIGN);

	EXPECT_EQ(expr_symbol_from_token(nullptr, true), EXPR_SYMBOL_INVALID);
	EXPECT_EQ(expr_symbol_from_token(&comma, false), EXPR_SYMBOL_INVALID);
	EXPECT_EQ(expr_symbol_from_token(&assignment, false), EXPR_SYMBOL_INVALID);
}

TEST(ExprPrecedenceTable, EncodesOperatorPrecedence) {
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_ADDITIVE, EXPR_SYMBOL_MULTIPLICATIVE),
			  EXPR_RELATION_SHIFT);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_MULTIPLICATIVE, EXPR_SYMBOL_ADDITIVE),
			  EXPR_RELATION_REDUCE);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_RELATIONAL, EXPR_SYMBOL_EQUALITY),
			  EXPR_RELATION_REDUCE);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_EQUALITY, EXPR_SYMBOL_AND),
			  EXPR_RELATION_REDUCE);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_AND, EXPR_SYMBOL_OR), EXPR_RELATION_REDUCE);
}

TEST(ExprPrecedenceTable, EncodesAssociativityAndNonAssociativity) {
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_ADDITIVE, EXPR_SYMBOL_ADDITIVE),
			  EXPR_RELATION_REDUCE);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_PREFIX, EXPR_SYMBOL_PREFIX),
			  EXPR_RELATION_SHIFT);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_RELATIONAL, EXPR_SYMBOL_RELATIONAL),
			  EXPR_RELATION_ERROR);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_EQUALITY, EXPR_SYMBOL_EQUALITY),
			  EXPR_RELATION_ERROR);
}

TEST(ExprPrecedenceTable, HandlesParenthesesAndInvalidRelations) {
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_LEFT_PAREN, EXPR_SYMBOL_RIGHT_PAREN),
			  EXPR_RELATION_MATCH);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_LEFT_PAREN, EXPR_SYMBOL_OPERAND),
			  EXPR_RELATION_SHIFT);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_LEFT_PAREN, EXPR_SYMBOL_ADDITIVE),
			  EXPR_RELATION_SHIFT);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_OPERAND, EXPR_SYMBOL_CALL_OPEN),
			  EXPR_RELATION_SHIFT);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_CALL_OPEN, EXPR_SYMBOL_RIGHT_PAREN),
			  EXPR_RELATION_MATCH);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_OPERAND, EXPR_SYMBOL_OPERAND),
			  EXPR_RELATION_ERROR);
	EXPECT_EQ(expr_precedence_relation(EXPR_SYMBOL_COUNT, EXPR_SYMBOL_END), EXPR_RELATION_ERROR);
}

} // namespace
