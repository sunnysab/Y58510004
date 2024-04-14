//
// Created by sunnysab on 3/30/24.
//

#ifndef PARALLEL_SIMD_SEARCH_H
#define PARALLEL_SIMD_SEARCH_H

#include <vector>
#include <cstddef>

auto simd_search(const char *text, const size_t text_len, const char *pattern,
                 const size_t pattern_len) -> std::vector<size_t>;

#endif //PARALLEL_SIMD_SEARCH_H
