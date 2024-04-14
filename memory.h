//
// Created by sunnysab on 3/22/24.
//


#include <cstddef>
#include <cstdint>
#include <vector>

auto check_result_quickly(const uint8_t *p, size_t len, const char *pattern, const std::vector<size_t> &result) -> bool;

/// Clear memory and place *count* patterns in it randomly.
auto generate_test_data(uint8_t *base, size_t size, const char *pattern, size_t count) -> void;