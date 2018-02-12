#!/bin/bash

WIGNORE="-Wno-zero-as-null-pointer-constant -Wno-old-style-cast"

pushd build

clang -Weverything $WIGNORE -g --std=c++14 ../src/main.cpp -isystem ../libs/glad/include -L/usr/local/lib -L../libs/glad -L -ldl -lm -lGL -lglfw -lglad -o main

popd

