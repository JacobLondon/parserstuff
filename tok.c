#include <assert.h>
#include <memory.h>
#include <stdio.h>

#include "tok.h"

void token_puts(Token *self)
{
	assert(self);
	(void)fprintf(stdout, "%d :: '%.*s'\n", self->id, self->len, self->symbol);
}

void tokenizer_init(char *text, size_t len, tokenizer_gettok_func gettok, Tokenizer *out)
{
	assert(text);
	assert(gettok);
	assert(out);

	(void)memset(out, 0, sizeof(*out));
	out->front = text;
	out->len = len;
	out->gettok = gettok;
	tokenizer_reset(out);
}

void tokenizer_cleanup(Tokenizer *self)
{
	assert(self);
	(void)memset(self, 0, sizeof(*self));
}

void tokenizer_reset(Tokenizer *self)
{
	assert(self);

	self->cursor = self->front;
	self->lineno = 1;
	self->colno = 1;
	self->_state = 0;
}

Token tokenizer_gettok(Tokenizer *self)
{
	assert(self);
	return self->gettok(self);
}

char tokenizer_next(Tokenizer *self)
{
	assert(self);
	const char ch = self->cursor[0];

	// no more
	if (ch == 0) {
		return 0;
	}
	// user says we have no more
	else if ((size_t)(self->cursor - self->front) >= (size_t)self->len) {
		return 0;
	}
	else if (ch == '\r') {
		self->colno = 1;
	}
	else if (ch == '\n') {
		self->colno = 1;
		self->lineno++;
	}
	else
	{
		self->colno++;
	}

	switch (self->_state)
	{
	// normal case
	case 1:
		self->cursor++;
		break;
	// first case
	case 0:
		self->_state = 1;
		break;
	default:
		assert(0);
		break;
	}

	return self->cursor[0];
}

char tokenizer_peek(Tokenizer *self)
{
	assert(self);

	if (self->cursor[0] == 0) {
		return 0;
	}
	return self->cursor[1];
}

size_t tokenizer_count(Tokenizer *self)
{
	size_t i = 0;
	Token tok;

	assert(self);
	tokenizer_reset(self);

	for (tok = tokenizer_gettok(self);
	     tok.symbol != NULL;
	     tok = tokenizer_gettok(self))
	{
		i++;
	}

	tokenizer_reset(self);
	return i;
}
