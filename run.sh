#!/bin/bash

Help()
{
    echo "Run Semantic-Relaxation benchmarks"
    echo
    echo "options:"
    echo "-c    Compile before run"
    echo
}

Benchmark()
{
    # generate different kinds of data
    # 2dd: dynamically change w/l to get diff mean
    # dcbo: ??? 
    
    #  32      128     512     2048     8192      32768       (*4)(2*2)
    # (8*16) (16*32) (32*64) (64*128) (128*256) (256*512)   (*4)(2*2)
    # "w l"
    declare -a twoddcfgs=(
        # "8 4"
        # "16 8"
        # "32 16"
        "64 32"
        # "128 64"
        # "256 128"
        # "512 256"
    )

    if [ ! -d ../semantic-relaxation-dcbo ]; then
        echo "Can't find ../semantic-relaxation-dcbo, exiting..."
        exit
    fi

    echo "Entering ../semantic-relaxation-dcbo"
    cd ../semantic-relaxation-dcbo
    rm ./../semantic-relaxation/bench.txt

    echo "Compiling 2Dd-queue_optimized..."
    buildArg="make 2Dd-queue_optimized RELAXATION_ANALYSIS=TIMER SAVE_TIMESTAMPS=1 SKIP_CALCULATIONS=1 VALIDATESIZE=0"
    eval "$buildArg" > /dev/null 2>&1
    
    for elem in "${twoddcfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        # change to 1s
        testDurMs=1000
        # change -n to 16 threads
        numThreads=2
        # change to 1'000'000
        startSize=10000

        echo "Running: ./bin/2Dd-queue_optimized -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs}"

        ./bin/2Dd-queue_optimized -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 

        dataPath="../semantic-relaxation/data/benchData/2ddqopt-w${strarr[0]}-l${strarr[1]}-i${startSize}-n${numThreads}-d${testDurMs}"

        # mkdir ../semantic-relaxation/data/benchData
        rm -rf $dataPath
        mkdir $dataPath
        mv results/timestamps/* $dataPath
        rm -rf results/timestamps

        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -t ${numThreads} -i ${dataPath} -r 2" 
            eval "$runArg" >> ./../semantic-relaxation/bench.txt
        fi
    done

    echo "Leaving ../semantic-relaxation-dcbo"
    cd ../semantic-relaxation
}

Compile()
{
    cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_VERBOSE_MAKEFILE=OFF
    cmake --build ./build

    if [ ! $? -eq 0 ]; then
        echo "Build failed"
        exit
    fi
}

Run()
{
    dataPath="./data/benchData/2ddqopt-w256-l128-i10000-n2-d200"
    # dataPath="./data/timestamps/2dd-q-opt-w50-l10-i1000-8t-30ms"

    t=12
    r=1
    runArg="./build/src/SemanticRelaxation -t ${t} -i ${dataPath} -r ${r}"
    eval "$runArg"
}

Plot()
{
    # plot data
    echo "Plotting..."
    python3 scripts/parse_bench.py
}

optBench=false
optCompile=false
optRun=false
optPlot=false
optSingle=false

while getopts ":hcrbps" option; do
    case $option in
        h) # display help
            Help
            exit;;
        c) # compile and run
            optCompile=true;;
        r) # run
            optRun=true;;
        b)  # bench
            optBench=true;;
        p) # plot
            optPlot=true;;
        \?) # compile first
            echo "Error: Invalid argument"
            exit;;
    esac
done

if [ "$optCompile" = true ]; then
    Compile
fi

if [ "$optBench" = true ]; then
    # Compile
    Benchmark
    if [ "$optPlot" = true ]; then
        Plot
    fi
    exit
fi

if [ "$optRun" = true ]; then
    Run
fi