#!/bin/bash

assert() {
  expected="$1"
  input="$2"
  debug=${3:-0}

  if [[ "$debug" -eq 1 ]]; then 
  gdb --args ./chibicc "$input" 
  # > tmp.s || exit
  # gcc -static -g -o tmp tmp.s
  else
  ./chibicc "$input" > tmp.s || exit
  fi

  gcc -static  -o tmp tmp.s
  ./tmp

  actual="$?"


  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1

  fi

}
assert 0 '{ return 0; }'
assert 42 '{ return 42; }'
assert 25 '{return 5+20; }'
assert 21 '{ return 5+20-4; }'
assert 41 '{ return  12 + 34 - 5 ; }'
assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'
assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'

assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'

assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'
assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'

assert 3 '{ int a=3; return a; }'
assert 8 '{ int a=3; int z=5; return a+z; }'
assert 6 '{ int a; int b; a=b=3; return a+b; }'
assert 3 '{ int foo=3; return foo; }'
assert 8 '{ int foo123=3; int bar=5; return foo123+bar; }'

assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'

assert 3 '{ {1; {2;} return 3;} }'

assert 5 '{ ;;; return 5; }'


assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else if(3){ return 4; } }'

assert 55 '{ int i=0; int j=0; for(i=0; i <= 10; i=i+1) j=i+j;  return j; }'
assert 3  '{for(;;){return 3;} return 5;}'

assert 10 '{ int i=0; while(i<10) { i=i+1;} return i;}'


# assert 3 '{ x=3; return *&x; }'
# assert 3 '{ x=3; y=&x; z=&y; return **z; }'
# assert 5 '{ x=3; y=5; return *(&x+8); }'
# assert 3 '{ x=3; y=5; return *(&y-8); }'
# assert 5 '{ x=3; y=&x; *y=5; return x; }'
# assert 7 '{ x=3; y=5; *(&x+8)=7; return y; }'
# assert 7 '{ x=3; y=5; *(&y-8)=7; return x; }'

assert 3 '{ int x=3; return *&x; }'
assert 3 '{ int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 '{ int x=3; int y=5; return *(&x+1); }'
assert 3 '{ int x=3; int y=5; return *(&y-1); }'
assert 5 '{ int x=3; int y=5; return *(&x-(-1)); }'
assert 5 '{ int x=3; int* y=&x; *y=5; return x; }'
assert 7 '{ int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 '{ int x=3; int y=5; *(&y-2+1)=7; return x; }'
assert 5 '{ int x=3; return (&x+2)-&x+3; }'
assert 8 '{ int x, y; x = 3; y = 5; return x + y;}'
assert 8 '{ int x=3, y =5; return x + y;}'


# assert 0 "{return 0;}"
# assert 42 '{return 42;}'
# assert 21 "{return 5+20-4;}"
# assert 41 "{return  12 + 34 - 5 ;}"
# assert 47 '{return 5+6*7;}'
# assert 15 '{return 5 * (9-6);}'
# assert 4  '{return (3+5)/2;}'
# assert 10 '{return -10+20;}'
# assert 10 '{return - - 10;}'
# assert 10 '{return - - +10;}'

# assert 0 "{return 0 == 1;}"
# assert 1 "{return 41==41;}"
# assert 1 "{return 0 != 1;}"
# assert 0 "{return 42 !=42;}"
# assert 1 "{return 0 < 1 ;}"
# assert 0 "{return 1 < 1;}"
# assert 0 "{return 2 < 1;}"
# assert 1 "{return 0 <= 1;}"
# assert 1 "{return 1<=1;}"
# assert 0 "return 2 <= 1;"


# assert 1 "return  1>0;"
# assert 0 "return 1 > 1;"
# assert 0 "return 1 > 2;"
# assert 1 "return 1 >= 0;"
# assert 1 "return 1>=1;"
# assert 0 "return 1>=2;"

# assert 3 "a=3; return a;"  
# assert 8 "a=3;b=5;return  a+b;"
# assert 6 "a=z=3;return  a+z;"


# assert 3 "abc=3;return abc;"
# assert 8 "_xy9=3; abc=5;return _xy9+abc;"
# assert 6 "__x9=bc_3=3;return __x9+bc_3;"

# assert 1 "return 1; 2; 3;"
# assert 2 "1; return 2; 3;"
# assert 3 "1; 2; return 3;"

echo OK
