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

Run()
{
    ./build/src/SemanticRelaxation
}

while getopts ":hcr" option; do
    case $option in
        h) # display help
            Help
            exit;;
        c) # compile and run
            Compile;;
        r) # run
            Run;;
        \?) # compile first
            echo "Error: Invalid argument"
            exit;;
    esac
done