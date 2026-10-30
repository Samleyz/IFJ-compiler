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

void compiler_cleanup(void) {
	lexer_cleanup();
	parser_cleanup();
}

int main(void) {
	error_set_cleanup(compiler_cleanup);

	if (!lexer_init(stdin)) {
		throw_err(ERR_INTERNAL, 0, "Lexer initialization failed!");
	}
	parser_init();

	ast_node_t ast_root;

	ast_root = parse_program();

	if (!ast_root || error_is_set()) {
		error_exit();
	}

	(void *)ast_root;

	// TODO semantic analysis
	// TODO code generation

	compiler_cleanup();

	return 0;
}
