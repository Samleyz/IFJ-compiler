/**
 * @file parser_grammar.h
 * @author Jan Mejzlík (xmejzlj00@vutbr.cz)
 * @author Michal Sobek (xsobekm00@vutbr.cz)
 * @brief Declarations of grammar parsing functions (rules) for IFJ26 parser
 * @date 2026-10-22
 */

#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

#include "ast.h"
#include "parser_private.h"

/**
 * @brief Parses the program header rule (prologue)
 * Rule 2: <prologue> -> "import" "ifj26" "." "*" "EOL"
 * @param p Pointer to parser structure
 * @return AST node pointer (p->root on success), or NULL on error
 */
ast_node_t parse_header(parser_t *p);

/**
 * @brief Parses a sequence of top-level definitions until EOF
 * Rule 3: <top_def_seq> -> <top_def> <top_def_seq>
 * Rule 4: <top_def_seq> -> eps
 * Appends each parsed top-level definition to p->root->as.program.items.
 * @param p Pointer to parser structure
 * @return AST node pointer (p->root on success), or NULL on error
 */
ast_node_t parse_top_def_seq(parser_t *p);

/**
 * @brief Parses a single top-level definition (<func_def> or <ext_def>)
 * Rule 5: <top_def> -> <func_def>
 * Rule 6: <top_def> -> <ext_def>
 * @param p Pointer to parser structure
 * @return AST node pointer (NODE_FUNC_DEF or NODE_EXTENSION), or NULL on error
 */
ast_node_t parse_top_def(parser_t *p);

/**
 * @brief Parses a function definition
 * Rule 24: <func_def> -> <opt_main> "def" identifier <param_list_seq> <opt_type> "=" <func_body>
 * Rule 25: <opt_main> -> "@main" <opt_eol>
 * Rule 26: <opt_main> -> eps
 * @param p Pointer to parser structure
 * @return AST node representing NODE_FUNC_DEF, or NULL on error
 */
ast_node_t parse_func_def(parser_t *p);

/**
 * @brief Parses function body rule (<func_body>)
 * Rule 31: <func_body> -> "EOL" <block>
 * Rule 32: <func_body> -> expr "EOL"
 * @param p Pointer to parser structure
 * @return AST node representing block or expression, or nullptr on error
 */
ast_node_t parse_func_body(parser_t *p);

/**
 * @brief Parses an extension definition
 * Rule 54: <ext_def> -> "extension" "(" identifier ":" <type> ")" "EOL" "indent" <ext_method_seq>
 * "dedent" Rule 55: <ext_method_seq> -> <ext_method> <ext_method_seq_tail> Rule 56:
 * <ext_method_seq_tail> -> <ext_method> <ext_method_seq_tail> Rule 57: <ext_method_seq_tail> -> eps
 * Rule 58: <ext_method> -> "def" identifier <param_list_seq> <opt_type> "=" <func_body>
 * @param p Pointer to parser structure
 * @return AST node representing NODE_EXTENSION, or NULL on error
 */
ast_node_t parse_ext_def(parser_t *p);

/**
 * @brief Parses a block rule (indented block of statements)
 * Rule 7: <block> -> "indent" <line> <line_seq> <opt_eol> "dedent"
 * @param p Pointer to parser structure
 * @return AST node representing NODE_BLOCK, or NULL on error
 */
ast_node_t parse_block(parser_t *p);

/**
 * @brief Parses a conditional statement (if-then-else)
 * Rule 76: <cond_stmt> -> "if" expr "then" <branch_body> <opt_else>
 * Rule 79: <opt_else> -> <opt_eol> "else" <else_branch>
 * Rule 80: <opt_else> -> eps
 * Rule 81: <else_branch> -> <cond_stmt>
 * Rule 82: <else_branch> -> <branch_body>
 * @param p Pointer to parser structure
 * @return AST node representing NODE_IF, or NULL on error
 */
ast_node_t parse_cond_stmt(parser_t *p);

/**
 * @brief Alias for parse_cond_stmt for backward compatibility
 * @param p Pointer to parser structure
 * @return AST node representing NODE_IF, or NULL on error
 */
