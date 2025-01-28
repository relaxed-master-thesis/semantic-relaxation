#pragma once

#include "Operation.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>

namespace bench {

template <typename T> class UnsafeVector {
  public:
	UnsafeVector() : capacity(128), length(0) {
		arr = static_cast<T *>(calloc(sizeof(T), capacity));
	}

	UnsafeVector(size_t size) : capacity(size), length(0) {
		arr = static_cast<T *>(calloc(sizeof(T), size));
	}

	~UnsafeVector() { free(arr); }

	void push(T &t) {
		if (length == capacity)
			expand();

		T *src = &t;
		T *dst = &(arr[length++]);
		T *res = static_cast<T *>(memcpy(dst, src, sizeof(T)));
		assert(res != NULL && res == dst && "Pushing failed");
	}

	void pop(size_t index) { 
		assert(index < length && "Index out of bounds");

		// TODO: do some flagging or smarter shit to make this less tragic
		T *dst = &(arr[index]);
		T *src = &(arr[index + 1]);
		auto res = static_cast<T*>(memmove(dst, src, (length - index)));
		assert(res != NULL && res == dst && "Popping failed");
	}

	size_t size() { return length; }

	T *operator[](size_t index) {
		assert(index < length && "Index out of bounds");

		return &(arr[index]);
	}

  private:
	void expand() {
		if (capacity >= 512'000)
			capacity += 100'000;
		else
			capacity *= 2;

		T *newArr = static_cast<T *>(realloc(arr, capacity * sizeof(T)));
		assert(arr != NULL && "Expansion of backing array failed (OOM)");
		arr = newArr;
	}

  private:
	T *arr;
	size_t capacity;
	size_t length;
};

struct InputData {
  public:
	InputData(std::shared_ptr<UnsafeVector<Operation>> gets,
			  std::shared_ptr<UnsafeVector<Operation>> puts)
		: gets(gets), puts(puts) {}

	std::shared_ptr<UnsafeVector<Operation>> gets;
	std::shared_ptr<UnsafeVector<Operation>> puts;
};

class AbstractParser {
  public:
	virtual InputData parse(const std::string &get, const std::string &put) = 0;
};

class AbstractExecutor {
  public:
	virtual void prepare(InputData data) = 0;
	virtual void execute() = 0;
};

template <class T, class V> class Benchmark {
  public:
	Benchmark() {
		static_assert(std::is_base_of<AbstractParser, T>::value,
					  "parser type does not derive AbstractParser");
		static_assert(std::is_base_of<AbstractExecutor, V>::value,
					  "executor type does not derive AbstractExecutor");

		parser = std::make_shared<T>();
		executor = std::make_shared<V>();
	}

	void run(const std::string &getOps, const std::string &putOps,
			 const std::string &output) {
		std::shared_ptr<AbstractParser> aparser =
			std::dynamic_pointer_cast<AbstractParser>(this->parser);
		std::shared_ptr<AbstractExecutor> aexecutor =
			std::dynamic_pointer_cast<AbstractExecutor>(this->executor);

		InputData data = aparser->parse(getOps, putOps);
		std::cout << "parse successful\n";
		aexecutor->prepare(data);
		std::cout << "prepare successful\n";
		aexecutor->execute();
	}

  private:
	std::shared_ptr<T> parser;
	std::shared_ptr<V> executor;
};
} // namespace bench