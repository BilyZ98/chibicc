#include "chibicc.h"
#include <string.h>

// static Obj local_obj;
static Obj* local_obj_ptr = NULL;

static Node* new_node(NodeKind kind, Token* tok) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

static Node *new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok) {
  Node* node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node* new_unary(NodeKind kind, Node* expr, Token* tok) {
  Node* node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

static Node* new_num(int val, Token* tok) {
  Node* node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}


static Node* new_add(Node* lhs, Node* rhs, Token* tok) {
  add_type(lhs);
  add_type(rhs);

  // num and num
  if(is_integer(lhs->type) && is_integer(rhs->type)){
    return new_binary(ND_ADD, lhs, rhs, tok);
  }

  // ptr and ptr is an error for add
  if(lhs->type->base && rhs->type->base) {
    fprintf(stderr, "lhs type kind:%d, rhs type kind:%d\n", lhs->type->kind, rhs->type->kind);
    fprintf(stderr, "lhs base: %d, rhs base: %d\n", lhs->type->base, rhs->type->base);
    error_tok(tok, "invalid operation");
  }
  //

  if(!lhs->type->base && rhs->type->base) {
    Node* tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  // ptr and num
  rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
  return new_binary(ND_ADD, lhs, rhs, tok);

}

static Node* new_sub(Node* lhs, Node* rhs, Token* tok) {
  add_type(lhs);
  add_type(rhs);

  // num and num
  if(is_integer(lhs->type) && is_integer(rhs->type)) {
    return new_binary(ND_SUB, lhs, rhs, tok);
  }

  // ptr - num
  if(lhs->type->base && is_integer(rhs->type)  ) {

    rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
    return new_binary(ND_SUB, lhs, rhs, tok);

  }

  // ptr - ptr 
  if(lhs->type->base && rhs->type->base) {
    Node* sub_node = new_binary(ND_SUB, lhs, rhs, tok);
    sub_node->type = ty_int;
    return new_binary(ND_DIV, sub_node, new_num(8, tok), tok );
  }

  error_tok(tok, "invalid operation");


}



static Node* expr(Token** rest, Token* tok) ;
static Node* expr_stmt(Token**rest, Token* tok);
static Node* assign(Token**rest, Token* tok);
static Node* mul(Token** rest, Token* tok);
static Node* unary(Token** rest, Token* tok);
static Node* equality(Token** rest, Token* tok);
static Node* relational(Token**rest, Token* tok);
static Node* add(Token** rest, Token*tok);
static Node* declaration(Token **rest, Token* tok);
static Node* primary(Token **rest, Token* tok);
static Node* stmt(Token**rest, Token* tok);
static Node* compound_stmt(Token**rest, Token* tok);

// stmt = "return" expr ";" 
//        | "{" compound_stmt
//        | expr_stmt
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "for" "(" expr_stmt expr? ";" expr? ")" stmt 
//        | "while" "(" expr ")" stmt
static Node* stmt(Token**rest, Token* tok) {
  if(equal(tok, "return")) {
    Node* node = new_unary(ND_RETURN, expr(&tok, tok->next), tok);
    *rest = skip(tok, ";");
    return node;
  }

  if(equal(tok, "{")) {
    return compound_stmt(rest, tok->next);
  }

  if(equal(tok, "if")) {
    tok = skip(tok->next, "(");
    Node* node = new_node(ND_IF, tok);
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");
    node->then = stmt(&tok, tok);
    if(equal(tok, "else")) {
      node->els = stmt(&tok, tok->next);
    }
    *rest = tok;
    return node;
  }

  if(equal(tok, "for")) {
    tok = skip(tok->next, "(");
    Node* node = new_node(ND_FOR, tok);
    node->init = expr_stmt(&tok, tok);
    if(!equal(tok, ";")) {
      node->cond = expr(&tok, tok);
    }
    tok = skip(tok, ";");

    if(!equal(tok, ")")) {
      node->inc = expr(&tok, tok);
    }
    tok = skip(tok, ")");
    node->then = stmt(&tok, tok);
    *rest = tok;
    return node;
  }

  if(equal(tok, "while")) {
    tok = skip(tok->next, "("); 
    Node* node = new_node(ND_FOR, tok);
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");
    node->then = stmt(&tok, tok);
    *rest = tok;
    return node;
  }

  return expr_stmt(rest, tok);
}

static Node* new_var_node(Obj* obj, Token* tok) {
  Node* node = new_node(ND_VAR, tok);
  node->obj = obj;
  return node;
}

static Obj* new_lvar(char* name, Type* ty) {
  // char* new_name = strndup(start, len)  ;
  Obj* new_lvar = calloc(1, sizeof(Obj));
  new_lvar->name = name; 
  new_lvar->ty = ty;
  new_lvar->next = local_obj_ptr;
  local_obj_ptr = new_lvar;
  return new_lvar;
}

static char* get_ident(Token* tok) {

  if(tok->kind != TK_IDENT) {
    error_tok(tok, "expected an identifier");
  }
  return strndup(tok->loc, tok->len);
  
}

// declspec = "int"
static Type* declspec(Token** rest, Token* tok) {

  *rest=  skip(tok, "int");
  return ty_int;
}


//  declarator = "*"* ident
static Type* declarator(Token** rest, Token* tok, Type* ty){

  while(consume(&tok, tok, "*")) {
    ty = pointer_to(ty);

  }

  if(tok->kind != TK_IDENT) {
    error_tok(tok, "expected a variable name");
  }


  ty->name = tok;
  *rest = tok->next;
  return ty;




}

// declaration = declspec ( declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
static Node* declaration(Token** rest, Token* tok) {
  Type* basety = declspec(&tok, tok);

  Node head = {};
  Node* cur = &head;
  int i = 0;
  while(!equal(tok, ";")) {
    if(i++ > 0) {
      tok = skip(tok, ",");
    }

    Type* ty = declarator(&tok, tok, basety);
    Obj* var = new_lvar(get_ident(ty->name), ty);

    if(!equal(tok, "=")) {
      continue;
    }

    Node* lhs = new_var_node(var, ty->name);
    Node* rhs = assign(&tok, tok->next);
    Node* node = new_binary(ND_ASSIGN, lhs, rhs, tok);

    cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);


  }
  Node* node = new_node(ND_BLOCK, tok);
  node->body = head.next;
  *rest = tok->next;
  return node;

}
// compound_stmt = (declaration | stmt)* "}"
static Node* compound_stmt(Token** rest, Token* tok) {
  Node head = {};
  Node* cur = &head;
  while(!equal(tok, "}")) {
    if(equal(tok, "int")) {
      cur = cur->next = declaration(&tok, tok);

    } else{
      cur = cur->next = stmt(&tok, tok);
    }
    add_type(cur);
  }
  Node* node = new_node(ND_BLOCK, tok);
  node->body = head.next;
  *rest = tok->next;

  return node;

}


// expr_stmt = expr? ";"
static Node* expr_stmt(Token**rest, Token* tok) {
  if(equal(tok, ";")) {
    Node* node = new_node(ND_BLOCK, tok);
    *rest = tok->next;
    return node;
  }
  Node* node = new_unary(ND_EXPR_STMT, expr(&tok, tok), tok);
  *rest = skip(tok, ";");
  return node;

}

// expr = assign
static Node* expr(Token **rest, Token* tok){
  return assign(rest, tok);
}

// assign = equality ("=" assign)?
static Node* assign(Token** rest, Token* tok) {
  Node* node = equality(&tok, tok);
  if(equal(tok, "=")) {
    node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next), tok);
  }

  *rest = tok;
  return node;
}