ast_node_t parse_if_statement(parser_t *p);

/**
 * @brief Parses a while loop statement
 * Rule 83: <while_stmt> -> "while" expr "do" "EOL" <block>
 * @param p Pointer to parser structure
 * @return AST node representing NODE_WHILE, or NULL on error
 */
ast_node_t parse_while_stmt(parser_t *p);

/**
 * @brief Parses a for loop statement
 * Rule 84: <for_stmt> -> "for" <for_header> "do" <branch_body>
 * Rule 85: <for_header> -> <for_generator> <guards_inline>
 * Rule 86: <for_header> -> "EOL" "indent" <for_generator> "EOL" <guards_multiline> "dedent"
 * @param p Pointer to parser structure
 * @return AST node representing NODE_FOR, or NULL on error
 */
ast_node_t parse_for_stmt(parser_t *p);

/**
 * @brief Parses a variable definition (val / var)
 * Rule 67: <var_def> -> <var_kw> identifier <opt_type> "=" expr
 * Rule 68: <var_kw> -> "val"
 * Rule 69: <var_kw> -> "var"
 * @param p Pointer to parser structure
 * @return AST node representing NODE_VAR_DEF, or NULL on error
 */
ast_node_t parse_var_def(parser_t *p);

/**
 * @brief Parses an assignment statement
 * Rule 70: <assign_stmt> -> identifier "=" expr
 * @param p Pointer to parser structure
 * @return AST node representing NODE_ASSIGN, or NULL on error
 */
ast_node_t parse_assign_stmt(parser_t *p);

/**
 * @brief Parses a return statement
 * Rule 51: <return_stmt> -> "return" <opt_expr>
 * Rule 52: <opt_expr> -> expr
 * Rule 53: <opt_expr> -> eps
 * @param p Pointer to parser structure
 * @return AST node representing NODE_RETURN, or NULL on error
 */
ast_node_t parse_return_stmt(parser_t *p);

/**
 * @brief Parses a simple statement (<var_def>, <assign_stmt>, <return_stmt>, or expr)
 * Rule 17: <simple_stmt> -> <var_def>
 * Rule 18: <simple_stmt> -> <assign_stmt>
 * Rule 19: <simple_stmt> -> <return_stmt>
 * Rule 20: <simple_stmt> -> expr
 * @param p Pointer to parser structure
 * @return AST node representing the statement, or NULL on error
 */
ast_node_t parse_simple_stmt(parser_t *p);

/**
 * @brief Parses a complex statement (<while_stmt>, <cond_stmt>, or <for_stmt>)
 * Rule 21: <complex_stmt> -> <while_stmt>
 * Rule 22: <complex_stmt> -> <cond_stmt>
 * Rule 23: <complex_stmt> -> <for_stmt>
 * @param p Pointer to parser structure
 * @return AST node representing the statement, or NULL on error
 */
ast_node_t parse_complex_stmt(parser_t *p);

/**
 * @brief Parses a parameter list in parentheses: "(" <param_list_opt> ")"
 * Rule 34: <param_list> -> "(" <param_list_opt> ")"
 * Rule 35: <param_list_opt> -> <param> <params_tail>
 * Rule 36: <param_list_opt> -> eps
 * Rule 37: <param> -> identifier ":" <type>
 * Rule 38: <params_tail> -> "," <params_tail_more>
 * Rule 39: <params_tail> -> eps
 * Rule 40: <params_tail_more> -> <param> <params_tail>
 * Rule 41: <params_tail_more> -> eps
 * @param p Pointer to parser structure
 * @return AST node representing NODE_PARAM_LIST, or NULL on error
 */
ast_node_t parse_param_list(parser_t *p);

/**
 * @brief Parses a data type keyword and returns its heap-allocated string representation
 * Rule 71: <type> -> "Int"
 * Rule 72: <type> -> "Double"
 * Rule 73: <type> -> "String"
 * Rule 74: <type> -> "Unit"
 * Rule 75: <type> -> "Boolean"
 * @param p Pointer to parser structure
 * @return Dynamically allocated string (e.g. "Int", "Double"), or NULL on error
 */
char *parse_type(parser_t *p);

#endif
