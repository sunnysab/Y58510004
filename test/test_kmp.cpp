//
// Created by sunnysab on 3/21/24.
//


#include <gtest/gtest.h>
#include "kmp.h"


TEST(KMP, TestEmptyString) {
    const char *text = "";
    const char *pattern = "PATTERN";

    auto result = do_kmp_algorithm(text, strlen(text), pattern, strlen(pattern));
    std::vector<size_t> expected = {};

    ASSERT_EQ(result, expected);
}

TEST(KMP, Test1) {
    const char *text = "ABABDABACDABABCABAB";
    const char *pattern = "ABABCABAB";

    auto result = do_kmp_algorithm(text, strlen(text), pattern, strlen(pattern));
    std::vector<size_t> expected = {10};

    ASSERT_EQ(result, expected);
}