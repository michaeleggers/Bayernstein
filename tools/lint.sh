#!/bin/bash
find . \( -path ./dependencies -o -path ./build \) -prune -o \( -name '*.cpp' -o -name '*.h' \) -print | xargs clang-tidy -p build/