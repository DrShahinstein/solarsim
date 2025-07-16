#!/bin/bash

./build/app.exe

if [ $? -eq 0 ]; then
    echo "exit 0"
else
    echo "exit -1"
fi
