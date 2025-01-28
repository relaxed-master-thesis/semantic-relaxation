#include "Benchmark.h"
#include "ErrorCalculator.h"
#include <utility>

namespace bench {
class GeijerImp : public ErrorCalculator, public AbstractExecutor {
  public:
	GeijerImp() = default;
	~GeijerImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	void execute() override;

  private:
	struct item {
		uint64_t value;
		item *next;
	};
	Operations get_stamps;
	item *put_stamps_head;
	size_t put_stamps_size;
	size_t get_stamps_size;
};
} // namespace bench
