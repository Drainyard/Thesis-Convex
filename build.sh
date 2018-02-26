#!/bin/bash

WIGNORE="-Wno-zero-as-null-pointer-constant -Wno-old-style-cast -Wno-c++98-compat -Wno-sign-conversion -Wno-cast-align -Wno-double-promotion -Wno-nested-anon-types -Wno-padded -Wno-unused-macros -Wno-global-constructors -Wno-missing-variable-declarations -Wno-missing-prototypes"

pushd build

clang -Weverything $WIGNORE -g --std=c++14 ../src/main.cpp -isystem ../libs/glad/include -isystem ../libs -L/usr/local/lib -L../libs/glad -L../libs -ldl -lm -lGL -lglfw -lglad -lstdc++ -o main -Wl,-rpath,\$ORIGIN/../build

popd

