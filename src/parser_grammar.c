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
	// Rule 2: <prologue> -> "import" "ifj26" "." "*" "EOL"

	// "import"
	if (!expect_keyword(p, KW_IMPORT)) {
		return NULL;
	}

	// "ifj26"
	if (!expect_specific_id(p, "ifj26")) {
		return NULL;
	}

	// "."
	if (!expect_operator(p, OPERATOR_DOT)) {
		return NULL;
	}

	// "*"
	if (!expect_operator(p, OPERATOR_STAR)) {
		return NULL;
	}

	// "EOL"
	if (!expect_token_type(p, TOKEN_EOL)) {
		return NULL;
	}

	return p->root;
}

ast_node_t parse_top_def_seq(parser_t *p) {
	// Rule 3: <top_def_seq> -> <top_def> <top_def_seq>
	// Rule 4: <top_def_seq> -> eps

	while (match_token_type(p, TOKEN_EOL))
		; // skipping EOLs

	// Rule 3: <top_def_seq> -> <top_def> <top_def_seq>
	if (peek_token_type(p, TOKEN_MAIN) || peek_keyword(p, KW_DEF) ||
		peek_keyword(p, KW_EXTENSION)) {

		// <top_def>
		ast_node_t def = parse_top_def(p);
		if (!def) {
			return NULL;
		}

		ast_list_append(&p->root->as.program.items, def);

		// <top_def_seq>
		return parse_top_def_seq(p);
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
	// Rule 6: <top_def> -> <ext_def>

	// Rule 5: <top_def> -> <func_def>
	if (peek_token_type(p, TOKEN_MAIN) || peek_keyword(p, KW_DEF)) {
		// <func_def>
		return parse_func_def(p);
	}
	// Rule 6: <top_def> -> <ext_def>
	else if (peek_keyword(p, KW_EXTENSION)) {
		// <ext_def>
		return parse_ext_def(p);
	}

	return NULL;
}

ast_node_t parse_func_def(parser_t *p) {
	// Rule 24: <func_def> -> <opt_main> "def" identifier <param_list_seq> <opt_type> "="
	// <func_body> Rule 25: <opt_main> -> "@main" <opt_eol> Rule 26: <opt_main> -> eps Rule 27:
	// <param_list_seq> -> <param_list> <param_list_seq> Rule 28: <param_list_seq> -> eps Rule 29:
	// <opt_type> -> ":" <type> Rule 30: <opt_type> -> eps

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
	if (!func_id) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
		return NULL;
	}
	parser_advance(p);

	// creating the node
	ast_node_t func_node = ast_create_node(NODE_FUNC_DEF, p->curr_tkn.line_num);
	if (!func_node) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate AST node");
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
		// <param_list>
		ast_node_t plist = parse_param_list(p);
		if (!plist) {
			ast_dtor(func_node);
			return NULL;
		}
		ast_list_append(&func_node->as.func_def.param_lists, plist);
	}
	func_node->as.func_def.has_parens = has_parens;

	// <opt_type>
	// Rule 29: <opt_type> -> ":" <type>
	// Rule 30: <opt_type> -> eps
	if (match_operator(p, OPERATOR_COLON)) {
		// <type>
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
	// Rule 31: <func_body> -> "EOL" <block>
	// Rule 32: <func_body> -> expr "EOL"

	// "EOL"
	if (match_token_type(p, TOKEN_EOL)) {
		// <block>
		return parse_block(p);
	}

	// expr
	ast_node_t expr = parse_expression(p);
	if (!expr) {
		return NULL;
	}

	// "EOL"
	if (!expect_token_type(p, TOKEN_EOL)) {
		ast_dtor(expr);
		return NULL;
	}

	return expr;
}

