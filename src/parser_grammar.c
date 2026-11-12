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
	// <block> -> "indent" <line> <line_seq> <opt_eol> "dedent"

	// "indent"
	if (!expect_token_type(p, TOKEN_INDENT)) {
		return NULL;
	}

	// creating the node
	ast_node_t block_node = ast_create_node(NODE_BLOCK, p->curr_tkn.line_num);
	if (!block_node) {
		return NULL;
	}
	ast_list_init(&block_node->as.block.statements);

	// <line>
	if (!parse_line(p, block_node)) {
		ast_dtor(block_node);
		return NULL;
	}

	// <line_seq>
	// <line_seq> -> "EOL" <line> <line_seq>
	// <line_seq> -> eps
	while (peek_token_type(p, TOKEN_EOL) || peek_line_beg(p)) {
		// "EOL"
		if (peek_token_type(p, TOKEN_EOL)) {
			if (peek_next_token_type(p, TOKEN_DEDENT)) {
				// <opt_eol>
				match_token_type(p, TOKEN_EOL);
				break;
			}
			match_token_type(p, TOKEN_EOL);
		}

		if (peek_token_type(p, TOKEN_DEDENT)) {
			break;
		}

		// <line>
		if (!parse_line(p, block_node)) {
			ast_dtor(block_node);
			return NULL;
		}
	}

	// <opt_eol>
	// <opt_eol> -> "EOL"
	// <opt_eol> -> eps
	if (peek_token_type(p, TOKEN_EOL)) {
		match_token_type(p, TOKEN_EOL);
	}

	// "dedent"
	if (!expect_token_type(p, TOKEN_DEDENT)) {
		ast_dtor(block_node);
		return NULL;
	}

	return block_node;
}

bool parse_line(parser_t *p, ast_node_t block_node) {
	// <line> -> <simple_seq>
	// <line> -> <complex_stmt>
	if (peek_keyword(p, KW_WHILE) || peek_keyword(p, KW_IF) || peek_keyword(p, KW_FOR)) {
		// <complex_stmt>
		ast_node_t cstmt = parse_complex_stmt(p);
		if (!cstmt) {
			return false;
		}
		return ast_list_append(&block_node->as.block.statements, cstmt);
	} else {
		// Rule 10: <simple_seq> -> <simple_stmt> <simple_seq_tail>
		// <simple_stmt>
		ast_node_t sstmt = parse_simple_stmt(p);
		if (!sstmt) {
			return false;
		}
		if (!ast_list_append(&block_node->as.block.statements, sstmt)) {
			ast_dtor(sstmt);
			return false;
		}

		// Rule 11: <simple_seq_tail> -> ";" <simple_stmt> <simple_seq_tail>
		// Rule 12: <simple_seq_tail> -> eps
		while (match_operator(p, OPERATOR_SEMICOLON)) {
			if (peek_token_type(p, TOKEN_EOL) || peek_token_type(p, TOKEN_DEDENT) ||
				peek_token_type(p, TOKEN_EOF)) {
				break;
			}
			// <simple_stmt>
			sstmt = parse_simple_stmt(p);
			if (!sstmt) {
				return false;
			}
			if (!ast_list_append(&block_node->as.block.statements, sstmt)) {
				ast_dtor(sstmt);
				return false;
			}
		}
		return true;
	}
}

ast_node_t parse_cond_stmt(parser_t *p) {
	// <cond_stmt> -> "if" expr "then" <branch_body> <opt_else>

	unsigned line_num = p->curr_tkn.line_num;

	// "if"
	if (!expect_keyword(p, KW_IF)) {
		return NULL;
	}

	// creating node
	ast_node_t if_node = ast_create_node(NODE_IF, line_num);
	if (!if_node) {
		return NULL;
	}

	// expr
	ast_node_t cond = parse_expression(p);
	if (!cond) {
		ast_dtor(if_node);
		return NULL;
	}
	if_node->as.if_stmt.condition = cond;

	// "then"
	if (!expect_keyword(p, KW_THEN)) {
		ast_dtor(if_node);
		return NULL;
	}

	// <branch_body>
	ast_node_t then_branch = parse_branch_body(p);
	if (!then_branch) {
		ast_dtor(if_node);
		return NULL;
	}
	if_node->as.if_stmt.then_branch = then_branch;

	// <opt_else>
	// <opt_else> -> <opt_eol> "else" <else_branch>
	// <opt_else> -> eps
	if (peek_token_type(p, TOKEN_EOL) && peek_next_keyword(p, KW_ELSE)) {
		// <opt_eol>
		match_token_type(p, TOKEN_EOL);
	}

	// "else"
	if (match_keyword(p, KW_ELSE)) {
		// <else_branch> -> <cond_stmt>
		// <else_branch> -> <branch_body>
		if (peek_keyword(p, KW_IF)) {
			// <cond_stmt>
			ast_node_t else_branch = parse_cond_stmt(p);
			if (!else_branch) {
				ast_dtor(if_node);
				return NULL;
			}
			if_node->as.if_stmt.else_branch = else_branch;
		} else {
			// <branch_body>
			ast_node_t else_branch = parse_branch_body(p);
			if (!else_branch) {
				ast_dtor(if_node);
				return NULL;
			}
			if_node->as.if_stmt.else_branch = else_branch;
		}
	} else {
		if_node->as.if_stmt.else_branch = NULL;
	}

	return if_node;
}

