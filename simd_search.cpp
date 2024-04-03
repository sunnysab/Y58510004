//
// Created by sunnysab on 3/30/24.
//

#include <cstdint>
#include <cassert>
#include <vector>
#include <cstring>
#include <immintrin.h>


namespace bits {

    template <typename T>
    T clear_leftmost_set(const T value) {
        assert(value != 0);

        return value & (value - 1);
    }

    template <typename T>
    unsigned get_first_bit_set(const T value) {
        assert(value != 0);

        return __builtin_ctz(value);
    }

    template <>
    unsigned get_first_bit_set<uint64_t>(const uint64_t value) {
        assert(value != 0);

        return __builtin_ctzl(value);
    }
} // namespace bits


auto simd_search(const char* text, const size_t text_len, const char* pattern, const size_t pattern_len) -> std::vector<size_t> {
    std::vector<size_t> result;

    // 向寄存器中填充 needle 的第一个字节
    const __m256i first = _mm256_set1_epi8(text[0]);
    // 向寄存器中填充 needle 的最后一个字节
    const __m256i last  = _mm256_set1_epi8(pattern[pattern_len - 1]);

    for (size_t i = 0; i < text_len; i += 32) {

        // 向寄存器中填充 s 的部分内容
        const __m256i block_first = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(text + i));

        // 向寄存器中填充 s 的部分内容，相对于上一行，本次填充的内容有所偏移
        const __m256i block_last  = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(text + i + pattern_len - 1));

        // 比较两组寄存器
        const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
        const __m256i eq_last  = _mm256_cmpeq_epi8(last, block_last);

        // 合并两个寄存器的比较结果
        uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

        while (mask != 0) {
            // 找到第一个值为 1 的 bit 的下标
            const auto bitpos = bits::get_first_bit_set(mask);

            if (memcmp(text + i + bitpos + 1, pattern + 1, pattern_len - 2) == 0) {
                result.push_back(i + bitpos);
            }

            mask = bits::clear_leftmost_set(mask);
        }
    }

    return result;
}