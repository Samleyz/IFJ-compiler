/**
 * @file lexer.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Tomáš Černý (xcernyt00@vutbr.cz)
 * @brief Implementation of a lexer, provides token on demand
 * @date 2026-10-04
 */

#include "lexer.h"
#include "error.h"
#include "lexer_private.h"
#include "token.h"

static lexer_state_t lexer_state;

bool lexer_init(FILE *source) {
	if (source == NULL) {
		return false;
	}

	lexer_state.fsm_state = START;
	lexer_state.bracket_cnt = 0;
	lexer_state.comment_depth_cnt = 0;
	lexer_state.line_num = 1;
	lexer_state.source_file = source;

	bool init_status =
		(lexer_state.indent_stack = stack_init()) &&	 // Stack init
		stack_push(lexer_state.indent_stack, (size_t)0); // Set initial indent level to be zero

	return init_status;
}

void lexer_cleanup(void) {
	stack_dtor(lexer_state.indent_stack); // isn't memory leak, nothing allocated is in the stack
}

token lexer_get_token(void) {
	int c;

	while (true) {
		c = fgetc(lexer_state.source_file);
		(void)c;

		switch (lexer_state.fsm_state) {
		case START:
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
			break;
		case MAIN_M:
			break;
		case MAIN_A:
			break;
		case MAIN_I:
			break;
		case MAIN_N:
			break;
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
			break;
		case OP_MINUS:
			break;
		case OP_COLON:
			break;
		case OP_STAR:
			break;
		case OP_COMMA:
			break;
		case OP_EX_MARK:
			break;
		case OP_NOT_EQUAL:
			break;
		case OP_AND:
			break;
		case OP_AND_ENTRY:
			break;
		case OP_PLUS:
			break;
		case OP_PLUS_PLUS:
			break;
		case OP_VERTICAL_BAR:
			break;
		case OP_OR:
			break;
		case OP_SLASH:
			break;
		case OP_LESS:
			break;
		case OP_LESS_EQUAL:
			break;
		case OP_LEFT_ARROW:
			break;
		case OP_GREATER:
			break;
		case OP_GREATER_EQUAL:
			break;
		case OP_ASSIGN:
			break;
		case OP_RIGHT_DOUBLE_ARROW:
			break;
		case OP_EQUAL:
			break;
		case ONE_LINE_COM:
			break;
		case BLOCK_COM:
			break;
		case NEWLINE_BLOCK_COM:
			break;
		case ALERT_COM_IN_1:
			break;
		case ALERT_COM_IN_2:
			break;
		case ALERT_COM_OUT_1:
			break;
		case ALERT_COM_OUT2:
			break;
		case COMMENT_ENTRY:
			break;
		default:
			break;
		}
	}
}
