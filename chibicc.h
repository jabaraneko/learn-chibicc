#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- tokenize.c ---

typedef enum {
  TK_PUNCT, // punctuators
  TK_NUM,   // numeric literals
  TK_EOF,   // end-of-file markers
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind; // token kind
  Token *next;    // next token
  int val;        // token value for TK_NUM
  char *loc;      // token location
  int len;        // token length
};

// Reports an error and exit. 
void error(char *fmt, ...);
void verror_at(char *p, char *loc, char *fmt, va_list ap);
void error_at(char *loc, char *fmt, ...);

// Checks that the current token is a specified type of token. 
Token *is_token_punct(Token* tok, char *s);
Token *is_token_number(Token *tok);
Token *is_token_eof(Token *tok);

// Tokenizes `p` and returns new tokens. 
Token *tokenize(char *p);

// --- parse.c ---

typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // / 
  ND_NEG,       // unary -
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_EXPR_STMT, // expression statement
  ND_NUM,       // integer
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind; // node kind
  Node *next;    // next node
  Node *lhs;     // left-hand side
  Node *rhs;     // right-hand side
  int val;       // node value for ND_NUM
};

// Parses tokens. 
Node *parse(Token *tok);

// --- codegen.c ---

// Generates assembly code. 
void codegen(Node *node);