ast_node_t parse_ext_def(parser_t *p) {
	// Rule 54: <ext_def> -> "extension" "(" identifier ":" <type> ")" "EOL" "indent"
	// <ext_method_seq> "dedent" Rule 55: <ext_method_seq> -> <ext_method> <ext_method_seq_tail>
	// Rule 56: <ext_method_seq_tail> -> <ext_method> <ext_method_seq_tail>
	// Rule 57: <ext_method_seq_tail> -> eps
	// Rule 58: <ext_method> -> "def" identifier <param_list_seq> <opt_type> "=" <func_body>

	ast_node_t ext_node = ast_create_node(NODE_EXTENSION, p->curr_tkn.line_num);
	if (!ext_node) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate AST node");
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
	if (!ext_reciever) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
		ast_dtor(ext_node);
		return NULL;
	}
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

	// <ext_method_seq>
	// Rule 55: <ext_method_seq> -> <ext_method> <ext_method_seq_tail>
	// Rule 58: <ext_method> -> "def" identifier <param_list_seq> <opt_type> "=" <func_body>
	while (peek_keyword(p, KW_DEF)) {
		// <ext_method>
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
	// Rule 7: <block> -> "indent" <line> <line_seq> <opt_eol> "dedent"

	// "indent"
	if (!expect_token_type(p, TOKEN_INDENT)) {
		return NULL;
	}

	// creating the node
	ast_node_t block_node = ast_create_node(NODE_BLOCK, p->curr_tkn.line_num);
	if (!block_node) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate AST node");
		return NULL;
	}
	ast_list_init(&block_node->as.block.statements);

	// <line>
	if (!parse_line(p, block_node)) {
		ast_dtor(block_node);
		return NULL;
	}

	// <line_seq>
	// Rule 13: <line_seq> -> "EOL" <line> <line_seq>
	// Rule 14: <line_seq> -> eps
	// A line directly after a nested block has no EOL, the block took it, so that is the only
	// place where a line may start without one. Otherwise "a = 1 b = 2" would be accepted.
	while (peek_token_type(p, TOKEN_EOL) ||
		   (peek_line_beg(p) && p->prev_tkn_type == TOKEN_DEDENT)) {
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
	// Rule 15: <opt_eol> -> "EOL"
	// Rule 16: <opt_eol> -> eps
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
	// Rule 8: <line> -> <simple_seq>
	// Rule 9: <line> -> <complex_stmt>
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
			print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to append to a list");
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
				print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to append to a list");
				ast_dtor(sstmt);
				return false;
			}
		}
		return true;
	}
}

ast_node_t parse_cond_stmt(parser_t *p) {
	// Rule 76: <cond_stmt> -> "if" expr "then" <branch_body> <opt_else>
	// Rule 79: <opt_else> -> <opt_eol> "else" <else_branch>
	// Rule 80: <opt_else> -> eps
	// Rule 81: <else_branch> -> <cond_stmt>
	// Rule 82: <else_branch> -> <branch_body>

	unsigned line_num = p->curr_tkn.line_num;

	// "if"
	if (!expect_keyword(p, KW_IF)) {
		return NULL;
	}

	// creating node
	ast_node_t if_node = ast_create_node(NODE_IF, line_num);
	if (!if_node) {
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
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
	// Rule 79: <opt_else> -> <opt_eol> "else" <else_branch>
	// Rule 80: <opt_else> -> eps
	if (peek_token_type(p, TOKEN_EOL) && peek_next_keyword(p, KW_ELSE)) {
		// <opt_eol>
		match_token_type(p, TOKEN_EOL);
	}

	// "else"
	if (match_keyword(p, KW_ELSE)) {
		// Rule 81: <else_branch> -> <cond_stmt>
		// Rule 82: <else_branch> -> <branch_body>
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
		// Rule 80: <opt_else> -> eps
		if_node->as.if_stmt.else_branch = NULL;
	}

	return if_node;
}

ast_node_t parse_branch_body(parser_t *p) {
	// Rule 77: <branch_body> -> "EOL" <block>
	// Rule 78: <branch_body> -> <simple_stmt>

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
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
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
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
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
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
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
				print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to append to a list");
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
				print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to append to a list");
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
	// Rule 67: <var_def> -> <var_kw> identifier <opt_type> "=" expr
	// Rule 68: <var_kw> -> "val"
	// Rule 69: <var_kw> -> "var"
	// Rule 29: <opt_type> -> ":" <type>
	// Rule 30: <opt_type> -> eps

	unsigned line_num = p->curr_tkn.line_num;

	// <var_kw>
	bool is_val = false;
	if (match_keyword(p, KW_VAL)) {
		is_val = true;
	} else if (match_keyword(p, KW_VAR)) {
		is_val = false;
	} else {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected 'val' or 'var'");
		return NULL;
	}

	ast_node_t var_node = ast_create_node(NODE_VAR_DEF, line_num);
	if (!var_node) {
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
		return NULL;
	}
	var_node->as.var_def.is_val = is_val;

	// identifier
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected variable name");
		ast_dtor(var_node);
		return NULL;
	}
	char *name = strdup(p->curr_tkn.attribute.string_val);
	if (!name) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
		ast_dtor(var_node);
		return NULL;
	}
	var_node->as.var_def.name = name;
	parser_advance(p);

	// <opt_type>
	// Rule 29: <opt_type> -> ":" <type>
	// Rule 30: <opt_type> -> eps
	if (match_operator(p, OPERATOR_COLON)) {
		// <type>
		char *type_name = parse_type(p);
		if (!type_name) {
			ast_dtor(var_node);
			return NULL;
		}
		var_node->as.var_def.type_name = type_name;
	} else {
		var_node->as.var_def.type_name = NULL;
	}

	// "="
	if (!expect_operator(p, OPERATOR_ASSIGN)) {
		ast_dtor(var_node);
		return NULL;
	}

	// expr
	ast_node_t init_expr = parse_expression(p);
	if (!init_expr) {
		ast_dtor(var_node);
		return NULL;
	}
	var_node->as.var_def.init_expr = init_expr;

	return var_node;
}

