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
    cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cmake --build ./build --clean-first
}

Run()
{
    ./build/src/SemanticRelaxation
}

optCompile=false
optRun=false

while getopts ":hcr" option; do
    case $option in
        h) # display help
            Help
            exit;;
        c) # compile and run
            optCompile=true;;
        r) # run
            optRun=true;;
        \?) # compile first
            echo "Error: Invalid argument"
            exit;;
    esac
done

if [ "$optCompile" = true ]; then
    Compile
fi

if [ "$optRun" = true ]; then
    Run
fi