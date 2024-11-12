#include "utils.h"

#include <glm/glm.hpp>
#include <sstream>
#include <stdlib.h>

float RandBetween(float min, float max) {

    float range = max - min;
    float r = (float)rand() / (float)RAND_MAX;

    return min + r * range;
}

std::vector<std::string> SplitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    const char* str = input.c_str();

    while ( *str != '\0' ) {
        if ( *str != delimiter ) {
            std::string token;
            while ( *str != delimiter && *str != '\0' ) {
                token += *str;
                str++;
            }
            tokens.push_back(token);
        } else {
            str++;
        }
    }

    return tokens;
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