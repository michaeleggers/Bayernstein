find . -path ./dependencies -prune -o \( -name '*.cpp' -o -name '*.h' \) -print | xargs clang-format -i
