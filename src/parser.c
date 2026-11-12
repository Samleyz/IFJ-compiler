/**
 * @file parser.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of parser, provides functions to parse the source code
 * @date 2026-10-20
 */

#include "parser.h"
#include "parser_grammar.h"
#include "parser_private.h"

static parser_t parser;

void parser_init(void) {
	parser.root = nullptr;
	parser_advance(&parser);
	parser_advance(&parser);
}

void parser_cleanup(void) {
	parser.root = ast_dtor(parser.root);
	free_token_attr(&parser.curr_tkn);
	free_token_attr(&parser.look_ahead_tkn);
}

ast_node_t parse_program(void) {
	ast_node_t root = ast_create_node(NODE_PROGRAM, parser.curr_tkn.line_num);
	if (!root) {
		throw_err(ERR_INTERNAL, 0, "Memory allocation failed for root AST");
	}

	parser.root = root;
	ast_list_init(&parser.root->as.program.items);

	if (!parse_header(&parser)) {
		return NULL;
	}

	if (!parse_top_def_seq(&parser)) {
		return NULL;
	}

	if (!expect_token_type(&parser, TOKEN_EOF)) {
		return NULL;
	}

	return parser.root;
}
