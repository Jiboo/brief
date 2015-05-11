#!/bin/bash
cpplint.py --extensions=hpp,cpp --linelength=120 --filter=-build/c++11,-readability/todo cli/*.cpp inc/brief/*.hpp src/*.cpp tst/unit/*.cpp
