#include "chibicc.h"
//
// Code generator
//
//
static int depth;

static void push(void) {
  printf(" push %%rax\n");
  depth++;
}

static void pop(char* arg) {
  printf(" pop %s\n", arg);
  depth--;
}

static int align(int num, int align) {
  return (num + align-1)/ align * align;
}
static void gen_addr_for_local_vars(Function* prog) {
  int cur_offset = 0;
  for(Obj* obj=prog->locals; obj; obj=obj->next) {
    cur_offset+=8;
    obj->offset = cur_offset;

  }
  // Why ?
  // Align to 16 because of AMD65 ABI sepcification.
  prog->stack_size = align(cur_offset, 16);


}
static void gen_addr(Node* node) {
  if(node->kind == ND_VAR) {
    // int offset = (node->name - 'a' + 1)*8;
    int offset = node->obj->offset;
    printf("  lea %d(%%rbp), %%rax\n", -offset);
    return;
  }

  error("not an lvalue");
}
static void gen_expr(Node* node) {
  switch(node->kind) {
    case ND_NUM:
    printf(" mov $%d, %%rax\n", node->val);
    return;

    case ND_NEG:
    gen_expr(node->lhs);
    printf(" neg %%rax\n");
    return;

    case ND_VAR:
    gen_addr(node);
    printf("  mov (%%rax), %%rax\n");
    return;;

    case ND_ASSIGN:
    gen_addr(node->lhs);
    push();
    gen_expr(node->rhs);
    pop("%rdi");
    printf("  mov %%rax, (%%rdi)\n");
    return;

  }

  gen_expr(node->rhs);
  push();
  gen_expr(node->lhs);
  pop("%rdi");

  switch(node->kind) {
    case ND_ADD:
    // adds the value in the %rdi register to the value 
    // in the %rax register, and stores the result in %rax.
    printf(" add %%rdi, %%rax\n");
    return;
    
    case ND_SUB:
    printf(" sub %%rdi, %%rax\n");
    return;

    case ND_MUL:
    printf(" imul %%rdi, %%rax\n");
    return;

    case ND_DIV:
    printf(" cqo\n");
    printf(" idiv %%rdi\n");
    return;

    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    printf(" cmp %%rdi, %%rax\n");

    if(node->kind == ND_EQ) {
      printf(" sete %%al\n");
    }else if(node->kind == ND_NE) {
      printf(" setne %%al\n");
    } else if(node->kind == ND_LT) {
      printf(" setl %%al\n");
    } else if(node->kind == ND_LE) {
      printf(" setle %%al\n");
    }

    printf(" movzb %%al, %%rax\n");
    return;
  }
  error("invalid expression");
}

static int count_depth(void) {
  static int i = 1;
  return i++;
}

static void gen_stmt(Node* node) {
  switch(node->kind) {
    case ND_RETURN:
    gen_expr(node->lhs);
    printf("  jmp .L.return\n");
    return;

    case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;

    case ND_BLOCK:
      for(Node* n = node->body; n; n = n->next) {
        // Traverse the AST to emit assembly
        gen_stmt(n);
        // assert(depth == 0);
      }
      return;

    case ND_IF: {
      int c = count_depth();
      gen_expr(node->cond);
      printf("  cmp $0, %%rax\n");
      printf("  je  .L.else.%d\n", c);
      gen_stmt(node->then);
      printf("  jmp .L.end.%d\n", c);
      printf(".L.else.%d:\n", c);
      if(node->els) {
          gen_stmt(node->els);
      }
      printf(".L.end.%d:\n",c);
      return;

    }
    case ND_FOR: {
      int c = count_depth();
      gen_stmt(node->init);
      printf(".L.begin.%d:\n", c);
      if(node->cond) {
        gen_expr(node->cond);
        printf("  cmp $0, %%rax\n");
        printf("  je .L.end.%d\n",c);
      }

      gen_stmt(node->then);
      if(node->inc) {
        gen_expr(node->inc);
      }
      printf("  jmp .L.begin.%d\n",c);
      printf(".L.end.%d:\n",c);
      return;
    }
      
  }
  error("invalid statement");

}
void codegen(Function* prog) {
  gen_addr_for_local_vars(prog);

  depth = 0;
  printf("  .globl main\n");
  printf("main: \n");
  // Prologue
  printf("  push %%rbp\n");
  printf("  mov %%rsp, %%rbp\n");
  printf("  sub $%d, %%rsp\n", prog->stack_size);

  gen_stmt(prog->body);
  assert(depth == 0);
  // for(Node* n = prog->node; n; n = n->next) {
  //   // Traverse the AST to emit assembly
  //   gen_stmt(n);
  //   assert(depth == 0);
  // }

  // Epilogue
  printf(".L.return:\n");
  printf("  mov %%rbp, %%rsp\n");
  printf("  pop %%rbp\n");
  // In x86-64 calling conventions, 
  // the return value of a function is typically stored 
  // in the rax register
  printf("  ret\n");


}

