#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Source string
static char *src;

// --- Error handling --- 

// Reports an error and exit. 
static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit. 
static void verror_at(char *p, char *loc, char *fmt, va_list ap) {
  int pos = loc - p;
  fprintf(stderr, "%s\n", p);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(src, loc, fmt, ap);
}

// --- Tokenizer ---

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

// Creates a new token. 
// Members of the token must be set. 
static Token *new_token(TokenKind kind) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  return tok;
}

// Checks that the current token is TK_PUNCT and matches `s`.
static Token *is_token_punct(Token* tok, char *s) {
  if (tok->kind != TK_PUNCT || memcmp(tok->loc, s, tok->len) != 0 || 
      s[tok->len] != '\0')
    return NULL;
  return tok;
}

// Checks that the current token is TK_NUM.
static Token *is_token_number(Token *tok) {
  if (tok->kind != TK_NUM)
    return NULL;
  return tok;
}

// Checks that the current token is TK_EOF.
static Token *is_token_eof(Token *tok) {
  if (tok->kind != TK_EOF)
    return NULL;
  return tok;
}

// Tokenizes `src` and returns new tokens. 
Token *tokenize() {
  char *p = src;
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
      cur = cur->next = new_token(TK_NUM);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->loc = q;
      cur->len = p - q;
      continue;
    }

    // Tokenize punctuators. 
    if (ispunct(*p)) {
      cur = cur->next = new_token(TK_PUNCT);
      cur->loc = p;
      cur->len = 1;
      p++;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF);
  cur->loc = p;
  cur->len = 0;
  return head.next;
}

// --- Parser ---

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // / 
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind; // node kind
  Node *lhs;     // left-hand side
  Node *rhs;     // right-hand side
  int val;       // node value for ND_NUM
};

// Creates a new node. 
// Members of the node must be set. 
static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *expr(Token **tok);
static Node *mul(Token **tok);
static Node *primary(Token **tok);

// expr = mul ("+" mul | "-" mul)*
static Node *expr(Token **tok) {
  Node *node = mul(tok);

  for (;;) {
    if (is_token_punct(*tok, "+")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_ADD);
      tmp->lhs = node;
      tmp->rhs = mul(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, "-")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_SUB);
      tmp->lhs = node;
      tmp->rhs = mul(tok);
      node = tmp;
      continue;
    }

    return node;
  }
}

// mul = primary ("*" primary | "/" primary)*
static Node *mul(Token **tok) {
  Node *node = primary(tok);

  for (;;) {
    if (is_token_punct(*tok, "*")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_MUL);
      tmp->lhs = node;
      tmp->rhs = primary(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, "/")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_DIV);
      tmp->lhs = node;
      tmp->rhs = primary(tok);
      node = tmp;
      continue;
    }

    return node;
  }
}

// primary = "(" expr ")" | num
static Node *primary(Token **tok) {
  if (is_token_punct(*tok, "(")) {
    *tok = (*tok)->next;
    Node *node = expr(tok);
    if (!is_token_punct(*tok, ")")) 
      error_at((*tok)->loc, "expected \")\"");
    *tok = (*tok)->next;
    return node;
  }

  if (is_token_number(*tok)) {
    Node *node = new_node(ND_NUM);
    node->val = (*tok)->val;
    *tok = (*tok)->next;
    return node;
  }

  error_at((*tok)->loc, "expected an expression");
}

// --- Code generator ---

static void gen_expr(Node *node) {
  // Convert terminal symbols. 
  if (node->kind == ND_NUM) {
    printf("  mov rax, %d\n", node->val);
    return;
  }

  gen_expr(node->rhs);
  printf("  push rax\n");
  gen_expr(node->lhs);
  printf("  pop rdi\n");

  // Convert non-terminal symbols.
  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      return;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      return;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      return;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      return;
  }

  error("invalid expression");
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments\n", argv[0]);

  // Initialize global variables. 
  src = argv[1];

  // Tokenize and parse.
  Token *tok = tokenize(src);
  Node *node = expr(&tok);

  if (!is_token_eof(tok))
    error_at(tok->loc, "extra token");

  // Prologue
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Traverse the AST to emit assembly. 
  gen_expr(node);

  // Epilogue
  printf("  ret\n");

  return 0;
}
