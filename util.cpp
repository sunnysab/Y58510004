//
// Created by sunnysab on 3/21/24.
//


#include <chrono>
#include <sstream>
#include "util.h"


template<typename Func>
auto calc_execution_time(Func lambda) -> std::pair<decltype(std::declval<Func>()()), long long> {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = lambda();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return {result, duration};
}



auto display_time(long long microseconds) -> std::string {
    std::ostringstream stream;

    long long hours = microseconds / (1000LL * 1000 * 60 * 60);
    microseconds %= (1000LL * 1000 * 60 * 60);
    long long minutes = microseconds / (1000LL * 1000 * 60);
    microseconds %= (1000LL * 1000 * 60);
    long long seconds = microseconds / (1000LL * 1000);
    microseconds %= (1000LL * 1000);
    long long milliseconds = microseconds / 1000;
    microseconds %= 1000;

    if (hours > 0) {
        stream << hours << "h";
    }
    if (minutes > 0) {
        stream << minutes << "m";
    }
    if (seconds > 0) {
        stream << seconds << "s";
    }
    if (milliseconds > 0) {
        stream << "+" << milliseconds << "ms";
    }
    if (microseconds > 0) {
        stream << microseconds << "us";
    }

    return stream.str();
}
