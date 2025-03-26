# Semantic relaxation

This is a Master's thesis project investigating error calculation for relaxed data structures. It is work in progress.

## How to use
See [this](docs/README.md) documentation.

## Build and run

### Prerequisites
- CMake version >= 3.20
- C++ 20 & OpenMP compatible compiler
- OpenMP header files available (`<omp.h>`)

You can build the project with the `./run.sh` script. The script currently takes in two flags: `-c -r`. Use the `-c` flag to compile the project and `-r` to run the executable post-compilation. You can also use these flags independently of each other.

**TODO: add more build flags**

## Documentation

Docs are located in the `docs` folder. There are independent documentation of utilisation classes and implementation classes (those who do actual work).
