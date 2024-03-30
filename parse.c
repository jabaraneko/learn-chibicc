#include "chibicc.h"

static Node *stmt(Token **tok);
static Node *expr_stmt(Token **tok);
static Node *expr(Token **tok);
static Node *assign(Token **tok);
static Node *equality(Token **tok);
static Node *relational(Token **tok);
static Node *add(Token **tok);
static Node *mul(Token **tok);
static Node *unary(Token **tok);
static Node *primary(Token **tok);

// Creates a new node. 
// Members of the node must be set. 
static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// stmt = expr-stmt
static Node *stmt(Token **tok) {
  return expr_stmt(tok);
}

// expr-stmt = expr ";"
static Node *expr_stmt(Token **tok) {
  Node *node = new_node(ND_EXPR_STMT);
  node->lhs = expr(tok);
  if (is_token_punct(*tok, ";")) {
    *tok = (*tok)->next;
    return node;
  }

  error_at((*tok)->loc, "expected ;");
}

// expr = assign
static Node *expr(Token **tok) {
  return assign(tok);
}

// assign = equality ("=" assign)?
static Node *assign(Token **tok) {
  Node *node = equality(tok);

  if (is_token_punct(*tok, "=")) {
    *tok = (*tok)->next;
    Node *tmp = new_node(ND_ASSIGN);
    tmp->lhs = node;
    tmp->rhs = assign(tok);
    node = tmp;
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **tok) {
  Node *node = relational(tok);

  for (;;) {
    if (is_token_punct(*tok, "==")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_EQ);
      tmp->lhs = node;
      tmp->rhs = relational(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, "!=")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_NE);
      tmp->lhs = node;
      tmp->rhs = relational(tok);
      node = tmp;
      continue;
    }

    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **tok) {
  Node *node = add(tok);

  for (;;) {
    if (is_token_punct(*tok, "<")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_LT);
      tmp->lhs = node;
      tmp->rhs = add(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, "<=")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_LE);
      tmp->lhs = node;
      tmp->rhs = add(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, ">")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_LT);
      tmp->lhs = add(tok);
      tmp->rhs = node;
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, ">=")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_LE);
      tmp->lhs = add(tok);
      tmp->rhs = node;
      node = tmp;
      continue;
    }

    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **tok) {
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

// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **tok) {
  Node *node = unary(tok);

  for (;;) {
    if (is_token_punct(*tok, "*")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_MUL);
      tmp->lhs = node;
      tmp->rhs = unary(tok);
      node = tmp;
      continue;
    }

    if (is_token_punct(*tok, "/")) {
      *tok = (*tok)->next;
      Node *tmp = new_node(ND_DIV);
      tmp->lhs = node;
      tmp->rhs = unary(tok);
      node = tmp;
      continue;
    }

    return node;
  }
}

// unary = ("+" | "-") unary | primary
static Node *unary(Token **tok) {
  if (is_token_punct(*tok, "+")) {
    *tok = (*tok)->next;
    Node *node = unary(tok);
    return node;
  }

  if (is_token_punct(*tok, "-")) {
    *tok = (*tok)->next;
    Node *node = new_node(ND_NEG);
    node->lhs = unary(tok);
    return node;
  }

  return primary(tok);
}

// primary = "(" expr ")" | ident | num
static Node *primary(Token **tok) {
  if (is_token_punct(*tok, "(")) {
    *tok = (*tok)->next;
    Node *node = expr(tok);
    if (!is_token_punct(*tok, ")")) 
      error_at((*tok)->loc, "expected \")\"");
    *tok = (*tok)->next;
    return node;
  }

  if (is_token_ident(*tok)) {
    Node *node = new_node(ND_VAR);
    node->name = *(*tok)->loc;
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

Node *parse(Token *tok) {
  Node head = {};
  Node *cur = &head;
  while (!is_token_eof(tok)) 
    cur = cur->next = stmt(&tok);
  return head.next;
}