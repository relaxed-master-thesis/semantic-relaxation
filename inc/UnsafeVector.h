#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>

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
		if (length == capacity) expand();
		T *res = static_cast<T *>(memcpy(&(arr[length]), &t, sizeof(T)));
		assert(res != NULL && res == &(arr[length++]) && "Pushing failed");
	}

	void pop(size_t index) { 
		assert(index < length && "Index out of bounds");
		auto res = static_cast<T*>(memmove(&(arr[index]), &(arr[index + 1]), (length-- - index)));
		assert(res != NULL && res == &(arr[index]) && "Popping failed");
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