ast_node_t parse_assign_stmt(parser_t *p) {
	// Rule 70: <assign_stmt> -> identifier "=" expr

	unsigned line_num = p->curr_tkn.line_num;

	// identifier
	if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
		print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected target identifier");
		return NULL;
	}

	char *target_id = strdup(p->curr_tkn.attribute.string_val);
	if (!target_id) {
		print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
		return NULL;
	}
	parser_advance(p);

	ast_node_t assign_node = ast_create_node(NODE_ASSIGN, line_num);
	if (!assign_node) {
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
		free(target_id);
		return NULL;
	}
	assign_node->as.assign.target_id = target_id;

	// "="
	if (!expect_operator(p, OPERATOR_ASSIGN)) {
		ast_dtor(assign_node);
		return NULL;
	}

	// expr
	ast_node_t expr = parse_expression(p);
	if (!expr) {
		ast_dtor(assign_node);
		return NULL;
	}
	assign_node->as.assign.expr = expr;

	return assign_node;
}

ast_node_t parse_return_stmt(parser_t *p) {
	// Rule 51: <return_stmt> -> "return" <opt_expr>
	// Rule 52: <opt_expr> -> expr
	// Rule 53: <opt_expr> -> eps

	unsigned line_num = p->curr_tkn.line_num;

	// "return"
	if (!expect_keyword(p, KW_RETURN)) {
		return NULL;
	}

	ast_node_t return_node = ast_create_node(NODE_RETURN, line_num);
	if (!return_node) {
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
		return NULL;
	}

	// <opt_expr>
	// Rule 52: <opt_expr> -> expr
	// Rule 53: <opt_expr> -> eps
	if (peek_operator(p, OPERATOR_SEMICOLON) || peek_token_type(p, TOKEN_EOL) ||
		peek_token_type(p, TOKEN_DEDENT) || peek_token_type(p, TOKEN_EOF)) {
		return_node->as.return_stmt.expr = NULL;
	} else {
		// expr
		ast_node_t expr = parse_expression(p);
		if (!expr) {
			ast_dtor(return_node);
			return NULL;
		}
		return_node->as.return_stmt.expr = expr;
	}

	return return_node;
}

