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

assert 0 "return 0;"
assert 42 'return 42;'
assert 21 "return 5+20-4;"
assert 41 "return  12 + 34 - 5 ;"
assert 47 'return 5+6*7;'
assert 15 'return 5 * (9-6);'
assert 4  'return (3+5)/2;'
assert 10 'return -10+20;'
assert 10 'return - - 10;'
assert 10 'return - - +10;'

assert 0 "return 0 == 1;"
assert 1 "return 41==41;"
assert 1 "return 0 != 1;"
assert 0 "return 42 !=42;"
assert 1 "return 0 < 1 ;"
assert 0 "return 1 < 1;"
assert 0 "return 2 < 1;"
assert 1 "return 0 <= 1;"
assert 1 "return 1<=1;"
assert 0 "return 2 <= 1;"


assert 1 "return  1>0;"
assert 0 "return 1 > 1;"
assert 0 "return 1 > 2;"
assert 1 "return 1 >= 0;"
assert 1 "return 1>=1;"
assert 0 "return 1>=2;"

assert 3 "a=3; return a;"  
assert 8 "a=3;b=5;return  a+b;"
assert 6 "a=z=3;return  a+z;"


assert 3 "abc=3;return abc;"
assert 8 "_xy9=3; abc=5;return _xy9+abc;"
assert 6 "__x9=bc_3=3;return __x9+bc_3;"

assert 1 "return 1; 2; 3;"
assert 2 "1; return 2; 3;"
assert 3 "1; 2; return 3;"

echo OK
