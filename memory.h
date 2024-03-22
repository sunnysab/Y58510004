//
// Created by sunnysab on 3/22/24.
//


#include <cstddef>
#include <cstdint>
#include <vector>

auto check_result_quickly(const uint8_t* p, size_t len, const char *pattern, const std::vector<size_t>  &result) -> bool;

/// Allocate a piece of memory and place *count* patterns in it randomly.
auto generate_test_data(size_t size, const char *pattern, size_t count) -> const uint8_t*;