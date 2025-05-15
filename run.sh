#!/bin/bash

programName=$0
optBench=false
optCompile=false
optRun=false
optPlot=false
optSingle=false
optSaveLog=false

dcboLogFile=""
twoddLogFile=""
graphLogFile=""
stackLogFile=""
twoddBatchLogFile=""
twoddApproxLogFile=""
twoddDelayLogFile=""
dcboBatchLogFile=""
dcboApproxLogFile=""
dcboDelayLogFile=""
queueSizeLogFile=""

# change to 1s
testDurMs=200
# change -n to 16 threads
numThreads=12
# should be at least 3
numRuns=3
# number of gets to calculate
dataSize=1000000
# preset for 2Dd-queue_optimized, can be 2ddq, batch, approx or delay
twoddPreset="2ddq"
#preset for dcbo, can be dcbo, batch, approx or delay
dcboPreset="dcbo"

twoddqType="2ddqopt"

declare -a twoddcfgs=(
    "8 4"
    "16 8"
    "32 16"
    "64 32"
    "128 64"
    "256 128"
    "512 256"
)

declare -a dcbocfgs=(
    "8"
    "32"
    "128"
    "512"
    "2048"
    "8192"
    "32768"
    "65536"
)

Help()
{
    echo "usage: $programName [-bchlp]"
    echo "  -b    --bench       Run benchmark"
    echo "  -c    --compile     Compile before run"
    echo "  -h    --help        Show this help message"
    echo "  -l    --save-log    Save log file(s) to logs/"
    echo "  -p    --plot        Plot data"
    exit 1
}

