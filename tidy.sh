#!/bin/bash

WIGNORE="-Wno-zero-as-null-pointer-constant -Wno-old-style-cast -Wno-c++98-compat -Wno-sign-conversion -Wno-cast-align -Wno-double-promotion -Wno-nested-anon-types -Wno-padded -Wno-unused-macros -Wno-global-constructors -Wno-missing-variable-declarations -Wno-missing-prototypes -Wno-unused-function -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments -Wno-c++98-compat-pedantic"

DEBUG=""

pushd build

clang-tidy ../src/main.cpp -- -Weverything $WIGNORE -g -O0 --std=c++14 $DEBUG  -isystem ../libs/glad/include -isystem ../libs

popd
