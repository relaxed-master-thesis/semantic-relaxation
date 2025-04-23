project(SemanticRelaxation VERSION 0.1)

add_executable(SemanticRelaxation)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winline")
target_compile_options(SemanticRelaxation PRIVATE -fpie -fpic -O3)

add_subdirectory(
    impl
)

add_subdirectory(
    impl/stackImpl
)



add_subdirectory(
    util
)

target_sources(SemanticRelaxation PRIVATE
    main.cpp
)

target_include_directories(SemanticRelaxation PUBLIC "../inc")