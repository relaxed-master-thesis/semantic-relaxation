#include "QKParser.h"
#include "Benchmark.h"
#include "Operation.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace bench {

static Operations *parseFile(const std::string &file) {
	// const size_t elems = 600'000;
	Operations ops{};
	std::cout << sizeof(Operation) << " bytes\n";
	ops->reserve(600'000);

	uint64_t time, value;
	std::ifstream infile(file);
	int numLines = 0;
	while (infile >> time >> value) {
		ops->emplace_back(time, value);
	}

	return ops;
}

InputData QKParser::parse(const std::string &getOps,
						  const std::string &putOps) {
	return InputData(parseFile(getOps), parseFile(putOps));
}

} // namespace bench