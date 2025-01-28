#include "QKParser.h"
#include "Benchmark.h"
#include "Operation.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

namespace bench {

static std::shared_ptr<UnsafeVector<Operation>>
parseFile(const std::string &file) {
	auto ops = std::make_shared<UnsafeVector<Operation>>(512'000);

	uint64_t time, value;
	std::ifstream infile(file);
	while (infile >> time >> value) {
		Operation op{time, value};
		ops->push(op);
	}

	return ops;
}

InputData QKParser::parse(const std::string &getOps,
						  const std::string &putOps) {
	return InputData(parseFile(getOps), parseFile(putOps));
}

} // namespace bench