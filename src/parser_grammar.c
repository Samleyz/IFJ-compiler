/**
 * @file parser_grammar.c
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Implementation of the IFJ26 grammar (LL1*) rules
 * @date 2026-10-22
 */

#include "parser_grammar.h"
#include "parse_expr.h"
#include "stdlib.h"
#include <string.h>

/*
 * Note for whoever implements this:
 * Grab token helpers from parser_private.h (peek, match, expect - remember expect blows up on
 * error). Expressions aren't handled here, just call parse_expression() from parse_expr.h.
 * Every rule returns an ast_node_t subtree. Create the AST node right at the start of the
 * function - all nodes will be tracked and cleaned up centrally on error anyway, so no need to
 * create temporary locals.
 */

ast_node_t parse_header(parser_t *p) {
	if (!expect_keyword(p, KW_IMPORT)) {
		return NULL;
	}
	if (!expect_specific_id(p, "ifj26")) {
		return NULL;
	}
	if (!expect_operator(p, OPERATOR_DOT)) {
		return NULL;
	}
	if (!expect_operator(p, OPERATOR_STAR)) {
		return nullptr;
	}
	if (!expect_token_type(p, TOKEN_EOL)) {
		return nullptr;
	}
	return p->root;
}

ast_node_t parse_top_def_seq(parser_t *p) {
	while (match_token_type(p, TOKEN_EOL))
		; // skipping EOLs

	// Rule 3: <top_def_seq> -> <top_def> <top_def_seq>
	if (peek_token_type(p, TOKEN_MAIN) || peek_keyword(p, KW_DEF) ||
		peek_keyword(p, KW_EXTENSION)) {

		ast_node_t def = parse_top_def(p); // <top_def>
		if (!def) {
			return NULL;
		}

		ast_list_append(&p->root->as.program.items, def);

		return parse_top_def_seq(p); // <top_def_seq>
	}
	// Rule 4: <top_def_seq> -> eps
	else if (peek_token_type(p, TOKEN_EOF)) {
		return p->root;
	} else {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num,
					"Unexpected token, expected top-level definition or EOF");
		return NULL;
	}
}

ast_node_t parse_top_def(parser_t *p) {
	// Rule 5: <top_def> -> <func_def>
	if (peek_token_type(p, TOKEN_MAIN) || peek_keyword(p, KW_DEF)) {
		ast_node_t func = parse_func_def(p);
		return func;
	}
	// Rule 6: <top_def> -> <ext_def>
	else if (peek_keyword(p, KW_EXTENSION)) {
		ast_node_t ext = parse_ext_def(p);
		return ext;
	}
	return NULL;
}

ast_node_t parse_func_def(parser_t *p) {
	// <opt_main>
	bool main = false;
	if (match_token_type(p, TOKEN_MAIN)) {
		main = true;
		match_token_type(p, TOKEN_EOL);
	}

	// "def"
	if (!expect_keyword(p, KW_DEF)) {
		return NULL;
	}

	// identifier
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected function name");
		return NULL;
	}
	char *func_id = strdup(p->curr_tkn.attribute.string_val);
	parser_advance(p);

	/// func node creation, more rule to comes
	ast_node_t func_node = ast_create_node(NODE_FUNC_DEF, p->curr_tkn.line_num);
	if (!func_node) {
		free(func_id);
		return NULL;
	}
	func_node->as.func_def.name = func_id;
	func_node->as.func_def.is_main = main;
	ast_list_init(&func_node->as.func_def.param_lists);

	// <param_list_seq>
	bool has_parens = false;
	while (peek_token_type(p, TOKEN_LPAREN)) {
		has_parens = true;
		ast_node_t plist = parse_param_list(p);
		if (!plist) {
			ast_dtor(func_node);
			return NULL;
		}
		ast_list_append(&func_node->as.func_def.param_lists, plist);
	}
	func_node->as.func_def.has_parens = has_parens;

	// <opt_type>
	if (match_operator(p, OPERATOR_COLON)) {
		char *ret_type = parse_type(p);
		if (!ret_type) {
			ast_dtor(func_node);
			return NULL;
		}
		func_node->as.func_def.return_type = ret_type;
	} else {
		func_node->as.func_def.return_type = NULL;
	}

	// "="
	if (!expect_operator(p, OPERATOR_ASSIGN)) {
		ast_dtor(func_node);
		return NULL;
	}

	// <func_body>
	ast_node_t body = parse_func_body(p);
	if (!body) {
		ast_dtor(func_node);
		return NULL;
	}
	func_node->as.func_def.body = body;

	return func_node;
}

