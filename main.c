#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "tok.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define EXHAUSTED() do { \
	fprintf(stderr, "Memory exhausted\n"); \
	exit(1); \
} while (0)

#define MISSING() do { \
	fprintf(stderr, "Missing token\n"); \
	exit(1); \
} while (0)

enum {
	TOK_EOF,
	TOK_NUM,

	_tok_begin_ops,
	TOK_ADD,
	TOK_MUL,
	TOK_POW,
	TOK_UNARY,
	_tok_end_ops,

	TOK_UNKN
};

typedef struct Node Node;
struct Node {
	Token token;
	Node *sibling;
	Node *children;
};

static Token gettok(Tokenizer *t);

static Node nodes[256];
static size_t node_ndx = 0;

static void node_insert(Token token)
{
	if (node_ndx >= ARRAY_SIZE(nodes)) {
		EXHAUSTED();
	}
	else if (node_ndx != 0) {
		nodes[node_ndx - 1].sibling = &nodes[node_ndx];
	}
	nodes[node_ndx++] = (Node){token, NULL, NULL};
}

static void node_puts(Node *node, int depth)
{
	if (!node) {
		return;
	}

	for (Node *current = node; current; current = current->sibling) {
		if (depth > 0) printf("%*c", 2 * depth, ' ');
		token_puts(&current->token);
		node_puts(node->children, depth + 1);
	}
}

// build a tree from the node, returning the root
static Node *node_recurse(Node *node)
{
	// left-most and top-most node
	Node *root = node;

	// highest precedence first
	for (int id = _tok_end_ops - 1; id > _tok_begin_ops; id--) {
		Node *next = NULL;
		for (Node *current = root; current; current = current->sibling) {

			// all infix operators (postfix needs to not check for next->sibling != NULL)
			next = current->sibling;
			if (next && next->token.id == id) {
				if (next->sibling == NULL) MISSING();

				/*
				 * Current is val1
				 * next is infix operator
				 * next->sibling is val2
				 */
				Node *val1 = current;
				Node *op = next;
				Node *val2 = next->sibling;

				switch (id) {
				case TOK_ADD: // fallthrough
				case TOK_MUL: // fallthrough
				case TOK_POW:
					op->children = val1;
					val1->sibling = val2;
					op->sibling = val2->sibling;
					val2->sibling = NULL;
					current = op;
					break;
				default:
					MISSING();
				}

				if (root == val1 || root == val2) {
					root = op;
				}
			}

			// all prefix operators
			else if (current->token.id == id) {
				if (current->sibling == NULL) MISSING();

				/*
				 * Current is prefix operator
				 * next is val
				 */
				Node *op = current;
				Node *val = next;

				switch (id) {
				case TOK_UNARY:
					op->children = val;
					op->sibling = val->sibling;
					val->sibling = NULL;
					current = op;
					break;
				//case TOK_LPAREN:
				// recurse()
				default:
					MISSING();
				}

				if (root == val) {
					root = op;
				}
			}
		}
	}

	return root;
}

static Token gettok(Tokenizer *t)
{
	int ch, peek;
	char *begin;
	size_t len = 0;
	Token token;

	do {
		ch = tokenizer_next(t);
	} while (isspace(ch));

	begin = t->cursor;

	if (isalpha(ch)) {
		while (isalnum(ch)) {
			ch = tokenizer_next(t);
			len++;
		}

		token.id = TOK_UNARY;
		token.len = len;
		token.symbol = begin;
		return token;
	}

	else if (isdigit(ch)) {
		while (isdigit(ch)) {
			ch = tokenizer_next(t);
			len++;
		}

		token.id = TOK_NUM;
		token.len = len;
		token.symbol = begin;
		return token;
	}

	else if (ch == '*') {
		peek = tokenizer_peek(t);
		if (peek == '*') {
			(void)tokenizer_next(t);
			token.id = TOK_POW;
			token.len = 2;
			token.symbol = begin;
			return token;
		}
		token.id = TOK_MUL;
		token.len = 1;
		token.symbol = begin;
		return token;
	}

	else if (ch == '+') {
		token.id = TOK_ADD;
		token.len = 1;
		token.symbol = begin;
		return token;
	}

	else if (ch == '\0') {
		token.id = TOK_EOF;
		token.len = 1;
		token.symbol = NULL;
		return token;
	}

	token.id = TOK_UNKN;
	token.len = 1;
	token.symbol = begin;
	return token;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	Tokenizer tokenizer;
	Token current;
	char *test = "abs 55 + 2 ** 3 * 4";

	printf("%s\n", test);
	tokenizer_init(test, strlen(test), gettok, &tokenizer);

	while (1) {
		current = tokenizer_gettok(&tokenizer);
		if (current.id == TOK_EOF) {
			break;
		}
		else if (current.id == TOK_UNKN) {
			printf("Unknown token:");
			token_puts(&current);
			fflush(stdout);
			exit(1);
		}

		node_insert(current);
	}

	node_puts(nodes, 0);

	if (node_ndx == 0) {
		printf("No tokens found\n");
		exit(1);
	}
	printf("---------------------\n");
	Node *root = node_recurse(&nodes[0]);
	node_puts(root, 0);

	tokenizer_cleanup(&tokenizer);
	return 0;
}
