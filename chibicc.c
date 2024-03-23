#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Checks that the current token is TK_PUNCT and matches `s`.
static Token *is_punct(Token* tok, char *s) {
  if (tok->kind != TK_PUNCT || memcmp(tok->loc, s, tok->len) != 0 || 
      s[tok->len] != '\0')
    return NULL;
  return tok;
}

// Checks that the current token is TK_NUM.
static Token *is_number(Token *tok) {
  if (tok->kind != TK_NUM)
    return NULL;
  return tok;
}

// Creates a new token. 
static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

// Tokenizes `p` and returns new tokens. 
Token *tokenize(char *p) {
  Token head = {};
  Token *cur = &head;

  while (*p) {
    // Skip whitespace characters. 
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Tokenize numeric literals. 
    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // Tokenize punctuators. 
    if (*p == '+' || *p == '-') {
      cur = cur->next = new_token(TK_PUNCT, p, p+1);
      p++;
      continue;
    }

    error("invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token *tok = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // The first token must be a number
  if (!is_number(tok))
    error("expected a number");
  printf("  mov rax, %d\n", tok->val);
  tok = tok->next;

  // ... followed by either `+ <number>` or `- <number>`. 
  while (tok->kind != TK_EOF) {
    if (is_punct(tok, "+")) {
      tok = tok->next;
      if (!is_number(tok))
        error("expected a number");
      printf("  add rax, %d\n", tok->val);
      tok = tok->next;
      continue;
    }

    if (is_punct(tok, "-")) {
      tok = tok->next;
      if (!is_number(tok))
        error("expected a number");
      printf("  sub rax, %d\n", tok->val);
      tok = tok->next;
      continue;
    }

    error("unexpected token");
  }

  printf("  ret\n");
  return 0;
}