// bottom up
// equality = relational ("==" relational | "!= relational")*
static Node* equality(Token **rest, Token*tok) {
  Node* node = relational(&tok, tok);
  for(;;) {
    if(equal(tok, "==")) {
      node = new_binary(ND_EQ, node, relational(&tok, tok->next), tok);
      continue;
    }

    if(equal(tok, "!=")) {
      node = new_binary(ND_NE, node, relational(&tok, tok->next), tok);
      continue;;
    }
    *rest = tok;
    return node;
  }

}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node* relational(Token**rest, Token* tok) {
  Node* node = add(&tok, tok);
  for(;;) {
    if(equal(tok, "<")) {
      node = new_binary(ND_LT, node, add(&tok, tok->next), tok);
      continue;;
    }

    if(equal(tok, "<=")) {
      node = new_binary(ND_LE, node, add(&tok, tok->next), tok);
      continue;;
    }

    if(equal(tok, ">")) {
      node = new_binary(ND_LT, add(&tok, tok->next), node, tok);
      continue;;
    }

    if(equal(tok, ">=")) {
      node = new_binary(ND_LE, add(&tok, tok->next), node, tok);
      continue;;
    }
    *rest = tok;
    return node;

  }
}

// add = mul ("+" mul | "-" mul)*
static Node* add(Token**rest, Token* tok) {
  Node* node= mul(&tok, tok);

  for(;;){
    if(equal(tok, "+") ) {
      // node = new_binary(ND_ADD, node, mul(&tok, tok->next), tok);
      node = new_add(node, mul(&tok, tok->next), tok);
      continue;
    }

    if(equal(tok, "-")) {
      // node = new_binary(ND_SUB, node, mul(&tok, tok->next), tok);
      node = new_sub(node, mul(&tok, tok->next), tok);
      continue;
    }

    *rest = tok;
    return node;

  }
}


