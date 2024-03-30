#include "chibicc.h"

// Source string
static char *src;

// Creates a new token. 
// Members of the token must be set. 
static Token *new_token(TokenKind kind) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  return tok;
}

// Checks that `p` starts with `q`. 
static bool startswith(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// Reads a punctuator token from `p` and returns its length.
static int read_punct(char *p) {
  if (startswith(p, "==") || startswith(p, "!=") ||
      startswith(p, "<=") || startswith(p, ">="))
    return 2;

  return ispunct(*p) ? 1 : 0;
}

// Returns true if `c` is valid as the first character of an identifier.
static bool is_ident1(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if `c` is valid as a non-first character of an identifier.
static bool is_ident2(char c) {
  return is_ident1(c) || ('0' <= c && c <= '9');
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void verror_at(char *p, char *loc, char *fmt, va_list ap) {
  int pos = loc - p;
  fprintf(stderr, "%s\n", p);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(src, loc, fmt, ap);
}

Token *is_token_punct(Token* tok, char *s) {
  if (tok->kind != TK_PUNCT || memcmp(tok->loc, s, tok->len) != 0 || 
      s[tok->len] != '\0')
    return NULL;
  return tok;
}

Token *is_token_number(Token *tok) {
  if (tok->kind != TK_NUM)
    return NULL;
  return tok;
}

Token *is_token_ident(Token* tok) {
  if (tok->kind != TK_IDENT)
    return NULL;
  return tok;
}

Token *is_token_eof(Token *tok) {
  if (tok->kind != TK_EOF)
    return NULL;
  return tok;
}

Token *tokenize(char *p) {
  src = p;
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

    // Tokenize identifiers. 
    if (is_ident1(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident2(*p));
      cur = cur->next = new_token(TK_IDENT);
      cur->loc = start;
      cur->len = p-start;
      continue;
    }

    // Tokenize punctuators. 
    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->next = new_token(TK_PUNCT);
      cur->loc = p;
      cur->len = punct_len;
      p += punct_len;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF);
  cur->loc = p;
  cur->len = 0;
  return head.next;
}
