//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_UTIL_H
#define PARALLEL_UTIL_H

#include <utility>
#include <string>


/// Convert time in human-readable format.
auto display_time(long long microseconds) -> std::string;

/// Convert file size in human-readable format.
auto display_size(long long bytes) -> std::string;

#endif //PARALLEL_UTIL_H
