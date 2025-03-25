project(SemanticRelaxation VERSION 0.1)

target_sources(SemanticRelaxation PRIVATE
    AITImp.cpp
    BatchPopImp.cpp
    ParallelGeijer.cpp
    GeijerBatchImp.cpp
    GeijerImp.cpp
    IVTImp.cpp
    ReplayImp.cpp
    HeuristicGeijer.cpp
    ParallelBatchImp.cpp
    ReplayTreeImp.cpp
    FenwickImp.cpp
    FAAImp.cpp
    FenwickAImp.cpp
    MonteReplayTree.cpp
    ReplayTreeAImp.cpp
    MinMax2DDAImp.cpp
    OrderApproxImp.cpp
    ParallelBoxImp.cpp
)