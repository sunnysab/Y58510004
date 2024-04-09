/// Find all occurrences of the pattern P, in a file F, based on OpenMP.
///
/// 2024.3.21
/// sunnysab

#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include "kmp.h"
#include "simd_search.h"
#include "util.h"
#include "file_mapper.h"
#include "memory.h"


struct Task {
    size_t  offset;
    size_t  size;

    Task() = default;
    Task(size_t offset, size_t size): offset(offset), size(size) {}
};


auto search_with_single_thread(const uint8_t* p, size_t total_length, const char *pattern)
    -> std::pair<std::vector<size_t>, long> {

    std::cout << "search_with_single_thread has been called." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto result = kmp_search(reinterpret_cast<const char *>(p), total_length, pattern, strlen(pattern));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    return {result, duration};
}

auto search_with_single_thread_simd(const uint8_t* p, size_t total_length, const char *pattern)
    -> std::pair<std::vector<size_t>, long> {

    std::cout << "search_with_single_thread_simd has been called." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto result = simd_search(reinterpret_cast<const char *>(p), total_length, pattern, strlen(pattern));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    return {result, duration};
}


auto search_with_openmp(const uint8_t* p, size_t total_length, const char *pattern, const unsigned int threads)
    -> std::pair<std::vector<size_t>, long> {

    std::cout << "search_with_openmp has been called." << std::endl;

    auto task_size = total_length / threads;
    std::vector<Task> tasks(threads);

    // Generate tasks.
    // Assume that total size is 395, we split it into 4 tasks. And the length to the pattern is 5.
    //  |__100__|__100__|__100__|__95__|
    // tasks are: [0, 100), [96, 200), [196, 300), [296, 395]
    auto base_addr = p;
    auto file_len = total_length;
    auto pattern_len = strlen(pattern);
    std::generate(tasks.begin(), tasks.end(), [&, i = 0]() mutable {
        auto start = i == 0 ? 0: (i * task_size - (pattern_len - 1));
        auto real_size = i == task_size - 1 ? std::min(task_size, file_len - i * task_size) : task_size;
        real_size += pattern_len;
        i++;
        return Task(start, real_size);
    });

    std::cout << "run algorithm in " << threads << " threads." << std::endl;

    auto mid_result = std::vector<std::vector<size_t>>(threads);
    auto start = std::chrono::high_resolution_clock::now();
    // Assign tasks to threads.
#pragma omp parallel num_threads(threads)
    {
        auto index = omp_get_thread_num();
        auto task = tasks[index];

        auto [_offset, _size] = task;
        auto result = kmp_search(reinterpret_cast<const char *>(base_addr + task.offset), _size, pattern, strlen(pattern));

        // kmp_search only returns the offset to the pattern from the start of the block, not the base address.
        for (auto &r: result) {
            r += _offset;
        }
        mid_result[index] = std::move(result);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    auto result = std::vector<size_t>();
    for (auto &r: mid_result) {
        result.insert(result.end(), r.begin(), r.end());
    }

    return {result, duration};
}

auto search_with_openmp_simd(const uint8_t* p, size_t total_length, const char *pattern, const unsigned int threads)
    -> std::pair<std::vector<size_t>, long> {

    std::cout << "search_with_openmp_simd has been called." << std::endl;

    auto task_size = total_length / threads;
    std::vector<Task> tasks(threads);

    // Generate tasks.
    // Assume that total size is 395, we split it into 4 tasks. And the length to the pattern is 5.
    //  |__100__|__100__|__100__|__95__|
    // tasks are: [0, 100), [96, 200), [196, 300), [296, 395]
    auto base_addr = p;
    auto file_len = total_length;
    auto pattern_len = strlen(pattern);
    std::generate(tasks.begin(), tasks.end(), [&, i = 0]() mutable {
        auto start = i == 0 ? 0: (i * task_size - (pattern_len - 1));
        auto real_size = i == task_size - 1 ? std::min(task_size, file_len - i * task_size) : task_size;
        real_size += pattern_len;
        i++;
        return Task(start, real_size);
    });

    std::cout << "run algorithm in " << threads << " threads." << std::endl;

    auto mid_result = std::vector<std::vector<size_t>>(threads);
    auto start = std::chrono::high_resolution_clock::now();
    // Assign tasks to threads.
#pragma omp parallel num_threads(threads)
    {
        auto index = omp_get_thread_num();
        auto task = tasks[index];

        auto [_offset, _size] = task;
        auto result = simd_search(reinterpret_cast<const char *>(base_addr + _offset), _size, pattern, strlen(pattern));

        // kmp_search only returns the offset to the pattern from the start of the block, not the base address.
        for (auto &r: result) {
            r += _offset;
        }
        mid_result[index] = std::move(result);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    auto result = std::vector<size_t>();
    for (auto &r: mid_result) {
        result.insert(result.end(), r.begin(), r.end());
    }

    return {result, duration};
}

auto search_in_file(const char *file, const char *pattern) {
    FileMapper  f(file);

    try {
        f.load();
    } catch (Exception &e) {
        std::cerr << e.what() << std::endl;
        return std::vector<size_t>();
    }

    auto [p, total_length] = std::tuple {f.get_start(), f.get_size()};
    std::cout << "file " << file << " loaded." << std::endl;
    std::cout << "[*] start = 0x" << std::hex << p << std::endl;
    std::cout << "[*] total_length = " << total_length << " bytes (" << display_size(total_length) << ")" << std::endl;

    auto [result, _duration] = search_with_openmp(p, total_length, pattern, 4);
    return result;
}

auto check_print_result(const uint8_t *text, size_t text_len, const char *pattern, const std::vector<size_t> &result) {
    std::cout << std::format("found PATTERN ({}) {} time(s).", pattern, result.size()) << std::endl;
    auto checker = check_result_quickly(text, text_len, pattern, result);

    if (!checker) {
        std::cerr << "result is incorrect." << std::endl;
    }
}

auto do_serial_test_in_memory(const uint8_t* p, const size_t size, const char *pattern)
-> std::vector<long> {

    auto [result1, duration1] = search_with_single_thread(p, size, pattern);
    check_print_result(p, size, pattern, result1);

    auto [result2, duration2] = search_with_single_thread_simd(p, size, pattern);
    check_print_result(p, size, pattern, result2);

    return {duration1, duration2};
}

auto do_parallel_test_in_memory(const uint8_t* p, const size_t size, const char *pattern, const unsigned int threads = 4)
    -> std::vector<long> {
    auto [result3, duration3] = search_with_openmp(p, size, pattern, threads);
    check_print_result(p, size, pattern, result3);

    auto [result4, duration4] = search_with_openmp_simd(p, size, pattern, threads);
    check_print_result(p, size, pattern, result4);

    return {duration3, duration4};
}



int main() {
    const auto MIN_MEMORY_USE = 128 * 1024 * 1024L;
    const auto MAX_MEMORY_USE = 8 * 1024 * 1024 * 1024L;
    const auto PATTERN = "PATTERN";

    // 一次分配，多次使用，提高测试性能.
    auto p = new uint8_t[MAX_MEMORY_USE];
    memset(p, 0, MAX_MEMORY_USE);

    // 内存大小
    for (auto size = MIN_MEMORY_USE; size <= MAX_MEMORY_USE; size *= 2) {

        generate_test_data(p, size, PATTERN, 5);
        auto durations = do_serial_test_in_memory(p, size, PATTERN);

        std::cout << "memory size: " << display_size(size)  << ", serial & SIMD costs: ";
        for (auto duration: durations) {
            std::cout << display_time(duration) << " ";
        }
        std::cout << std::endl;

        // 线程数
        for (auto cores = 2; cores <= 4; cores *= 2) {
            auto durations_with_threads = do_parallel_test_in_memory(p, size, PATTERN, cores);
            std::cout << "c" << cores << ": ";
            for (auto duration: durations_with_threads) {
                std::cout << display_time(duration) << ", ";
            }
            std::cout << std::endl;
        }
    }

    delete[] p;
    return 0;
}
