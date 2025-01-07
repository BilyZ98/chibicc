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
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node* lhs; //left hand side
  Node* rhs; // right hand side
  Node* next; // next node
  // char name;
  Obj* obj; // used if kind == ND_VAR
  int val;  // used if kind == ND_NUM
};

typedef struct Function Function;
struct Function {
  Node* node;
  Obj* locals;
  int stack_size;


};


Function *parse(Token* tok);

// codegen.c
void codegen(Function* prog);
