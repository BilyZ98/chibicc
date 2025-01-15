#include "chibicc.h"

static char *current_input;
// reports an error location and exit
static void verror_at(char *loc, char* fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n",current_input);
  fprintf(stderr, "%*s", pos, ""); //print pos spaces
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
 void error(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap); 
  fprintf(stderr, "\n");
  exit(1);
};

  


 void error_at(char* loc, char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

 void error_tok(Token*tok, char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}


 bool equal(Token* tok, char* op) {
  return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

static bool startwith(char* p, char* q) {
  return strncmp(p, q, strlen(q)) == 0;
}

static int read_punct(char* p){
  if(startwith(p, "==") || startwith(p, "!=")
    || startwith(p, "<=") || startwith(p, ">=")) {
    return 2;
  }
  return ispunct(*p) ? 1 : 0;
}

Token* skip(Token* tok, char*s) {
  if(!equal(tok, s)) {
    error_tok(tok, "expected '%s'", s);
  }
  return tok->next;
}

static int get_number(Token* tok) {
  if(tok->kind != TK_NUM) {
    error_tok(tok, "expected a number");
  }

  return tok->val;
}

static Token* new_token(TokenKind kind, char* start, char* end) {
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

static bool isdent1(char c) {

  return (c >= 'a' && c <= 'z') || (c >='A' && c <= 'Z')|| c == '_';
  
}

static bool isident2(char c) {
  return isdent1(c) || (c >= '0' && c <='9');

}

static bool is_keyword(Token* tok) {
  char* arr[] = {"return", "if", "else"} ;
  for(int i=0; i < sizeof(arr)/ sizeof(*arr); i++ ) {
    if(equal(tok, arr[i])) {
      return true;
    }
  }
  return false;
}
static void convert_keywords(Token* tok) {
  for(; tok->kind != TK_EOF; tok = tok->next) {
    if(is_keyword(tok)) {
      tok->kind = TK_KEYWORD;
    }
  }

}
Token* tokenize(char* p) {
  current_input = p;
  Token head = {};
  Token* cur = &head;
  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char* q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // identifier or keyword
    if(isdent1(*p)) {
      char* id_start = p;
      while(p && isident2(*p)) {
        p++;
      }
      cur = cur->next = new_token(TK_IDENT, id_start, p);
      continue;
    }
    
    int punct_len = read_punct(p);
    if(punct_len) {
      cur->next = new_token(TK_PUNCT, p, p+punct_len);
      cur = cur->next;
      p+=punct_len;
      continue;;
    }
    error_at(p, "invalid token");


  }
  cur = cur->next = new_token(TK_EOF, p, p);
  convert_keywords(head.next);
  return head.next;
}


