#ifndef _UTILS_H_
#define _UTILS_H_
#include <string>
#include <vector>

float RandBetween(float min, float max);

double GetDeltaTime();

std::vector<float> ParseFloatValues(const std::string& input);
#endif
