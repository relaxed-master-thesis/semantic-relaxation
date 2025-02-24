# Benchmark

Benchmark is a templated class that executes and measures an implementation. The template parameter has a hard requirement of deriving the [`bench::AbstractExecutor`](#abstractexecutor) class. The project will not compile if this requirement is not met.

To run a benchmark, instantiate a benchmark with a suitable template class as shown below and run it using the `bench::Benchmark<T>::run()` method. This method takes an argument of type [`const bench::InputData&`](#inputdata) and returns a [`bench::Result`](#result).

**Note: a benchmark may never mutate the input data.**

```cpp
bench::InputData data = ...
auto benchmark = bench::Benchmark<bench::GeijerImp>{}
auto result = benchmark.run(data);
```

Additionally, if the used template parameter requires constructor arguments you may pass them indirectly via the `bench::Benchmark` constructor.

```cpp
// Calls the ctor HeuristicGeijer(uint64_t, uint64_t, uint64_t)
auto bench = bench::Benchmark<bench::HeuristicGeijer>{10'000, 10'000, 2'000};
```

# InputData

This struct represents the input data for a benchmark. It contains two vectors of [`bench::Operation`](Operation.md) which represent enqueue and dequeue operations respectively.

Construction can be done manually but is more naturally used together with a parser such as [`TimestampParser`](util/QKParser.md)

# AbstractParser

`AbstractParser` is a parser interface which only requires parsers to return an [`bench::InputData`](#inputdata) given two file names (one for get-stamps and one for put-stamps).

# AbstractExecutor

An interface for implementations that calculate relaxation errors.

# Result