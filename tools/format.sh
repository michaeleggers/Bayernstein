find . -path ./dependencies -prune -o -name '*.cpp' -print | xargs clang-format -i