// expr = unary ("*" unary | "/" unary)*
static Node* mul(Token**rest, Token* tok) {
  Node* node = unary(&tok, tok);

  for(;;) {
    if(equal(tok, "*")) {
      node =  new_binary(ND_MUL, node, unary(&tok, tok->next), tok);
      continue;;
    }

    if(equal(tok, "/")) {
      node = new_binary(ND_DIV, node, unary(&tok, tok->next), tok);
      continue;;
    }

    *rest = tok;
    return node;
  }
}


// unary = ("+" | "-" | "*" | "&") unary | primary
static Node* unary(Token **rest, Token* tok) {
  if(equal(tok, "+"))
    return unary(rest, tok->next);

  if(equal(tok, "-"))
    return new_unary(ND_NEG, unary(rest, tok->next), tok);

  if(equal(tok, "&"))
    return new_unary(ND_ADDR, unary(rest, tok->next), tok);

  if(equal(tok, "*")) 
    return new_unary(ND_DEREF, unary(rest, tok->next), tok);

  return primary(rest, tok);

}


static Obj* find_var(char* start, int len) {
  for(Obj* o_idx = local_obj_ptr; o_idx; o_idx=o_idx->next) {
    if(len == strlen(o_idx->name) && strncmp(start, o_idx->name, len) == 0) {
      return o_idx;
    }
  }
  return NULL;
}



// primary = "(" expr ")" | ident | num
static Node* primary(Token **rest, Token* tok) {
  if(equal(tok, "(")) {
    Node* node = expr(&tok, tok->next);
    *rest = skip(tok, ")");
    return node;
  }

  if(tok->kind == TK_IDENT) {
    // Node* node = new_var_node(*(tok->loc));
    Obj* obj = find_var(tok->loc, tok->len);
    if(!obj) {
      error_tok(tok, "undefined variable");
    }
    Node* node = new_var_node(obj, tok);
    *rest = tok->next;
    return node;
  }

  if(tok->kind  == TK_NUM) {
    Node* node = new_num(tok->val, tok);
    *rest = tok->next;
    return node;
  }


  fprintf(stderr, "tok kind %d \n",tok->kind);
  error_tok(tok, "expected an expression, tok kind", tok->kind);

}

// program = stmt*
Function* parse(Token* tok) {

  tok = skip(tok, "{");
  Function* func = calloc(1, sizeof(Function));
  func->body = compound_stmt(&tok, tok);
  func->locals = local_obj_ptr;

  return func;
  // return head.next;

}
