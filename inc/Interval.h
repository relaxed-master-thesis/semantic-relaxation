#include <algorithm>
#include <cstdint>
#include <memory>
namespace bench {
class Interval {
  public:
	Interval(uint64_t start, uint64_t end) : start(start), end(end), max(end) {}
	~Interval() = default;

	int compareTo(Interval &other);
    // {
	// 	if (this->start < other.start)
	// 		return -1;
	// 	else if (this->start == other.start)
	// 		return this->end <= other.end ? -1 : 1;
	// 	else
	// 		return 1;
	// }

	uint64_t start;
	uint64_t end;
    uint64_t max;
	std::shared_ptr<Interval> left;
	std::shared_ptr<Interval> right;
  private:
};

}