#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// tokenize.c
typedef enum {
  TK_PUNCT, // punctuators
  TK_NUM, 
  TK_IDENT, // Identity
  TK_KEYWORD, // keywrods
  TK_EOF,

} TokenKind;



typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token* next;
  int val; // if TokenKind is TK_NUM, its value
  char* loc;
  int len;
};

typedef struct Obj Obj;
struct Obj{
  char* name;
  Obj* next;
  int offset;
};


void error(char* fmt, ...) ;
void error_at(char* loc, char* fmt, ...);

void error_tok(Token*tok, char* fmt, ...) ;
bool equal(Token* tok, char* op) ;
Token* skip(Token* tok, char*s) ;
Token* tokenize(char*input) ;
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // - 
  ND_MUL,       // *
  ND_DIV,       // /
  ND_NEG,       //  unary -
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_EXPR_STMT, // Expression statement
  ND_ASSIGN,     //  =
  ND_VAR,        // Variable
  ND_NUM,       // Integer
  ND_ADDR,      // unary &
  ND_DEREF,     // unary *
  ND_RETURN,   // keyword return 
  ND_BLOCK,    // {} block 
  ND_IF,        // if else block
  ND_FOR,       // "for" and "while" block
} NodeKind;

typedef enum {
  TY_INT,
  TY_PTR,

} TypeKind;

typedef struct Node Node;
typedef struct Type Type;
struct Node {
  NodeKind kind;
  Type* type;
  Node* lhs; //left hand side
  Node* rhs; // right hand side
  Node* next; // next node
  Node* body; // {} body node

  Token* tok; // representative token

  // "if" or "for" statement
  Node* cond; 
  Node* then; 
  Node* els; 
  Node* init;
  Node* inc;


  // char name;
  Obj* obj; // used if kind == ND_VAR
  int val;  // used if kind == ND_NUM
};



struct Type{
  TypeKind kind;
  Type* base;

};

typedef struct Function Function;
struct Function {
  Node* body;
  Obj* locals;
  int stack_size;


};

extern Type* ty_int;

Function *parse(Token* tok);

// codegen.c
void codegen(Function* prog);
bool is_integer(Type* ty);
void add_type(Node* node);
