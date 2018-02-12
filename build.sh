#!/bin/bash

WIGNORE="-Wno-zero-as-null-pointer-constant -Wno-old-style-cast -Wno-c++98-compat"

pushd build

clang -Weverything $WIGNORE -g --std=c++14 ../src/main.cpp -isystem ../libs/glad/include -isystem ../libs -L/usr/local/lib -L../libs/glad -L../libs -ldl -lm -lGL -lglfw -lglad -o main -Wl,-rpath,\$ORIGIN/../build

popd