ast_node_t parse_branch_body(parser_t *p) {
	// <branch_body> -> "EOL" <block>
	// <branch_body> -> <simple_stmt>

	// "EOL"
	if (match_token_type(p, TOKEN_EOL)) {
		// <block>
		return parse_block(p);
	}

	// <simple_stmt>
	return parse_simple_stmt(p);
}

ast_node_t parse_while_stmt(parser_t *p) {
	// Rule 83: <while_stmt> -> "while" expr "do" "EOL" <block>

	unsigned line_num = p->curr_tkn.line_num;

	// "while"
	if (!expect_keyword(p, KW_WHILE)) {
		return NULL;
	}

	// node creation
	ast_node_t while_node = ast_create_node(NODE_WHILE, line_num);
	if (!while_node) {
		return NULL;
	}

	// expr
	ast_node_t cond = parse_expression(p);
	if (!cond) {
		ast_dtor(while_node);
		return NULL;
	}
	while_node->as.while_stmt.condition = cond;

	// "do"
	if (!expect_keyword(p, KW_DO)) {
		ast_dtor(while_node);
		return NULL;
	}

	// "EOL"
	if (!expect_token_type(p, TOKEN_EOL)) {
		ast_dtor(while_node);
		return NULL;
	}

	// <block>
	ast_node_t body = parse_block(p);
	if (!body) {
		ast_dtor(while_node);
		return NULL;
	}
	while_node->as.while_stmt.body = body;

	return while_node;
}

