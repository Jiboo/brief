#!/bin/bash

# PascalCase for classes
# snake_case_t for trivial types
# camelCase for functions&variables&namespaces
# UPCASE for constants.
# field_, _parameter, local
# 2 spaces indent, empty line at end of file, braces on same line, 120 columns max
# wrap line before operator (except comma) or type, align if possible, or 4 spaces indent
# int *pointer, &ref;
# 1 space after if, for, while, do..., one space before opening brace
# 2 spaces between comment and code, 1 space after // or /*

cpplint.py --extensions=hpp,cpp \
           --linelength=120 \
           --filter=-build/c++11,-runtime/references,-readability/todo \
           cli/*.cpp inc/brief/*.hpp src/*.cpp tst/unit/*.cpp
