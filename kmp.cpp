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


// KMP搜索算法：在文本中搜索模式串，返回所有匹配的起始索引位置。
// @param text 指向文本字符串的指针。
// @param text_len 文本的长度。
// @param pattern 指向模式串字符串的指针。
// @param pattern_len 模式串的长度。
// @return 一个包含匹配索引位置的向量（vector）。
auto kmp_search(const char* text, const size_t text_len, const char* pattern, const size_t pattern_len) -> std::vector<size_t> {
    // 部分匹配表（Partial Match Table），也称为前缀后缀表（Prefix-Suffix Table）。
    int pps[pattern_len];
    // 构建前缀后缀数组，为匹配过程提供跳转信息以避免冗余检查。
    build_prefix_suffix_array(pattern, pattern_len, pps);

    // 用于存放匹配结果的索引位置。
    std::vector<size_t> result;
    // i用于遍历文本，j用于遍历模式串。
    int i = 0;
    int j = 0;
    // 遍历整个文本字符串。
    while (i < text_len) {
        // 如果当前字符匹配成功，则模式串和文本都向后移动一个字符。
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }
        // 完整匹配，将当前匹配的起始索引加入结果。
        if (j == pattern_len) {
            result.push_back(i - j);
            // 根据部分匹配表调整模式串指针j。
            j = pps[j - 1];
        }
        // 如果字符不匹配，并且i没有到达文本尾部。
        else if (i < text_len && pattern[j] != text[i]) {
            // j不为0时根据部分匹配表回溯。
            // 不是从模式串的开始位置重新匹配，j回到有最大前缀后缀匹配长度的位置。
            if (j != 0)
                j = pps[j - 1];
                // j为0时，则移动文本指针i。
            else
                i = i + 1;
        }
    }
    // 返回匹配结果的集合。
    return result;
}
