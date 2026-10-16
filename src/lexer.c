/**
 * @file lexer.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of a lexer, provides token on demand
 * @date 2026-10-04
 */

#include "lexer.h"
#include "lexer_private.h"
#include "token.h"
#include <ctype.h>
#include <stdlib.h>

static lexer_state_t lexer_state;

bool lexer_init(FILE *source) {
	if (source == NULL) {
		return false;
	}

	if (atexit(lexer_cleanup) != 0) {
		return false;
	}

	lexer_state.fsm_state = START;
	lexer_state.bracket_cnt = 0;
	lexer_state.comment_depth_cnt = 0;
	lexer_state.line_num = 1;
	lexer_state.source_file = source;

	// TODO stack init
	return true;
}

void lexer_cleanup(void) {
	// TODO free stack
}

token lexer_get_token(void) {
	int c;

	while (true) {
		c = fgetc(lexer_state.source_file);
		(void)c;

		switch (lexer_state.fsm_state) {
		case START:
			if (c == EOF) {
				lexer_state.fsm_state = EOF_STATE;
			} else if (c == '\n') {
				lexer_state.fsm_state =
					lexer_state.bracket_cnt == 0 ? EOL_STATE : B_EOL_STATE;
			} else if (c == ' ') {
				lexer_state.fsm_state = START;
			} else if (c == '\t') {
				lexer_state.fsm_state = TAB_STATE;
			} else if (c == '/') {
				lexer_state.fsm_state = OP_SLASH;
			} else if (c == '"') {
				dstring_erase(lexer_state.dynamic_string);
				lexer_state.fsm_state = STRING;
			} else if (isalpha(c)) {
				dstring_erase(lexer_state.dynamic_string);
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num, "Failed to append identifier");
				}
				lexer_state.fsm_state = IDENTIFIER;
			} else if (isdigit(c)) {
				dstring_erase(lexer_state.dynamic_string);
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num, "Failed to append number");
				}
				if (c == '0') {
					lexer_state.fsm_state = ZERO;
				} else {
					lexer_state.fsm_state = NUMBER;
				}
			} else if (c == '_') {
				dstring_erase(lexer_state.dynamic_string);
				lexer_state.fsm_state = UNDERSCORE;
			} else if (c == '@') {
				lexer_state.fsm_state = MAIN_AT;
			} else if (c == '(') {
				lexer_state.fsm_state = O_BRACKET;
			} else if (c == ')') {
				lexer_state.fsm_state = C_BRACKET;
			} else if (c == '[') {
				lexer_state.fsm_state = O_SQUARE_BRACKET;
			} else if (c == ']') {
				lexer_state.fsm_state = C_SQUARE_BRACKET;
			} else if (c == '&') {
				lexer_state.fsm_state = OP_AND_ENTRY;
			} else if (c == '|') {
				lexer_state.fsm_state = OP_VERTICAL_BAR;
			} else if (c == '+') {
				lexer_state.fsm_state = OP_PLUS;
			} else if (c == '-') {
				lexer_state.fsm_state = OP_MINUS;
			} else if (c == '*') {
				lexer_state.fsm_state = OP_STAR;
			} else if (c == ';') {
				lexer_state.fsm_state = OP_SEMICOLON;
			} else if (c == ',') {
				lexer_state.fsm_state = OP_COMMA;
			} else if (c == ':') {
				lexer_state.fsm_state = OP_COLON;
			} else if (c == '!') {
				lexer_state.fsm_state = OP_EX_MARK;
			} else if (c == '<') {
				lexer_state.fsm_state = OP_LESS;
			} else if (c == '>') {
				lexer_state.fsm_state = OP_GREATER;
			} else if (c == '=') {
				lexer_state.fsm_state = OP_ASSIGN;
			} else if (c == '.') {
				dstring_erase(lexer_state.dynamic_string);
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num, "Failed to append decimal point");
				}
				lexer_state.fsm_state = DOT_STATE_2;
			} else {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Unrecognized character");
			}
			break;
		case IDENTIFIER:
			break;
		case UNDERSCORE:
			break;
		case NUMBER:
			break;
		case UND_FOR_NUMS:
			break;
		case ZERO:
			break;
		case DOUBLE:
			break;
		case EXPONENT:
			break;
		case EX_NUM:
			break;
		case EXP_SIGN:
			break;
		case DOT_STATE:
			break;
		case DOT_STATE_2:
			break;
		case D_RETURN:
			break;
		case UND_FOR_DOUBLES:
			break;
		case UND_FOR_EXP:
			break;
		case EOL_STATE:
			break;
		case EOF_STATE:
			break;
		case HANDLE_INDENT:
			break;
		case TAB_STATE:
			break;
		case MAIN_AT:
			if (c != 'm') {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Expected @main");
			}
			lexer_state.fsm_state = MAIN_M;
			break;
		case MAIN_M:
			if (c != 'a') {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Expected @main");
			}
			lexer_state.fsm_state = MAIN_A;
			break;
		case MAIN_A:
			if (c != 'i') {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Expected @main");
			}
			lexer_state.fsm_state = MAIN_I;
			break;
		case MAIN_I:
			if (c != 'n') {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Expected @main");
			}
			lexer_state.fsm_state = MAIN_N;
			break;
		case MAIN_N:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_MAIN, .line_num = lexer_state.line_num};

		case O_BRACKET:
			break;
		case C_BRACKET:
			break;
		case B_EOL_STATE:
			break;
		case O_SQUARE_BRACKET:
			break;
		case C_SQUARE_BRACKET:
			break;
		case STRING:
			break;
		case STRING_END:
			break;
		case BLOCK_STRING:
			break;
		case BLOCK_STRING_END:
			break;
		case BLOCK_STRING_EX_1:
			break;
		case BLOCK_STRING_EX_2:
			break;
		case ALERT_ESC:
			break;
		case OP_SEMICOLON:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_SEMICOLON,
						   .line_num = lexer_state.line_num};
		case OP_MINUS:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_MINUS,
						   .line_num = lexer_state.line_num};
		case OP_COLON:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_COLON,
						   .line_num = lexer_state.line_num};
		case OP_STAR:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_STAR,
						   .line_num = lexer_state.line_num};
		case OP_COMMA:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_COMMA,
						   .line_num = lexer_state.line_num};
		case OP_EX_MARK:
			if (c == '=') {
				lexer_state.fsm_state = OP_NOT_EQUAL;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_EX_MARK,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_NOT_EQUAL:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_NOT_EQUAL,
						   .line_num = lexer_state.line_num};
		case OP_AND:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_AND,
						   .line_num = lexer_state.line_num};
		case OP_AND_ENTRY:
			if (c != '&') {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Expected &&");
			}
			lexer_state.fsm_state = OP_AND;
			break;
		case OP_PLUS:
			if (c == '+') {
				lexer_state.fsm_state = OP_PLUS_PLUS;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_PLUS,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_PLUS_PLUS:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_PLUS_PLUS,
						   .line_num = lexer_state.line_num};
		case OP_VERTICAL_BAR:
			if (c == '|') {
				lexer_state.fsm_state = OP_OR;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_VERTICAL_BAR,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_OR:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_OR,
						   .line_num = lexer_state.line_num};
		case OP_SLASH:
			if (c == '/') {
				lexer_state.fsm_state = ONE_LINE_COM;
			} else if (c == '*') {
				lexer_state.comment_depth_cnt = 1;
				lexer_state.fsm_state = BLOCK_COM;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_SLASH,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_LESS:
			if (c == '=') {
				lexer_state.fsm_state = OP_LESS_EQUAL;
			} else if (c == '-') {
				lexer_state.fsm_state = OP_LEFT_ARROW;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_LESS,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_LESS_EQUAL:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_LESS_EQUAL,
						   .line_num = lexer_state.line_num};
		case OP_LEFT_ARROW:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_LEFT_ARROW,
						   .line_num = lexer_state.line_num};
		case OP_GREATER:
			if (c == '=') {
				lexer_state.fsm_state = OP_GREATER_EQUAL;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_GREATER,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_GREATER_EQUAL:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_GREATER_EQUAL,
						   .line_num = lexer_state.line_num};
		case OP_ASSIGN:
			if (c == '=') {
				lexer_state.fsm_state = OP_EQUAL;
			} else if (c == '>') {
				lexer_state.fsm_state = OP_RIGHT_DOUBLE_ARROW;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
				return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_ASSIGN,
							   .line_num = lexer_state.line_num};
			}
			break;
		case OP_RIGHT_DOUBLE_ARROW:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR,
						   .attribute.operator = OPERATOR_RIGHT_DOUBLE_ARROW,
						   .line_num = lexer_state.line_num};
		case OP_EQUAL:
			ungetc(c, lexer_state.source_file);
			lexer_state.fsm_state = START;
			return (token){.type = TOKEN_OPERATOR, .attribute.operator = OPERATOR_EQUAL,
						   .line_num = lexer_state.line_num};
		case ONE_LINE_COM:
			if (c == EOF) {
				lexer_state.fsm_state = EOF_STATE;
			} else if (c == '\n') {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = START;
			}
			break;
		case BLOCK_COM:
			if (c == EOF) {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Unterminated block comment");
			}
			if (c == '\n') {
				lexer_state.line_num++;
				lexer_state.fsm_state = NEWLINE_BLOCK_COM;
			} else if (c == '*') {
				lexer_state.fsm_state = ALERT_COM_OUT_1;
			} else if (c == '/') {
				lexer_state.fsm_state = ALERT_COM_IN_1;
			}
			break;
		case NEWLINE_BLOCK_COM:
			if (c == EOF) {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Unterminated block comment");
			}
			if (c == '\n') {
				lexer_state.line_num++;
			} else if (c == '*') {
				lexer_state.fsm_state = ALERT_COM_OUT2;
			} else if (c == '/') {
				lexer_state.fsm_state = ALERT_COM_IN_2;
			}
			break;
		case ALERT_COM_IN_1:
			if (c == '*') {
				lexer_state.comment_depth_cnt++;
			} else {
				ungetc(c, lexer_state.source_file);
			}
			lexer_state.fsm_state = BLOCK_COM;
			break;
		case ALERT_COM_IN_2:
			if (c == '*') {
				lexer_state.comment_depth_cnt++;
			} else {
				ungetc(c, lexer_state.source_file);
			}
			lexer_state.fsm_state = NEWLINE_BLOCK_COM;
			break;
		case ALERT_COM_OUT_1:
			if (c == '/') {
				lexer_state.comment_depth_cnt--;
				lexer_state.fsm_state =
					lexer_state.comment_depth_cnt == 0 ? START : BLOCK_COM;
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = BLOCK_COM;
			}
			break;
		case ALERT_COM_OUT2:
			if (c == '/') {
				lexer_state.comment_depth_cnt--;
				if (lexer_state.comment_depth_cnt == 0) {
					lexer_state.line_num--;
					lexer_state.fsm_state = EOL_STATE;
				} else {
					lexer_state.fsm_state = NEWLINE_BLOCK_COM;
				}
			} else {
				ungetc(c, lexer_state.source_file);
				lexer_state.fsm_state = NEWLINE_BLOCK_COM;
			}
			break;
		case COMMENT_ENTRY:
			if (c == '/') {
				lexer_state.fsm_state = ONE_LINE_COM;
			} else if (c == '*') {
				lexer_state.comment_depth_cnt = 1;
				lexer_state.fsm_state = BLOCK_COM;
			} else {
				throw_err(ERR_LEXICAL, lexer_state.line_num, "Invalid comment entry");
			}
			break;
		default:
			throw_err(ERR_INTERNAL, lexer_state.line_num, "Invalid lexer state");
		}
	}
}