ast_node_t parse_func_body(parser_t *p) {

	if (match_token_type(p, TOKEN_EOL)) {
		return parse_block(p);
	}

	ast_node_t expr = parse_expression(p);
	if (!expr) {
		return NULL;
	}

	if (!expect_token_type(p, TOKEN_EOL)) {
		ast_dtor(expr);
		return NULL;
	}

	return expr;
}

ast_node_t parse_ext_def(parser_t *p) {
	// <ext_def> -> "extension" "(" identifier ":" <type> ")" "EOL" "indent" <ext_method_seq>

	// already peeked for extension, we know its present
	ast_node_t ext_node = ast_create_node(NODE_EXTENSION, p->curr_tkn.line_num);
	if (!ext_node) {
		free(ext_node);
		return NULL;
	}
	ast_list_init(&ext_node->as.extension.methods);

	// "extension"
	if (!expect_keyword(p, KW_EXTENSION)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// "("
	if (!expect_token_type(p, TOKEN_LPAREN)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// identifier
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected function name");
		ast_dtor(ext_node);
		return NULL;
	}
	char *ext_reciever = strdup(p->curr_tkn.attribute.string_val);
	ext_node->as.extension.receiver_name = ext_reciever;
	parser_advance(p);

	// ":"
	if (!expect_operator(p, OPERATOR_COLON)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// <type>
	char *rec_type = parse_type(p);
	if (!rec_type) {
		ast_dtor(ext_node);
		return NULL;
	}
	ext_node->as.extension.receiver_type = rec_type;

	// ")"
	if (!expect_token_type(p, TOKEN_RPAREN)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// "EOL"
	if (!expect_token_type(p, TOKEN_EOL)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// "indent"
	if (!expect_token_type(p, TOKEN_INDENT)) {
		ast_dtor(ext_node);
		return NULL;
	}

	// ext_method_seq
	while (peek_keyword(p, KW_DEF)) {
		ast_node_t method = parse_func_def(p);
		if (!method) {
			ast_dtor(ext_node);
			return NULL;
		}
		ast_list_append(&ext_node->as.extension.methods, method);

		while (match_token_type(p, TOKEN_EOL))
			;
	}

	if (ext_node->as.extension.methods.count == 0) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num,
					"Extension must contain at least one method");
		ast_dtor(ext_node);
		return NULL;
	}

	// "dedent"
	if (!expect_token_type(p, TOKEN_DEDENT)) {
		ast_dtor(ext_node);
		return NULL;
	}

	return ext_node;
}

ast_node_t parse_block(parser_t *p) {
	// TODO: Rule 7
	// <block> -> "indent" <line> <line_seq> <opt_eol> "dedent"
	(void)p;
	return nullptr;
}

ast_node_t parse_cond_stmt(parser_t *p) {
	// TODO: Rule 76-82
	// <cond_stmt> -> "if" expr "then" <branch_body> <opt_else>
	(void)p;
	return nullptr;
}

ast_node_t parse_if_statement(parser_t *p) { return parse_cond_stmt(p); }

ast_node_t parse_while_stmt(parser_t *p) {
	// TODO: Rule 83
	// <while_stmt> -> "while" expr "do" "EOL" <block>
	(void)p;
	return nullptr;
}

ast_node_t parse_for_stmt(parser_t *p) {
	// TODO: Rule 84-95
	// <for_stmt> -> "for" <for_header> "do" <branch_body>
	(void)p;
	return nullptr;
}

ast_node_t parse_var_def(parser_t *p) {
	// TODO: Rule 67-69
	// <var_def> -> <var_kw> identifier <opt_type> "=" expr
	(void)p;
	return nullptr;
}

ast_node_t parse_assign_stmt(parser_t *p) {
	// TODO: Rule 70
	// <assign_stmt> -> identifier "=" expr
	(void)p;
	return nullptr;
}

ast_node_t parse_return_stmt(parser_t *p) {
	// TODO: Rule 51-53
	// <return_stmt> -> "return" <opt_expr>
	(void)p;
	return nullptr;
}

ast_node_t parse_simple_stmt(parser_t *p) {
	// TODO: Rule 17-20
	// <simple_stmt> -> <var_def> | <assign_stmt> | <return_stmt> | expr
	(void)p;
	return nullptr;
}

ast_node_t parse_complex_stmt(parser_t *p) {
	// TODO: Rule 21-23
	// <complex_stmt> -> <while_stmt> | <cond_stmt> | <for_stmt>
	(void)p;
	return nullptr;
}

ast_node_t parse_param_list(parser_t *p) {
	// TODO: Rule 34-41
	// <param_list> -> "(" <param_list_opt> ")"
	(void)p;
	return nullptr;
}

char *parse_type(parser_t *p) {
	// TODO: Rule 71-75
	// <type> -> "Int" | "Double" | "String" | "Unit" | "Boolean"
	(void)p;
	return nullptr;
}
