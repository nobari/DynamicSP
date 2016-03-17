#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
g++ ${DIR}/src/main.cpp -O3 -ltbb -std=c++11 -march=native -march=native -funroll-loops -ftree-vectorize -o ${DIR}/src/apsp