ast_node_t parse_for_stmt(parser_t *p) {
	// Rule 84: <for_stmt> -> "for" <for_header> "do" <branch_body>
	// Rule 85: <for_header> -> <for_generator> <guards_inline>
	// Rule 86: <for_header> -> "EOL" "indent" <for_generator> "EOL" <guards_multiline> "dedent"
	// Rule 87: <for_generator> -> identifier "<-" expr <range_type>
	// Rule 88: <range_type> -> "to" expr <opt_by>
	// Rule 89: <range_type> -> "until" expr
	// Rule 90: <opt_by> -> "by" expr
	// Rule 91: <opt_by> -> eps
	// Rule 92: <guards_inline> -> "if" expr <guards_inline>
	// Rule 93: <guards_inline> -> eps
	// Rule 94: <guards_multiline> -> "if" expr "EOL" <guards_multiline>
	// Rule 95: <guards_multiline> -> eps

	unsigned line_num = p->curr_tkn.line_num;

	// "for"
	if (!expect_keyword(p, KW_FOR)) {
		return NULL;
	}

	ast_node_t for_node = ast_create_node(NODE_FOR, line_num);
	if (!for_node) {
		return NULL;
	}
	ast_list_init(&for_node->as.for_stmt.guards);

	// <for_header>
	bool is_multiline = false;
	if (match_token_type(p, TOKEN_EOL)) {
		// Rule 86: "EOL" "indent" <for_generator> "EOL" <guards_multiline> "dedent"
		is_multiline = true;
		// "indent"
		if (!expect_token_type(p, TOKEN_INDENT)) {
			ast_dtor(for_node);
			return NULL;
		}
	}

	// <for_generator>
	// identifier
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected loop variable identifier");
		ast_dtor(for_node);
		return NULL;
	}
	char *var_name = strdup(p->curr_tkn.attribute.string_val);
	if (!var_name) {
		ast_dtor(for_node);
		return NULL;
	}
	for_node->as.for_stmt.var_name = var_name;
	parser_advance(p);

	// "<-"
	if (!expect_operator(p, OPERATOR_LEFT_ARROW)) {
		ast_dtor(for_node);
		return NULL;
	}

	// expr
	ast_node_t from_expr = parse_expression(p);
	if (!from_expr) {
		ast_dtor(for_node);
		return NULL;
	}
	for_node->as.for_stmt.from_expr = from_expr;

	// <range_type>
	// Rule 88: <range_type> -> "to" expr <opt_by>
	// Rule 89: <range_type> -> "until" expr
	if (match_specific_id(p, "to")) {
		for_node->as.for_stmt.is_inclusive = true;
		// expr (to)
		ast_node_t to_expr = parse_expression(p);
		if (!to_expr) {
			ast_dtor(for_node);
			return NULL;
		}
		for_node->as.for_stmt.to_expr = to_expr;

		// <opt_by>
		// Rule 90: <opt_by> -> "by" expr
		// Rule 91: <opt_by> -> eps
		if (match_specific_id(p, "by")) {
			// expr (step)
			ast_node_t step_expr = parse_expression(p);
			if (!step_expr) {
				ast_dtor(for_node);
				return NULL;
			}
			for_node->as.for_stmt.step_expr = step_expr;
		}
	} else if (match_specific_id(p, "until")) {
		for_node->as.for_stmt.is_inclusive = false;
		// expr (to)
		ast_node_t to_expr = parse_expression(p);
		if (!to_expr) {
			ast_dtor(for_node);
			return NULL;
		}
		for_node->as.for_stmt.to_expr = to_expr;
	} else {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected 'to' or 'until' in for range");
		ast_dtor(for_node);
		return NULL;
	}

	// Guards
	if (is_multiline) {
		// "EOL" after <for_generator>
		if (!expect_token_type(p, TOKEN_EOL)) {
			ast_dtor(for_node);
			return NULL;
		}

		// Rule 94: <guards_multiline> -> "if" expr "EOL" <guards_multiline>
		// Rule 95: <guards_multiline> -> eps
		while (match_keyword(p, KW_IF)) {
			// expr
			ast_node_t guard = parse_expression(p);
			if (!guard) {
				ast_dtor(for_node);
				return NULL;
			}
			// "EOL"
			if (!expect_token_type(p, TOKEN_EOL)) {
				ast_dtor(guard);
				ast_dtor(for_node);
				return NULL;
			}
			if (!ast_list_append(&for_node->as.for_stmt.guards, guard)) {
				ast_dtor(guard);
				ast_dtor(for_node);
				return NULL;
			}
		}

		// "dedent"
		if (!expect_token_type(p, TOKEN_DEDENT)) {
			ast_dtor(for_node);
			return NULL;
		}
	} else {
		// Rule 92: <guards_inline> -> "if" expr <guards_inline>
		// Rule 93: <guards_inline> -> eps
		while (match_keyword(p, KW_IF)) {
			// expr
			ast_node_t guard = parse_expression(p);
			if (!guard) {
				ast_dtor(for_node);
				return NULL;
			}
			if (!ast_list_append(&for_node->as.for_stmt.guards, guard)) {
				ast_dtor(guard);
				ast_dtor(for_node);
				return NULL;
			}
		}
	}

	// "do"
	if (!expect_keyword(p, KW_DO)) {
		ast_dtor(for_node);
		return NULL;
	}

	// <branch_body>
	ast_node_t body = parse_branch_body(p);
	if (!body) {
		ast_dtor(for_node);
		return NULL;
	}
	for_node->as.for_stmt.body = body;

	return for_node;
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
	// Rule 21: <complex_stmt> -> <while_stmt>
	// Rule 22: <complex_stmt> -> <cond_stmt>
	// Rule 23: <complex_stmt> -> <for_stmt>
	if (peek_keyword(p, KW_WHILE)) {
		// <while_stmt>
		return parse_while_stmt(p);
	}
	if (peek_keyword(p, KW_IF)) {
		// <cond_stmt>
		return parse_cond_stmt(p);
	}
	if (peek_keyword(p, KW_FOR)) {
		// <for_stmt>
		return parse_for_stmt(p);
	}

	print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected complex statement");
	return NULL;
}

ast_node_t parse_param_list(parser_t *p) {
	// TODO: Rule 34-41
	// <param_list> -> "(" <param_list_opt> ")"
	(void)p;
	return nullptr;
}

char *parse_type(parser_t *p) {
	// Rule 71: <type> -> "Int"
	// Rule 72: <type> -> "Double"
	// Rule 73: <type> -> "String"
	// Rule 74: <type> -> "Unit"
	// Rule 75: <type> -> "Boolean"

	// "Int"
	if (match_keyword(p, KW_INT)) {
		return strdup("Int");
	}
	// "Double"
	if (match_keyword(p, KW_DOUBLE)) {
		return strdup("Double");
	}
	// "String"
	if (match_keyword(p, KW_STRING)) {
		return strdup("String");
	}
	// "Unit"
	if (match_keyword(p, KW_UNIT)) {
		return strdup("Unit");
	}
	// "Boolean"
	if (match_keyword(p, KW_BOOLEAN)) {
		return strdup("Boolean");
	}

	print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected data type");
	return NULL;
}
