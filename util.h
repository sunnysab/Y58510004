//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_UTIL_H
#define PARALLEL_UTIL_H

#include <utility>
#include <string>

template<typename Func>
auto calc_execution_time(Func lambda) -> std::pair<decltype(std::declval<Func>()()), long long>;

auto display_time(long long microseconds) -> std::string;


#endif //PARALLEL_UTIL_H
