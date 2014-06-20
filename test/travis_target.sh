#! /bin/bash

$HOME/bin/cmake .
if [ $? -ne 0 ]; then
    echo "cmake failed."
    exit 1
fi

make
if [ $? -ne 0 ]; then
    echo "test building failed."
    exit 1
fi

make test
