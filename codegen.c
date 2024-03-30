#include "chibicc.h"

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

// Computes the absolute address of a given node. 
static void gen_addr(Node *node) {
  if (node->kind != ND_VAR)
    error("not a lvalue");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->var->offset);
}

// Assigns offsets to local variables. 
static void assign_lvar_offsets(Function *prog) {
  int offset = 0;
  for (Obj *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = align_to(offset, 16);
}

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
    case ND_VAR:
      gen_addr(node);
      printf("  mov rax, [rax]\n");
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      printf("  push rax\n");
      gen_expr(node->rhs);
      printf("  pop rdi\n");
      printf("  mov [rdi], rax\n");
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

static void gen_stmt(Node *node) {
  if (node->kind == ND_EXPR_STMT) {
    gen_expr(node->lhs);
    return;
  }

  error("invalid statement");
}

void codegen(Function *prog) {
  assign_lvar_offsets(prog);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);

  for (Node *n = prog->body; n; n = n->next) {
    // Traverse the AST to emit assembly. 
    gen_stmt(n);
  }

  // Epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
