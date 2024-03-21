//
// Created by sunnysab on 3/21/24.
//


#include <chrono>
#include <sstream>
#include <cmath>
#include <array>
#include "util.h"

/// Calculate execution time of a lambda function.
/// \tparam Func
/// \param lambda: the lambda function to be executed and measured.
/// \return a pair of result of lambda function and execution time in microseconds.
template<typename Func>
auto calc_execution_time(Func lambda) -> std::pair<decltype(std::declval<Func>()()), long long> {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = lambda();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return {result, duration};
}


/// Convert time in human-readable format.
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



/// Convert file size in human-readable format.
auto display_size(long long bytes) -> std::string {
    static const std::array<std::string, 7> suffixes = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    std::string formattedSize;
    std::ostringstream stream;

    if (bytes == 0) {
        return "0 B";
    }

    int i = static_cast<int>(std::log(bytes) / std::log(1024));
    // 防止越界
    if (i >= suffixes.size()) i = suffixes.size() - 1;

    double count = bytes / std::pow(1024, i);
    stream.precision(2);
    stream << std::fixed << count << " " << suffixes[i];
    formattedSize = stream.str();
    return formattedSize;
}
