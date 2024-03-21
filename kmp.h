//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_KMP_H
#define PARALLEL_KMP_H

auto do_kmp_algorithm(const char* text, size_t text_len, const char* pattern, size_t pattern_len) -> std::vector<size_t>;

#endif //PARALLEL_KMP_H
