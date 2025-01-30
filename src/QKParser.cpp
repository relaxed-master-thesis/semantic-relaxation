#include "QKParser.h"
#include "Benchmark.h"
#include "Operation.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace bench {

static std::shared_ptr<std::vector<Operation>>
parseFile(const std::string &file) {
	// auto ops = std::make_shared<UnsafeVector<Operation>>(512'000);
	auto ops = std::make_shared<std::vector<Operation>>();

	uint64_t time, value;
	std::ifstream infile(file);
	while (infile >> time >> value) {
		Operation op{time, value};
		ops->emplace_back(time, value);
	}

	std::cout << "vec sizes: " << ops->size() << ", " << ops->max_size() << "\n";
	return ops;
}

InputData QKParser::parse(const std::string &getOps,
						  const std::string &putOps) {
	return InputData(parseFile(getOps), parseFile(putOps));
}

} // namespace bench