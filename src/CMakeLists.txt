project(SemanticRelaxation VERSION 0.1)

add_executable(SemanticRelaxation)

target_compile_options(SemanticRelaxation PRIVATE -fpie -fpic)

add_subdirectory(
    impl
)

add_subdirectory(
    util
)

target_sources(SemanticRelaxation PRIVATE
    main.cpp
    Interval.cpp
)

target_include_directories(SemanticRelaxation PUBLIC "../inc")