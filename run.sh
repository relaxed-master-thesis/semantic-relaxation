#!/bin/bash

Help()
{
    echo "Run Semantic-Relaxation benchmarks"
    echo
    echo "options:"
    echo "-c    Compile before run"
    echo
}

Compile()
{
    cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cmake --build ./build
}

while getopts ":hc" option; do
    case $option in
        h) # display help
            Help
            exit;;
        c) # compile and run
            Compile;;
        \?) # compile first
            echo "Error: Invalid argument"
            exit;;
    esac
done

./build/src/SemanticRelaxation