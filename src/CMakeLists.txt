project(SemanticRelaxation VERSION 0.1)

add_executable(SemanticRelaxation)
target_sources(SemanticRelaxation PRIVATE
    main.cpp
    qkparser.cpp
)

target_include_directories(SemanticRelaxation PUBLIC "../inc")