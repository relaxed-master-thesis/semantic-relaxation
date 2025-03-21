#include "bench/util/TimestampParser.h"
#include "bench/Operation.h"
#include "bench/util/InputData.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace bench {

static std::shared_ptr<std::vector<Operation>>
parseFile(const std::string &file) {
	uint64_t time, value;
	std::ifstream infile(file);
	auto ops = std::make_shared<std::vector<Operation>>();
	if (!infile.good()) {
		std::cerr << "File \"" << file << "\" not found\n";
		return ops;
	}

	while (infile >> time >> value) {
		ops->emplace_back(time, value);
	}

	return ops;
}

InputData TimestampParser::parse(const std::string &getOps,
								 const std::string &putOps) {
	const auto gets = parseFile(getOps);
	const auto puts = parseFile(putOps);
	return InputData(gets, puts);
}
} // namespace bench