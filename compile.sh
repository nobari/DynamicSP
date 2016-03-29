#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
g++ ${DIR}/src/* -O3 -std=c++11 -ltbb -march=native -mtune=native -funroll-loops -ftree-vectorize -o ${DIR}/src/apsp
