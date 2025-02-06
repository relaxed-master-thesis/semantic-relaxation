project(SemanticRelaxation VERSION 0.1)

add_executable(SemanticRelaxation)

target_compile_options(SemanticRelaxation PRIVATE -fpie -fpic)

target_sources(SemanticRelaxation PRIVATE
    main.cpp
    GeijerImp.cpp
    QKParser.cpp
    ReplayImp.cpp
    BatchPopImp.cpp
    GeijerBatch.cpp
    GeijerBatchPopImp.cpp
)

target_include_directories(SemanticRelaxation PUBLIC "../inc")