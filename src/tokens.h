/**
 * tokens.h
 * Implementation IFJ-26, 30.9.2026
 * Autor: Michal Sobek, xsobekm00, VUT FIT
 * Translated: gcc 13.3.0
 * header file for tokens
 */

#ifndef TOKENS_H
#define TOKENS_H

/**
 * @brief we are differentiating between these 8 token types (for now :D)
 */
typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT,       // number containing exactly one '.'
    TOKEN_STRING,      // " ",""" """
    TOKEN_OPERATOR,    // \b(+|-|*|/|=|<|>|<=|>=|==|!=)\b
    TOKEN_KEYWORD,     // \b(def|do|int|...)\b check project spec for all keywords
    TOKEN_PUNCTUATION, // [()\{\}\[\];,\t]
    TOKEN_UNDERSLASH,
    TOKEN_EOF,
} token_type;

/**
 * @brief token struct --- abstract representation of a lexeme [lexeme_type, lexeme]
 */
typedef struct
{
    token_type type;
    char *lexeme;
} token;

/**
 * @brief a list of keywords defined in the project spec, used in check_keyword()
 */
extern const char *keywords[];

/**
 * @brief a list of operators defined in the project spec used, in check_operator_validity()
 */
extern const char *operators[];

/**
 * @brief token creation & storage on heap
 * @param type - type of the token (op, id, number, str,...)
 * @param lexeme - lexeme to be analyzed
 * @return a pointer to the token type for later analysation
 */
token *create_token(token_type type, char *lexeme);

/**
 * @brief free the allocated memory of token
 * @param token pointer of token to be freed
 */
void free_token(token *token);

/**
 * @brief func to determine if the lexeme is an identifier or an keyword
 * @param lexeme - lexeme to be checked
 */
void check_keyword(char *lexeme);

/**
 * @brief helper func to check if a operator token has a valid value
 * @param lexeme - operator lexeme to be validated
 */
void check_operator_validity(char *lexeme);

#endif