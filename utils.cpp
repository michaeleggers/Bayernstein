#include "utils.h"

#include <stdlib.h>

float RandBetween(float min, float max) {

    float range = max - min;
    float r = (float)rand() / (float)RAND_MAX;

    return min + r * range;
}

// TODO: This function is very similar to the one in map_parser.
// We need to to some kind of parsing (for config-files etc.)
// anyways in the future so we could think about
// writing a more generic parser that deals with those
// things.
static void advanceToNextNonWhiteSpace(char* c) {
    while (isspace(*c)) {
        c++;
    }
}

bool IsStringFloat(const std::string& input) {
    std::string test = input;
    char* c = test.data();
    advanceToNextNonWhiteSpace(c);
    if ( *c == '-' || *c == '+' ) { // -, + allowed at the beginning of the number
        c++;
    }
    if ( *c == '.' ) { // leading . allowed
        c++;
    }
    bool dotAppeared = false;
    while ( *c != '\0' ) {
        if ( *c >= '0' && *c <= '9' ) {
            c++;
        }
        else if ( !dotAppeared && *c == '.' ) {
            dotAppeared = true;
            c++;
        }
        else {
            return false;
        }
    }

    return true;
}

std::vector<float> ParseFloatValues(const std::string& input) {
    std::vector<float> values;
    const char* str = input.c_str();
    float value = 0.0;
    bool parsingNumber = false;

    while (*str != '\0') {
        if (std::isdigit(*str) || *str == '-' || *str == '.') {
            parsingNumber = true;
            char* end;
            value = std::strtof(str, &end);
            values.push_back(value);
            str = end;
        } else if (parsingNumber && *str == ' ') {
            parsingNumber = false;
            str++;
        } else {
            str++;
        }
    }

    return values;
}
