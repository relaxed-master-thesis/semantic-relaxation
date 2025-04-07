
namespace bench {
static unsigned long x = 123456789, y = 362436069, z = 521288629;
static void reset_random() {
	x = 123456789;
	y = 362436069;
	z = 521288629;
}

inline unsigned long xorshf96(void) { // period 2^96-1
	unsigned long t;
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;

	t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;

	return z;
}
}