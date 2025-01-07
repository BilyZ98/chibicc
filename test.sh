#!/bin/bash

assert() {
  expected="$1"
  input="$2"
  debug=${3:-0}

  echo "debug num is $debug"
  ./chibicc "$input" > tmp.s || exit
  if [[ "$debug" -eq 1 ]]; then 
    echo "got here"
  gcc -static -g -o tmp tmp.s
  gdb --args ./tmp
  else
    echo "adfs $debug"
  gcc -static  -o tmp tmp.s
  ./tmp
  fi

  actual="$?"


  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1

  fi

}

assert 0 "0;"
assert 42 '42;'
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5 * (9-6);'
assert 4  '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- - 10;'
assert 10 '- - +10;'

assert 0 "0 == 1;"
assert 1 "41==41;"
assert 1 "0 != 1;"
assert 0 "42 !=42;"
assert 1 "0 < 1 ;"
assert 0 "1 < 1;"
assert 0 " 2 < 1;"
assert 1 "0 <= 1;"
assert 1 "1<=1;"
assert 0 " 2 <= 1;"


assert 1 " 1>0;"
assert 0 "1 > 1;"
assert 0 "1 > 2;"
assert 1 "1 >= 0;"
assert 1 "1>=1;"
assert 0 "1>=2;"
assert 3 "1;2;3;"

assert 3 "a=3; a;"  1
assert 8 "a=3;b=5; a+b;"
assert 6 "a=z=3; a+z;"


assert 3 "abc=3; abc;"
assert 8 "_xy9=3; abc=5; _xy9+abc;"
assert 6 "__x9=bc_3=3; __x9+bc_3;"

echo OK
