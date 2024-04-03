//
// Created by sunnysab on 3/22/24.
//

#include <random>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <format>
#include "memory.h"


auto check_result_quickly(const uint8_t* p, size_t len, const char *pattern, const std::vector<size_t>  &result) -> bool {
    auto pattern_length = strlen(pattern);

    return !std::any_of(result.begin(), result.end(), [&](size_t i) {
        if (i >= len) {
            std::cerr << "incorrect position, found a position out of range." << std::endl;
            return true;
        }

        // std::cout << "checking memory at 0x" << std::hex << reinterpret_cast<std::uintptr_t>(p + i) << std::endl;
        auto flag = memcmp(p + i, pattern, pattern_length) != 0;
        if (flag) {
            std::cerr << "incorrect position, found a position that does not match the pattern." << std::endl;
            for (auto pc = p + i; pc < p + i + pattern_length; pc++) {
                std::cerr << std::format("{:02x} ", *pc);
            }
            std::cerr << std::endl;
        }
        return flag;
    });
}



/// Allocate a piece of memory and place *count* patterns in it randomly.
auto generate_test_data(size_t size, const char *pattern, size_t count) -> const uint8_t* {
    auto pattern_len = strlen(pattern);
    if (size < pattern_len * count) {
        throw std::runtime_error("size is too small.");
    }

    auto p = new uint8_t[size];

    std::random_device rd;
    std::mt19937 gen(rd());
    // Designing a perfect placement algorithm can be complex. Here, we will simply evenly distribute the PATTERN
    // throughout the memory region.
    auto positions = std::vector<size_t>(count);
    auto block_size = size / count;
    std::uniform_int_distribution<int> dis(0, block_size - pattern_len);
    std::generate(positions.begin(), positions.end(), [&, i = 0, step = block_size]() mutable {
        return i++ * step + dis(gen);
    });

    auto i = 0;
    for (auto pos: positions) {
        auto _addr = p + pos;
        if (i++ < 4) {
            std::cout << "place pattern at 0x" << std::hex << reinterpret_cast<std::uintptr_t>(_addr) << std::endl;
        }
        memcpy(_addr, pattern, pattern_len);
    }

    auto flag = check_result_quickly(p, size, pattern, positions);
    if (!flag) {
        throw std::runtime_error("failed to place pattern in memory.");
    }
    return p;
}