/**
 * @file main.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @brief Performs translation from laguage IFJ26 to IFJ26code
 * @date 2026-10-20
 */

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"

int main(void) {
	if (!lexer_init(stdin)) {
		throw_err(ERR_INTERNAL, 0, "Lexer initialization failed!");
	}
	if (!parser_init()) {
		throw_err(ERR_INTERNAL, 0, "Parser initialization failed!");
	}

	ast_node_t ast_root;

	ast_root = parse_program();

	(void *)ast_root;

	// TODO semantic analysis
	// TODO code generation

	parser_cleanup();
	lexer_cleanup();

	return 0;
}
