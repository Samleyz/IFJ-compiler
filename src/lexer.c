/**
 * @file lexer.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of a lexer, provides token on demand
 * @date 2026-10-04
 */

#include "lexer.h"
#include "dynamic_string.h"
#include "error.h"
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

	bool init_status = true;
	init_status =
		init_status && (lexer_state.indent_stack = stack_init()) && // Stack init
		stack_push(lexer_state.indent_stack, (size_t)0); // Set initial indent level to be zero

	init_status =
		init_status && (lexer_state.dynamic_string = dstring_init()); // Dynamic string init

	return init_status;
}

void lexer_cleanup(void) {
	stack_dtor(lexer_state.indent_stack); // isn't memory leak-nothing allocated stored in the stack
	dstring_dtor(lexer_state.dynamic_string);
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
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
				c == '_') {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
				// stay
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// extract string
				char *id_str = dstring_get_string(lexer_state.dynamic_string);
				if (!id_str) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to get string from dynamic string");
				}
				// check keyword
				kw_type_t kw_type;
				if (is_keyword(id_str, &kw_type)) {
					free(id_str);
					dstring_erase(lexer_state.dynamic_string);
					return (token){.type = TOKEN_KEYWORD,
								   .attribute = (token_attr_t){.keyword = kw_type},
								   .line_num = lexer_state.line_num};
				}
				// check bool literal
				bool bool_val;
				if (is_bool_literal(id_str, &bool_val)) {
					free(id_str);
					dstring_erase(lexer_state.dynamic_string);
					return (token){.type = TOKEN_BOOLEAN,
								   .attribute = (token_attr_t){.bool_literal = bool_val},
								   .line_num = lexer_state.line_num};
				}
				// normal identifier
				dstring_erase(lexer_state.dynamic_string);
				return (token){.type = TOKEN_IDENTIFIER,
							   .attribute = (token_attr_t){.string_val = id_str},
							   .line_num = lexer_state.line_num};
			}
			break;
		case UNDERSCORE:
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
				c == '_') {
				// append _
				if (!dstring_append_char(lexer_state.dynamic_string, '_')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append _ to dynamic string");
				}
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return underscore token
				dstring_erase(lexer_state.dynamic_string);
				return (token){.type = TOKEN_UNDERSCORE,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case NUMBER:
			if (c >= '0' && c <= '9') {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
				// stay
			} else if (c == '_') {
				// move
				lexer_state.fsm_state = UND_FOR_NUMS;
			} else if (c == '.') {
				// move
				lexer_state.fsm_state = DOT_STATE;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c == 'e' || c == 'E') {
				// move
				lexer_state.fsm_state = EXPONENT;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c == 'd' || c == 'D') {
				// move
				lexer_state.fsm_state = D_RETURN;
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return number token
				char *num_str = dstring_get_string(lexer_state.dynamic_string);
				if (!num_str) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to get string from dynamic string");
				}
				int num;
				if (convert_int(num_str, &num)) {
					free(num_str);
					dstring_erase(lexer_state.dynamic_string);
					return (token){.type = TOKEN_INTEGER,
								   .attribute = (token_attr_t){.int_val = num},
								   .line_num = lexer_state.line_num};
				} else {
					free(num_str);
					return (token){.type = TOKEN_INVALID,
								   .attribute = (token_attr_t){},
								   .line_num = lexer_state.line_num};
				}
			}
			break;
		case UND_FOR_NUMS:
			if (c == '_') {
				// stay
			} else if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = NUMBER;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case ZERO:
			if (c == 'e' || c == 'E') {
				// move
				lexer_state.fsm_state = EXPONENT;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c == '.') {
				// move
				lexer_state.fsm_state = DOT_STATE;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c == 'd' || c == 'D') {
				// move
				lexer_state.fsm_state = D_RETURN;
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return zero token
				dstring_erase(lexer_state.dynamic_string);
				return (token){.type = TOKEN_INTEGER,
							   .attribute = (token_attr_t){.int_val = 0},
							   .line_num = lexer_state.line_num};
			}
			break;
		case DOUBLE:
			if (c == 'e' || c == 'E') {
				// move
				lexer_state.fsm_state = EXPONENT;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c == '_') {
				// move
				lexer_state.fsm_state = UND_FOR_DOUBLES;
			} else if (c >= '0' && c <= '9') {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
				// stay
			} else if (c == 'd' || c == 'D') {
				// move
				lexer_state.fsm_state = D_RETURN;
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return double token
				char *num_str = dstring_get_string(lexer_state.dynamic_string);
				if (!num_str) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to get string from dynamic string");
				}
				double num;
				if (convert_double(num_str, &num)) {
					free(num_str);
					dstring_erase(lexer_state.dynamic_string);
					return (token){.type = TOKEN_DOUBLE,
								   .attribute = (token_attr_t){.double_val = num},
								   .line_num = lexer_state.line_num};
				} else {
					// invalid
					free(num_str);
					return (token){.type = TOKEN_INVALID,
								   .attribute = (token_attr_t){},
								   .line_num = lexer_state.line_num};
				}
			}
			break;
		case EXPONENT:
			if (c == '+' || c == '-') {
				// move
				lexer_state.fsm_state = EXP_SIGN;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = EX_NUM;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case EX_NUM:
			if (c >= '0' && c <= '9') {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
				// stay
			} else if (c == '_') {
				// move
				lexer_state.fsm_state = UND_FOR_EXP;
			} else if (c == 'd' || c == 'D') {
				// move
				lexer_state.fsm_state = D_RETURN;
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return number token
				char *num_str = dstring_get_string(lexer_state.dynamic_string);
				if (!num_str) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to get string from dynamic string");
				}
				double double_num;
				if (convert_double(num_str, &double_num)) {
					free(num_str);
					dstring_erase(lexer_state.dynamic_string);
					return (token){.type = TOKEN_DOUBLE,
								   .attribute = (token_attr_t){.double_val = double_num},
								   .line_num = lexer_state.line_num};
				} else {
					// invalid
					free(num_str);
					return (token){.type = TOKEN_INVALID,
								   .attribute = (token_attr_t){},
								   .line_num = lexer_state.line_num};
				}
			}
			break;
		case EXP_SIGN:
			if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = EX_NUM;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case DOT_STATE:
			if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = DOUBLE;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case DOT_STATE_2:
			if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = DOUBLE;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case D_RETURN:
			// unget
			ungetc(c, lexer_state.source_file);
			// move
			lexer_state.fsm_state = START;
			// try return double token
			char *num_str = dstring_get_string(lexer_state.dynamic_string);
			if (!num_str) {
				throw_err(ERR_INTERNAL, lexer_state.line_num,
						  "failed to get string from dynamic string");
			}
			double double_num;
			if (convert_double(num_str, &double_num)) {
				free(num_str);
				dstring_erase(lexer_state.dynamic_string);
				return (token){.type = TOKEN_DOUBLE,
							   .attribute = (token_attr_t){.double_val = double_num},
							   .line_num = lexer_state.line_num};
			} else {
				// invalid
				free(num_str);
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case UND_FOR_DOUBLES:
			if (c == '_') {
				// stay
			} else if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = DOUBLE;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case UND_FOR_EXP:
			if (c == '_') {
				// stay
			} else if (c >= '0' && c <= '9') {
				// move
				lexer_state.fsm_state = EX_NUM;
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
			break;
		case EOL_STATE:
			break;
		case EOF_STATE:
			break;
		case HANDLE_INDENT:
			ungetc(c, lexer_state.source_file);
			int hi_status = handle_indent(&lexer_state);
			if (hi_status == 0) {
				// Continue in FSM
				c = fgetc(lexer_state.source_file);
				if (c == '\t') {
					lexer_state.fsm_state = TAB_STATE;
				} else {
					ungetc(c, lexer_state.source_file);
					lexer_state.fsm_state = START;
				}
			} else if (hi_status == 1) {
				return (token){.type = TOKEN_INDENT,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			} else if (hi_status == -1) {
				return (token){.type = TOKEN_DEDENT,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			} else if (hi_status == -2) {
				// Indentation error
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			} else if (hi_status == -3) {
				throw_err(ERR_INTERNAL, lexer_state.line_num,
						  "failed stack push inside handle indent");
			}
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
			if (c == '\\') {
				// move
				lexer_state.fsm_state = ALERT_ESC;
			} else if (c == '"') {
				// move
				lexer_state.fsm_state = STRING_END;
			} else {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			}
			break;
		case STRING_END:
			if (c == '"') {
				// move
				lexer_state.fsm_state = BLOCK_STRING;
			} else {
				// unget
				ungetc(c, lexer_state.source_file);
				// move
				lexer_state.fsm_state = START;
				// return string token
				char *str_val = dstring_get_string(lexer_state.dynamic_string);
				if (!str_val) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to get string from dynamic string");
				}
				dstring_erase(lexer_state.dynamic_string);
				return (token){.type = TOKEN_STRING,
							   .attribute = (token_attr_t){.string_val = str_val},
							   .line_num = lexer_state.line_num};
			}
			break;
		case BLOCK_STRING:
			if (c == '"') {
				// move
				lexer_state.fsm_state = BLOCK_STRING_EX_1;
			} else {
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			}
			break;
		case BLOCK_STRING_END:
			// unget
			ungetc(c, lexer_state.source_file);
			// move
			lexer_state.fsm_state = START;
			// return string token
			char *str_val = dstring_get_string(lexer_state.dynamic_string);
			if (!str_val) {
				throw_err(ERR_INTERNAL, lexer_state.line_num,
						  "failed to get string from dynamic string");
			}
			dstring_erase(lexer_state.dynamic_string);
			return (token){.type = TOKEN_STRING,
						   .attribute = (token_attr_t){.string_val = str_val},
						   .line_num = lexer_state.line_num};
			break;
		case BLOCK_STRING_EX_1:
			if (c == '"') {
				// move
				lexer_state.fsm_state = BLOCK_STRING_EX_2;
			} else {
				// move
				lexer_state.fsm_state = BLOCK_STRING;
				// append "
				if (!dstring_append_char(lexer_state.dynamic_string, '"')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append \" to dynamic string");
				}
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			}
			break;
		case BLOCK_STRING_EX_2:
			if (c == '"') {
				// move
				lexer_state.fsm_state = BLOCK_STRING_END;
			} else {
				// move
				lexer_state.fsm_state = BLOCK_STRING;
				// append "
				if (!dstring_append_char(lexer_state.dynamic_string, '"')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append \" to dynamic string");
				}
				// append "
				if (!dstring_append_char(lexer_state.dynamic_string, '"')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append \" to dynamic string");
				}
				// append
				if (!dstring_append_char(lexer_state.dynamic_string, (char)c)) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append char to dynamic string");
				}
			}
			break;
		case ALERT_ESC:
			if (c == 't') {
				// move
				lexer_state.fsm_state = STRING;
				// append tab
				if (!dstring_append_char(lexer_state.dynamic_string, '\t')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append tab to dynamic string");
				}
			} else if (c == 'r') {
				// move
				lexer_state.fsm_state = STRING;
				// append return
				if (!dstring_append_char(lexer_state.dynamic_string, '\r')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append return to dynamic string");
				}
			} else if (c == 'n') {
				// move
				lexer_state.fsm_state = STRING;
				// append newline
				if (!dstring_append_char(lexer_state.dynamic_string, '\n')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append newline to dynamic string");
				}
			} else if (c == '"') {
				// move
				lexer_state.fsm_state = STRING;
				// append double quote
				if (!dstring_append_char(lexer_state.dynamic_string, '"')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append double quote to dynamic string");
				}
			} else if (c == '\\') {
				// move
				lexer_state.fsm_state = STRING;
				// append backslash
				if (!dstring_append_char(lexer_state.dynamic_string, '\\')) {
					throw_err(ERR_INTERNAL, lexer_state.line_num,
							  "failed to append backslash to dynamic string");
				}
			} else {
				// invalid
				return (token){.type = TOKEN_INVALID,
							   .attribute = (token_attr_t){},
							   .line_num = lexer_state.line_num};
			}
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
