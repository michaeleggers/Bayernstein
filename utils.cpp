#include "utils.h"

#include <cstdlib>

float RandBetween(const float min, const float max) {
	const float range = max - min;
	const float r = (float)rand() / (float)RAND_MAX;

	return min + r * range;
}
