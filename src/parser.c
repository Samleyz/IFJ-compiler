/**
 * @file parser.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Implementation of parser, provides functions to parse the source code
 * @date 2026-10-20
 */

#include "parser.h"
#include "parser_private.h"

static parser_t parser;

void parser_init(void) {
	parser.root = nullptr;
	parser_advance(&parser);
	parser_advance(&parser);
}

void parser_cleanup(void) {}

ast_node_t parse_program(void) {
	// TODO parse program rule implementation
	return nullptr;
}
