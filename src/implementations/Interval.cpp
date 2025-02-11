#include "Interval.h"
namespace bench {
int Interval::compareTo(Interval &other) {
	if (this->start < other.start)
		return -1;
	else if (this->start == other.start)
		return this->end <= other.end ? -1 : 1;
	else
		return 1;
}

} // namespace bench