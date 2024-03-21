//
// Created by sunnysab on 3/21/24.
//
// Ref: https://www.tutorialspoint.com/c-program-for-kmp-algorithm-for-pattern-searching

#include <vector>
#include "kmp.h"


static void build_prefix_suffix_array(const char* pattern, size_t pattern_len, int* pps) {
    int length = 0;
    pps[0] = 0;
    size_t i = 1;
    while (i < pattern_len) {
        if (pattern[i] == pattern[length]) {
            length++;
            pps[i] = length;
            i++;
        }
        else {
            if (length != 0)
                length = pps[length - 1];
            else {
                pps[i] = 0;
                i++;
            }
        }
    }
}

auto do_kmp_algorithm(const char* text, const size_t text_len, const char* pattern, const size_t pattern_len) -> std::vector<size_t> {
    int pps[pattern_len];
    build_prefix_suffix_array(pattern, pattern_len, pps);

    std::vector<size_t>  result;
    int i = 0;
    int j = 0;
    while (i < text_len) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }
        if (j == pattern_len) {
            result.push_back(i - j);
            j = pps[j - 1];
        }
        else if (i < text_len && pattern[j] != text[i]) {
            if (j != 0)
                j = pps[j - 1];
            else
                i = i + 1;
        }
    }
    return result;
}