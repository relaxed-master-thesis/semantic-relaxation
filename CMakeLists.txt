cmake_minimum_required(VERSION 3.20)

project(SemanticRelaxation VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(
    src
)

find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries(SemanticRelaxation PUBLIC OpenMP::OpenMP_CXX)
endif()
