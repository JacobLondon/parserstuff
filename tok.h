#ifndef JLIB_TOK_H
#define JLIB_TOK_H

#include <stddef.h>

typedef struct Tokenizer Tokenizer;

typedef struct {
	char *symbol;
	int len;
	int id;
} Token;

/**
 * Return the next token using tokenizer_next to get next characters.
 * 
 * \warning
 *   Ensure an EOF token's symbol is NULL
 */
typedef Token (* tokenizer_gettok_func)(struct Tokenizer *t);

struct Tokenizer {
	char *cursor; // current position
	char *front; // yes just read the whole file
	size_t lineno;
	size_t colno;
	size_t len;
	tokenizer_gettok_func gettok;
	int _state;
};

void token_puts(Token *self);

/**
 * Initialize a tokenizer
 * 
 * \warning
 *   the \a text must be SHARED with the tokenizer
 */
void tokenizer_init(char *text, size_t len, tokenizer_gettok_func gettok, Tokenizer *out);

/**
 * Clear a tokenizer
 */
void tokenizer_cleanup(Tokenizer *self);

/**
 * Reset the tokenizer's state. Must be done before tokenizing a second time
 */
void tokenizer_reset(Tokenizer *self);

/**
 * Get the next token, symbol is NULL @ EOF
 */
Token tokenizer_gettok(Tokenizer *self);

/**
 * Get the next char, 0 on EOF
 */
char tokenizer_next(Tokenizer *self);

char tokenizer_peek(Tokenizer *self);

/**
 * Get the number of tokens in the buffer, can use
 * to allocate a buffer for tokens
 */
size_t tokenizer_count(Tokenizer *self);

#endif // JLIB_TOK_H
