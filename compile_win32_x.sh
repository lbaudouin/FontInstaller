#!/bin/bash
qmake -spec win32-x-g++ -o win32/
cd win32
make
cd ..
