#!/bin/bash
find src \( -name \*.c -or -name \*.h \) | xargs -n1 -P4 clang-format -style=file -i