ast_node_t parse_simple_stmt(parser_t *p) {
	// Rule 17: <simple_stmt> -> <var_def>
	// Rule 18: <simple_stmt> -> <assign_stmt>
	// Rule 19: <simple_stmt> -> <return_stmt>
	// Rule 20: <simple_stmt> -> expr

	// <var_def>
	if (peek_keyword(p, KW_VAL) || peek_keyword(p, KW_VAR)) {
		return parse_var_def(p);
	}

	// <return_stmt>
	if (peek_keyword(p, KW_RETURN)) {
		return parse_return_stmt(p);
	}

	// <assign_stmt>
	if (peek_token_type(p, TOKEN_IDENTIFIER) && peek_next_operator(p, OPERATOR_ASSIGN)) {
		return parse_assign_stmt(p);
	}

	// expr
	if (peek_is_expr_beg(p)) {
		unsigned line_num = p->curr_tkn.line_num;
		ast_node_t expr = parse_expression(p);
		if (!expr) {
			return NULL;
		}

		ast_node_t stmt_node = ast_create_node(NODE_EXPR_STMT, line_num);
		if (!stmt_node) {
			print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
			ast_dtor(expr);
			return NULL;
		}
		stmt_node->as.expr_stmt.expr = expr;
		return stmt_node;
	}

	print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected simple statement");
	return NULL;
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
	// Rule 34: <param_list> -> "(" <param_list_opt> ")"
	// Rule 35: <param_list_opt> -> <param> <params_tail>
	// Rule 36: <param_list_opt> -> eps
	// Rule 37: <param> -> identifier ":" <type>
	// Rule 38: <params_tail> -> "," <params_tail_more>
	// Rule 39: <params_tail> -> eps
	// Rule 40: <params_tail_more> -> <param> <params_tail>
	// Rule 41: <params_tail_more> -> eps

	unsigned line_num = p->curr_tkn.line_num;

	// "("
	if (!expect_token_type(p, TOKEN_LPAREN)) {
		return NULL;
	}

	ast_node_t plist_node = ast_create_node(NODE_PARAM_LIST, line_num);
	if (!plist_node) {
		print_error(ERR_INTERNAL, line_num, "Failed to allocate AST node");
		return NULL;
	}
	ast_list_init(&plist_node->as.param_list.params);

	// <param_list_opt>
	if (peek_token_type(p, TOKEN_IDENTIFIER)) {
		do {
			// Rule 37: <param> -> identifier ":" <type>
			// identifier
			if (!peek_token_type(p, TOKEN_IDENTIFIER)) {
				print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected parameter name");
				ast_dtor(plist_node);
				return NULL;
			}
			char *param_name = strdup(p->curr_tkn.attribute.string_val);
			if (!param_name) {
				print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a name");
				ast_dtor(plist_node);
				return NULL;
			}
			unsigned param_line = p->curr_tkn.line_num;
			parser_advance(p);

			// ":"
			if (!expect_operator(p, OPERATOR_COLON)) {
				free(param_name);
				ast_dtor(plist_node);
				return NULL;
			}

			// <type>
			char *type_name = parse_type(p);
			if (!type_name) {
				free(param_name);
				ast_dtor(plist_node);
				return NULL;
			}

			ast_node_t param_node = ast_create_node(NODE_PARAM, param_line);
			if (!param_node) {
				print_error(ERR_INTERNAL, param_line, "Failed to allocate AST node");
				free(param_name);
				free(type_name);
				ast_dtor(plist_node);
				return NULL;
			}
			param_node->as.param.name = param_name;
			param_node->as.param.type_name = type_name;

			if (!ast_list_append(&plist_node->as.param_list.params, param_node)) {
				print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to append to a list");
				ast_dtor(param_node);
				ast_dtor(plist_node);
				return NULL;
			}

			// Rule 38: <params_tail> -> "," <params_tail_more>
			// Rule 39: <params_tail> -> eps
			unsigned comma_line = p->curr_tkn.line_num;
			if (!match_operator(p, OPERATOR_COMMA)) {
				break;
			}

			// Rule 41: <params_tail_more> -> eps (trailing comma before ')')
			if (peek_token_type(p, TOKEN_RPAREN)) {
				// only allowed if a line break separates the comma from ')'
				if (p->curr_tkn.line_num <= comma_line) {
					print_error(ERR_SYNTACTIC, p->curr_tkn.line_num,
								"A trailing comma must be followed by a line break");
					ast_dtor(plist_node);
					return NULL;
				}
				break;
			}
			// Rule 40: <params_tail_more> -> <param> <params_tail>
		} while (true);
	}

	// ")"
	if (!expect_token_type(p, TOKEN_RPAREN)) {
		ast_dtor(plist_node);
		return NULL;
	}

	return plist_node;
}

char *parse_type(parser_t *p) {
	// Rule 71: <type> -> "Int"
	// Rule 72: <type> -> "Double"
	// Rule 73: <type> -> "String"
	// Rule 74: <type> -> "Unit"
	// Rule 75: <type> -> "Boolean"

	const char *name = NULL;
	if (match_keyword(p, KW_INT)) {
		name = "Int";
	} else if (match_keyword(p, KW_DOUBLE)) {
		name = "Double";
	} else if (match_keyword(p, KW_STRING)) {
		name = "String";
	} else if (match_keyword(p, KW_UNIT)) {
		name = "Unit";
	} else if (match_keyword(p, KW_BOOLEAN)) {
		name = "Boolean";
	}

	if (name) {
		char *copy = strdup(name);
		if (!copy) {
			print_error(ERR_INTERNAL, p->curr_tkn.line_num, "Failed to allocate a type name");
		}
		return copy;
	}

	print_error(ERR_SYNTACTIC, p->curr_tkn.line_num, "Expected data type");
	return NULL;
}
