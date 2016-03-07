#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
pip install --quiet --user ${DIR}/src/packages/*
