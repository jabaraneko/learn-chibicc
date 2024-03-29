#include "chibicc.h"

static void gen_expr(Node *node) {
  // Convert terminal symbols. 
  switch (node->kind) {
    case ND_NUM:
      printf("  mov rax, %d\n", node->val);
      return;
    case ND_NEG:
      gen_expr(node->lhs);
      printf("  neg rax\n");
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
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      return;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      return;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      return;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      return;
  }

  error("invalid expression");
}

void codegen(Node *node) {
  // Prologue
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Traverse the AST to emit assembly. 
  gen_expr(node);

  // Epilogue
  printf("  ret\n");
}