Benchmark_2ddq()
{
    oldTestDurMs=$testDurMs
    for elem in "${twoddcfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        # change to 1'000'000
        startSize=$((strarr[0] * strarr[1] * 2))

        if [ "$twoddqType" = "2ddqopt" ]; then
            echo "Running: ./bin/2Dd-queue_optimized -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs}"
        else
            echo "Running: ./bin/2Dd-queue -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs}"
        fi

        dataPath="../semantic-relaxation/data/benchData/${twoddqType}-w${strarr[0]}-l${strarr[1]}-i${startSize}-n${numThreads}-d${testDurMs}"

        getCount=0
        while [[ "$getCount" -lt "$dataSize" ]]; 
        do
            getFile="$dataPath/combined_get_stamps.txt"
            # check if dataPath does not exist
            if [ ! -d $dataPath ] || [ $(wc -l < "$getFile") -lt "$dataSize" ]; then
                rm -rf $dataPath
                if [ "$twoddqType" = "2ddqopt" ]; then
                    ./bin/2Dd-queue_optimized -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 
                else
                    ./bin/2Dd-queue -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 
                fi
                mkdir $dataPath
                mv results/timestamps/* $dataPath
                rm -rf results/timestamps
            else
                echo "Data already exists, using old data..."
            fi
            getCount=$(wc -l < "$getFile")
            if [ "$getCount" -lt "$dataSize" ]; then
                echo "Not enough data, fixing test duration..."
                # dataProp=$((getCount / dataSize))
                # dataProp="scale=2 ; $getCount / $dataSize" | bc
                # dataProp=$(( dataProp > 0 ? dataProp : testDurMs * 2 ))
                # testDurMs=$((testDurMs * dataProp))
                testDurMs=$((testDurMs * 2))
                rm -rf $dataPath
                dataPath="../semantic-relaxation/data/benchData/${twoddqType}-w${strarr[0]}-l${strarr[1]}-i${startSize}-n${numThreads}-d${testDurMs}"
                echo "New test duration: $testDurMs"
            fi
        done
        
        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -i ${dataPath} -r ${numRuns} -g ${dataSize} -p ${twoddPreset} -q 2ddq"
            if [ "$optSaveLog" = true ]; then
                eval "$runArg" >> ./../semantic-relaxation/tmp.txt
            else
                eval "$runArg"
            fi
        else
            echo "Failed to move ${dataPath}, skipping..."
        fi
    done
    
    if [ "$optSaveLog" = true ]; then
        if [ ! -d ./../semantic-relaxation/logs ]; then
            mkdir ./../semantic-relaxation/logs
        fi

        if [ -f ./../semantic-relaxation/tmp.txt ]; then
            if [ "$twoddPreset" = "generic" ]; then
                twoddLogFile="${twoddqType}-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$twoddLogFile
            elif [ "$twoddPreset" = "batch" ]; then
                twoddBatchLogFile="${twoddqType}-batch-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$twoddBatchLogFile
            elif [ "$twoddPreset" = "approx" ]; then
                twoddApproxLogFile="${twoddqType}-approx-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$twoddApproxLogFile
            elif [ "$twoddPreset" = "delay" ]; then
                twoddDelayLogFile="${twoddqType}-delay-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$twoddDelayLogFile
            fi
        fi
    fi
    testDurMs=$oldTestDurMs
    echo "Resetting test duration to $testDurMs"
}

Benchmark_dcbo() {
    oldTestDurMs=$testDurMs
    for elem in "${dcbocfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        startSize=$((strarr[0] * 4))

        echo "Running: ./bin/dcbo-faaaq -w ${strarr[0]} -i ${startSize} -n ${numThreads} -d ${testDurMs}"
        dataPath="../semantic-relaxation/data/benchData/dcbo-w${strarr[0]}-i${startSize}-n${numThreads}-d${testDurMs}"

        getCount=0
        while [[ "$getCount" -lt "$dataSize" ]]; 
        do
            getFile="$dataPath/combined_get_stamps.txt"
            # check if dataPath does not exist
            if [ ! -d $dataPath ] || [ $(wc -l < "$getFile") -lt "$dataSize" ]; then
                rm -rf $dataPath
                ./bin/dcbo-faaaq -w ${strarr[0]} -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null
                mkdir $dataPath
                mv results/timestamps/* $dataPath
                rm -rf results/timestamps
            else
                echo "Data already exists, using old data..."
            fi
            getCount=$(wc -l < "$getFile")

            if [ "$getCount" -lt "$dataSize" ]; then
                echo "Not enough data, fixing test duration..."
                # dataProp=$((getCount / dataSize))
                # dataProp="scale=2 ; $getCount / $dataSize" | bc
                # dataProp=$(( dataProp > 0 ? dataProp : testDurMs * 2 ))
                # testDurMs=$((testDurMs * dataProp))
                testDurMs=$((testDurMs * 2))
                rm -rf $dataPath
                dataPath="../semantic-relaxation/data/benchData/dcbo-w${strarr[0]}-i${startSize}-n${numThreads}-d${testDurMs}"
            fi
        done

        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -i ${dataPath} -r ${numRuns} -g ${dataSize} -p ${dcboPreset} -q dcbo"
            if [ "$optSaveLog" = true ]; then
                eval "$runArg" >> ./../semantic-relaxation/tmp.txt
            else
                eval "$runArg"
            fi
        else
            echo "Failed to move ${dataPath}, skipping..."
        fi
    done

    if [ "$optSaveLog" = true ]; then
        if [ ! -d ./../semantic-relaxation/logs ]; then
            mkdir ./../semantic-relaxation/logs
        fi

        if [ -f ./../semantic-relaxation/tmp.txt ]; then
            if [ "$dcboPreset" = "generic" ]; then
                dcboLogFile="dcbo-faaq-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$dcboLogFile
            elif [ "$dcboPreset" = "batch" ]; then
                dcboBatchLogFile="dcbo-faaq-batch-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$dcboBatchLogFile
            elif [ "$dcboPreset" = "approx" ]; then
                dcboApproxLogFile="dcbo-faaq-approx-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$dcboApproxLogFile
            elif [ "$dcboPreset" = "delay" ]; then
                dcboDelayLogFile="dcbo-faaq-delay-$(date -d "today" +"%Y%m%d%H%M").log"
                mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$dcboDelayLogFile
            fi
        fi
    fi
    testDurMs=$oldTestDurMs
}

Benchmark_graph() {
    for elem in "${twoddcfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        inputFile="road_usa"

        if [ "$twoddqType" = "2ddqopt" ]; then
            echo "Running: ./bin/2Dd-queue_optimized -f graphdata/${inputFile}.mtx -w ${strarr[0]} -l ${strarr[1]} -n ${numThreads}"
        else
            echo "Running: ./bin/2Dd-queue -f graphdata/${inputFile}.mtx -w ${strarr[0]} -l ${strarr[1]} -n ${numThreads}"
        fi

        dataPath="../semantic-relaxation/data/benchData/${twoddqType}-${inputFile}-w${strarr[0]}-l${strarr[1]}-n${numThreads}-d1"

        getCount=0

        getFile="$dataPath/combined_get_stamps.txt"
        # check if dataPath does not exist
        if [ ! -d $dataPath ] || [ $(wc -l < "$getFile") -lt "$dataSize" ]; then
            rm -rf $dataPath
            if [ "$twoddqType" = "2ddqopt" ]; then
                ./bin/2Dd-queue_optimized -f graphdata/${inputFile}.mtx -w ${strarr[0]} -l ${strarr[1]} -n ${numThreads} > /dev/null 
            else
                ./bin/2Dd-queue -f graphdata/${inputFile}.mtx -w ${strarr[0]} -l ${strarr[1]} -n ${numThreads} > /dev/null 
            fi
            mkdir $dataPath
            mv results/timestamps/* $dataPath
            rm -rf results/timestamps
        else
            echo "Data already exists, using old data..."
        fi
        getCount=$(wc -l < "$getFile")
        if [ "$getCount" -gt 0 ] && [ "$getCount" -lt "$dataSize" ]; then
            echo "Not enough data in ${dataPath}, skipping..."
            continue
        fi
        
        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -i ${dataPath} -r ${numRuns} -g ${dataSize} -p generic -q 2ddq"
            if [ "$optSaveLog" = true ]; then
                eval "$runArg" >> ./../semantic-relaxation/tmp.txt
            else
                eval "$runArg"
            fi
        else
            echo "Failed to produce data, skipping..."
        fi
    done
    
    if [ "$optSaveLog" = true ]; then
        if [ ! -d ./../semantic-relaxation/logs ]; then
            mkdir ./../semantic-relaxation/logs
        fi

        if [ -f ./../semantic-relaxation/tmp.txt ]; then
            graphLogFile="${twoddqType}-graph-$(date -d "today" +"%Y%m%d%H%M").log"
            mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$graphLogFile
        fi
    fi
}
Benchmark_queueSize()
{
    oldTestDurMs=$testDurMs
    for elem in "${twoddcfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        # change to 1'000'000
        startSize=$((strarr[0] * strarr[1] * 2))

        if [ "$twoddqType" = "2ddqopt" ]; then
            echo "Running: ./bin/2Dd-queue_optimized -w 16 -l 8 -i ${startSize} -n ${numThreads} -d ${testDurMs}"
        else
            echo "Running: ./bin/2Dd-queue -w 16 -l 8 -i ${startSize} -n ${numThreads} -d ${testDurMs}"
        fi

        dataPath="../semantic-relaxation/data/benchData/${twoddqType}-w16-l8-i${startSize}-n${numThreads}-d${testDurMs}"

        getCount=0
        while [[ "$getCount" -lt "$dataSize" ]]; 
        do
            getFile="$dataPath/combined_get_stamps.txt"
            # check if dataPath does not exist
            if [ ! -d $dataPath ] || [ $(wc -l < "$getFile") -lt "$dataSize" ]; then
                rm -rf $dataPath
                if [ "$twoddqType" = "2ddqopt" ]; then
                    ./bin/2Dd-queue_optimized -w 16 -l 8 -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 
                else
                    ./bin/2Dd-queue -w 16 -l 8 -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 
                fi
                mkdir $dataPath
                mv results/timestamps/* $dataPath
                rm -rf results/timestamps
            else
                echo "Data already exists, using old data..."
            fi
            getCount=$(wc -l < "$getFile")
            if [ "$getCount" -lt "$dataSize" ]; then
                echo "Not enough data, fixing test duration..."
                # dataProp=$((getCount / dataSize))
                # dataProp="scale=2 ; $getCount / $dataSize" | bc
                # dataProp=$(( dataProp > 0 ? dataProp : testDurMs * 2 ))
                # testDurMs=$((testDurMs * dataProp))
                testDurMs=$((testDurMs * 2))
                rm -rf $dataPath
                dataPath="../semantic-relaxation/data/benchData/${twoddqType}-w16-l8-i${startSize}-n${numThreads}-d${testDurMs}"
                echo "New test duration: $testDurMs"
            fi
        done
        
        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -i ${dataPath} -r ${numRuns} -g ${dataSize} -p generic -q 2ddq"
            if [ "$optSaveLog" = true ]; then
                eval "$runArg" >> ./../semantic-relaxation/tmp.txt
            else
                eval "$runArg"
            fi
        else
            echo "Failed to move ${dataPath}, skipping..."
        fi
    done
    
    if [ "$optSaveLog" = true ]; then
        if [ ! -d ./../semantic-relaxation/logs ]; then
            mkdir ./../semantic-relaxation/logs
        fi

        if [ -f ./../semantic-relaxation/tmp.txt ]; then
            queueSizeLogFile="${twoddqType}-queue-size-w8-l16-mean-ca-26-$(date -d "today" +"%Y%m%d%H%M").log"
            mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$queueSizeLogFile
        fi
    fi
    testDurMs=$oldTestDurMs
    echo "Resetting test duration to $testDurMs"
}
Benchmark_2ddStack() {
    oldTestDurMs=$testDurMs
    for elem in "${twoddcfgs[@]}"; do
        read -a strarr <<< "$elem"

        if [ -d results/timestamps ]; then
            rm -rf results/timestamps
        fi

        # change to 1'000'000
        startSize=$((strarr[0] * strarr[1] * 2))

        echo "Running: ./bin/2Dd-stack -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs}"

        dataPath="../semantic-relaxation/data/benchData/2ddstack-w${strarr[0]}-l${strarr[1]}-i${startSize}-n${numThreads}-d${testDurMs}"

        getCount=0
        while [[ "$getCount" -lt "$dataSize" ]]; 
        do
            getFile="$dataPath/combined_get_stamps.txt"
            # check if dataPath does not exist
            if [ ! -d $dataPath ] || [ $(wc -l < "$getFile") -lt "$dataSize" ]; then
                rm -rf $dataPath
                ./bin/2Dd-stack -w ${strarr[0]} -l ${strarr[1]} -i ${startSize} -n ${numThreads} -d ${testDurMs} > /dev/null 
                mkdir $dataPath
                mv results/timestamps/* $dataPath
                rm -rf results/timestamps
            else
                echo "Data already exists, using old data..."
            fi
            getCount=$(wc -l < "$getFile")
            if [ "$getCount" -lt "$dataSize" ]; then
                echo "Not enough data, fixing test duration..."
                # dataProp=$((getCount / dataSize))
                # dataProp="scale=2 ; $getCount / $dataSize" | bc
                # dataProp=$(( dataProp > 0 ? dataProp : testDurMs * 2 ))
                # testDurMs=$((testDurMs * dataProp))
                testDurMs=$((testDurMs * 2))
                rm -rf $dataPath
                dataPath="../semantic-relaxation/data/benchData/2ddstack-w${strarr[0]}-l${strarr[1]}-i${startSize}-n${numThreads}-d${testDurMs}"
            fi
        done
        
        if [ $? -eq 0 ]; then
            runArg="./../semantic-relaxation/build/src/SemanticRelaxation -i ${dataPath} -r ${numRuns} -g ${dataSize} -p generic -q stack"
            if [ "$optSaveLog" = true ]; then
                eval "$runArg" >> ./../semantic-relaxation/tmp.txt
            else
                eval "$runArg"
            fi
        else
            echo "Failed to move ${dataPath}, skipping..."
        fi
    done
    
    if [ "$optSaveLog" = true ]; then
        if [ ! -d ./../semantic-relaxation/logs ]; then
            mkdir ./../semantic-relaxation/logs
        fi

        if [ -f ./../semantic-relaxation/tmp.txt ]; then
            stackLogFile="2ddstack-$(date -d "today" +"%Y%m%d%H%M").log"
            mv ./../semantic-relaxation/tmp.txt ./../semantic-relaxation/logs/$stackLogFile
        fi
    fi
    testDurMs=$oldTestDurMs
    echo "Resetting test duration to $testDurMs"
}

Benchmark()
{
    if [ ! -d ../semantic-relaxation-dcbo ]; then
        echo "Can't find ../semantic-relaxation-dcbo, exiting..."
        exit
    fi

    echo "Entering ../semantic-relaxation-dcbo"
    cd ../semantic-relaxation-dcbo
    rm ./../semantic-relaxation/bench.txt

    if [ "$twoddType" = "2ddqopt" ]; then
        echo "Compiling 2Dd-queue_optimized..."
        twoddBuildCmd="make 2Dd-queue_optimized RELAXATION_ANALYSIS=TIMER SAVE_TIMESTAMPS=1 SKIP_CALCULATIONS=1 VALIDATESIZE=0"
        eval "$twoddBuildCmd" > /dev/null
    else
        echo "Compiling 2Dd-queue..."
        twoddBuildCmd="make 2Dd-queue RELAXATION_ANALYSIS=TIMER SAVE_TIMESTAMPS=1 SKIP_CALCULATIONS=1 VALIDATESIZE=0"
        eval "$twoddBuildCmd" > /dev/null
    fi
  
  

    if [ $? -ne 0 ]; then
        # echo "Build failed for 2Dd-queue_optimized"
        echo "Build failed for 2Dd-queue"
        exit
    fi

    echo "Compiling dcbo-faaaq..."
    dcboBuildCmd="make dcbo-faaaq RELAXATION_ANALYSIS=TIMER SAVE_TIMESTAMPS=1 SKIP_CALCULATIONS=1 VALIDATESIZE=0"
    eval "$dcboBuildCmd" > /dev/null

    if [ $? -ne 0 ]; then
        echo "Build failed for dcbo-faaaq"
        exit
    fi

    echo "Compiling 2Dd-stack..."
    stackBuildCmd="make 2Dd-stack RELAXATION_ANALYSIS=TIMER SAVE_TIMESTAMPS=1 SKIP_CALCULATIONS=1 VALIDATESIZE=0"
    eval "$stackBuildCmd" > /dev/null
  
    if [ $? -ne 0 ]; then
        echo "Build failed for 2Dd-stack"
        exit
    fi

    #set to 2ddq if not running 2Dd-queue_optimized
    twoddqType="2ddq"
    
    # #run 2Dd-queue_optimized benchmark
    # dcboPreset="generic"
    # Benchmark_dcbo

    # dcboPreset="batch"
    # Benchmark_dcbo

    # dcboPreset="approx"
    # Benchmark_dcbo

    # dcboPreset="delay"
    # Benchmark_dcbo

    # #run 2Dd-queue_optimized benchmark
    # twoddPreset="generic"
    # Benchmark_2ddq

    # twoddPreset="batch"
    # Benchmark_2ddq

    twoddPreset="approx"
    Benchmark_2ddq

    # twoddPreset="delay"
    # Benchmark_2ddq

    # #run stack test benchmark
    # Benchmark_2ddStack

    # run graph test benchmark
    # Benchmark_graph

    # run queue size test benchmark
    # Benchmark_queueSize
    
    echo "Leaving ../semantic-relaxation-dcbo"
    cd ./../semantic-relaxation
}

Compile()
{
    cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_VERBOSE_MAKEFILE=OFF
    cmake --build ./build --parallel 8

    if [ ! $? -eq 0 ]; then
        echo "Build failed"
        exit
    fi
}

Run()
{
    # dataPath="./data/benchData/${twoddqType}-w256-l128-i10000-n2-d200"
    # dataPath="./data/timestamps/2dd-q-opt-w50-l10-i1000-8t-30ms"
    dataPath="./data/timestamps/2dd-q-law"

    t=12
    r=1
    g=300000
    runArg="./build/src/SemanticRelaxation -t ${t} -i ${dataPath} -r ${r} -g ${g} -w 1 -l 1 -q 2ddq "
    eval "$runArg"
}

Plot()
{
    # plot data
    echo "Plotting..."
    # if [ -z "$twoddLogFile" ] && [ -z "$dcboLogFile"] && [ -z "$graphLogFile"]; then
    #     echo "No log files produced, using latest..."
    #     cd logs
    #     twoddLogFile=$(ls -1rt ${twoddqType}-g1*.log 2>/dev/null | tail -n 1)
    #     dcboLogFile=$(ls -1rt dcbo-faaq-*.log 2>/dev/null | tail -n 1)
    #     graphLogFile=$(ls -1rt ${twoddqType}-graph-*.log 2>/dev/null | tail -n 1)
    #     cd ..
    # fi

    pythonArgs=""
    qsPythonArgs=""
    if [ -n "$twoddLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${twoddLogFile}"
    fi
    if [ -n "$dcboLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${dcboLogFile}"
    fi
    if [ -n "$graphLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${graphLogFile}"
    fi
    if [ -n "$stackLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${stackLogFile}"
    fi
    if [ -n "$twoddBatchLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${twoddBatchLogFile}"
    fi
    if [ -n "$twoddApproxLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${twoddApproxLogFile}"
    fi
    if [ -n "$twoddDelayLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${twoddDelayLogFile}"
    fi
    if [ -n "$dcboBatchLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${dcboBatchLogFile}"
    fi
    if [ -n "$dcboApproxLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${dcboApproxLogFile}"
    fi
    if [ -n "$dcboDelayLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${dcboDelayLogFile}"
    fi
    if [ -n "$queueSizeLogFile" ]; then
        pythonArgs="${pythonArgs} logs/${queueSizeLogFile} -show"
    fi

    pythonArgs="${pythonArgs} -show"

    python3 scripts/parse_bench.py $pythonArgs
    
    # elif [ -z "$graphLogFile" ]; then
    #     echo "Found only dcbo and twodd log files"
    #     python3 scripts/parse_bench.py logs/$dcboLogFile logs/$twoddLogFile

    # elif [ -z "$twoddLogFile" ]; then
    #     echo "Found only dcbo log file"
    #     python3 scripts/parse_bench.py logs/$dcboLogFile
    # elif [ -z "$dcboLogFile" ]; then
    #     echo "Found only twodd log file"
    #     python3 scripts/parse_bench.py logs/$twoddLogFile
    # else
    #     python3 scripts/parse_bench.py logs/$twoddLogFile logs/$dcboLogFile logs/$graphLogFile
    # fi
}

# while getopts ":hcrbpsl" option; do
#     case $option in
#         h) # display help
#             Help
#             exit;;
#         c) # compile and run
#             optCompile=true;;
#         r) # run
#             optRun=true;;
#         b)  # bench
#             optBench=true;;
#         p) # plot
#             optPlot=true
#             optSaveLog=true;;
#         l) # save log
#             optSaveLog=true;;
#         \?) # compile first
#             echo "Error: Invalid argument"
#             exit;;
#     esac
# done

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            Help
            exit;;
        -c|--compile)
            optCompile=true;;
        -r|--run)
            optRun=true;;
        -b|--bench)
            optBench=true;;
        -p|--plot)
            optPlot=true
            optSaveLog=true;;
        -l|--save-log)
            optSaveLog=true;;
        --) # end of all options
            shift
            break;;
        -*)
            echo "Error: Invalid argument"
            exit 1;;
        *) # preserve positional arguments
            POSITIONAL_ARGS+=("$1")
            ;;
    esac
    shift
done
set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

if [ "$optCompile" = true ]; then
    Compile
fi

if [ "$optBench" = true ]; then
    Compile
    Benchmark
fi

if [ "$optPlot" = true ]; then
    Plot
fi

if [ "$optRun" = true ]; then
    Run
    exit
fi