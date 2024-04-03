//
// Created by sunnysab on 24-4-3.
//

#include <gtest/gtest.h>
#include "simd_search.h"

TEST(SIMD, TestEmptyString) {
    const char *text = "";
    const char *pattern = "PATTERN";

    auto result = simd_search(text, strlen(text), pattern, strlen(pattern));
    std::vector<size_t> expected = {};

    ASSERT_EQ(result, expected);
}

TEST(SIMD, Test1) {
    const char *text = "ABABDABACDABABCABAB";
    const char *pattern = "ABABCABAB";

    auto result = simd_search(text, strlen(text), pattern, strlen(pattern));
    std::vector<size_t> expected = {10};

    ASSERT_EQ(result, expected);
}

TEST(SIMD, Test2) {
    auto buffer = new char [1 << 20];
    auto pattern = "ABABCABAB";
    auto pattern_length = strlen(pattern);

    auto expected = std::vector<size_t> {10, 1000, 10000, 100000, 1000000};
    for (auto offset: expected) {
        memcpy(buffer + offset, pattern, pattern_length);
    }

    auto result = simd_search(buffer, 1 << 20, pattern, pattern_length);
    delete[] buffer;

    ASSERT_EQ(result.size(), expected.size());
    ASSERT_EQ(result, expected);
}