project(SemanticRelaxation VERSION 0.1)

add_executable(SemanticRelaxation)

target_compile_options(SemanticRelaxation PRIVATE -fpie -fpic)

target_sources(SemanticRelaxation PRIVATE
    main.cpp
)

add_subdirectory(
    impl
    util
)

target_include_directories(SemanticRelaxation PUBLIC "../inc")