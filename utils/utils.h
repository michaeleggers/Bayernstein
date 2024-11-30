#ifndef _UTILS_H_
#define _UTILS_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>

double                      GetDeltaTime();
float                       RandBetween(float min, float max);
bool                        IsStringFloat(const std::string& string);
std::vector<std::string>    SplitString(const std::string& input, char delimiter);

template<typename T>
T StringToFloat(const char* str, char** end);

template<>
float StringToFloat<float>(const char* str, char** end);

template<>
double StringToFloat<double>(const char* str, char** end);

template<>
int StringToFloat<int>(const char* str, char** end);

template<typename T>
std::vector<T> ParseValues(const std::string& input) {
    std::vector<T> values;
    const char* str = input.c_str();
    T value = 0.0;
    bool parsingNumber = false;

    while ( *str != '\0' ) {
        if ( std::isdigit(*str) || *str == '-' || *str == '.' ) {
            parsingNumber = true;
            char* end;
            value = StringToFloat<T>(str, &end);
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


#endif

