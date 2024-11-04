#include "utils.h"

#include <glm/glm.hpp>
#include <stdlib.h>

float RandBetween(float min, float max) {

    float range = max - min;
    float r = (float)rand() / (float)RAND_MAX;

    return min + r * range;
}

std::vector<float> ParseFloatValues(const std::string& input) {
    std::vector<float> values;
    const char* str = input.c_str();
    float value = 0.0;
    bool parsingNumber = false;

    while ( *str != '\0' ) {
        if ( std::isdigit(*str) || *str == '-' || *str == '.' ) {
            parsingNumber = true;
            char* end;
            value = std::strtof(str, &end);
            values.push_back(value);
            str = end;
        } else if ( parsingNumber && *str == ' ' ) {
            parsingNumber = false;
            str++;
        } else {
            str++;
        }
    }

    return values;
}