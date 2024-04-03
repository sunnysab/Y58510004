//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_KMP_H
#define PARALLEL_KMP_H

auto kmp_search(const char* text, const size_t text_len, const char* pattern, const size_t pattern_len) -> std::vector<size_t>;

#endif //PARALLEL_KMP_H
