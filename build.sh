#!/bin/sh

set -xe

CXX_FLAGS="-Wall -Wextra -ggdb -fsanitize=address -pedantic -std=c++2a"
BIN="main"
CXX="g++"
CXX_FILES="src/main.cpp"

$CXX $CXX_FLAGS $CXX_FILES -o $BIN
