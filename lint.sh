#!/bin/bash

# PascalCase for classes
# snake_case_t for trivial types
# camelCase for functions, variables & namespaces
# UPCASE for constants.
# field_, _parameter, local
# 2 spaces indent, empty line at end of file, braces on same line, 120 columns max
# wrap line before operator (except comma) or type, align if possible, or 4 spaces indent
# int *pointer, &ref;
# 1 space after if, for, while, do..., one space before opening brace
# 2 spaces between comment and code, 1 space after // or /*

cpplint --extensions=hpp,cpp \
           --linelength=120 \
           --filter=-build/c++11,-runtime/reference,-readability/todo \
           cli/*.cpp inc/brief/*.hpp inc/brief/model/*.hpp src/*.cpp tst/unit/*.cpp

# need to copy compile_commands.json from build dir first

clang-tidy -checks=*,-google-readability-todo,-google-readability-braces-around-statements,-readability-braces-around-statements \
  src/* inc/brief/* inc/brief/model/*
