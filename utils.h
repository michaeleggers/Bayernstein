#ifndef _UTILS_H_
#define _UTILS_H_
#include <glm/glm.hpp>
#include <string>
#include <vector>

float RandBetween(float min, float max);

double GetDeltaTime();

std::vector<float> ParseFloatValues(const std::string& input);

glm::vec3 Truncate(const glm::vec3& v, float max);
#endif
