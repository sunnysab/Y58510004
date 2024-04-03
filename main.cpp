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


auto do_task(Task  &task, const uint8_t *base, const char *pattern) -> std::vector<size_t> {
    auto [_offset, _size] = task;
    auto result = kmp_search(reinterpret_cast<const char *>(base + _offset), _size, pattern, strlen(pattern));

    // kmp_search only returns the offset to the pattern from the start of the block, not the base address.
    for (auto &r: result) {
        r += _offset;
    }
    return result;
}

auto search_with_single_thread(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t> {
    std::cout << "search_with_single_thread has been called." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto result = kmp_search(reinterpret_cast<const char *>(p), total_length, pattern, strlen(pattern));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    std::cout << std::format("found PATTERN ({}) {} times.", pattern, result.size()) << std::endl;
    std::cout << "checking result..." << std::endl;
    auto checker = check_result_quickly(p, total_length, pattern, result);

    if (checker) {
        std::cout << "result is correct." << std::endl;
    } else {
        std::cout << "result is incorrect." << std::endl;
    }
    return result;
}

auto search_with_single_thread_simd(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t> {
    std::cout << "search_with_single_thread_simd has been called." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto result = simd_search(reinterpret_cast<const char *>(p), total_length, pattern, strlen(pattern));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    std::cout << std::format("found PATTERN ({}) {} times.", pattern, result.size()) << std::endl;
    std::cout << "checking result..." << std::endl;
    auto checker = check_result_quickly(p, total_length, pattern, result);

    if (checker) {
        std::cout << "result is correct." << std::endl;
    } else {
        std::cout << "result is incorrect." << std::endl;
    }
    return result;
}


auto search_with_openmp(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t> {
    std::cout << "search_with_openmp has been called." << std::endl;

    auto processor_count = 4;
    auto task_size = total_length / processor_count;
    std::vector<Task> tasks(processor_count);

    // Generate tasks.
    // Assume that total size is 395, we split it into 4 tasks. And the length to the pattern is 5.
    //  |__100__|__100__|__100__|__95__|
    // tasks are: [0, 100), [96, 200), [196, 300), [296, 395]
    auto addr = p;
    auto file_len = total_length;
    auto pattern_len = strlen(pattern);
    std::generate(tasks.begin(), tasks.end(), [&, i = 0]() mutable {
        auto start = i == 0 ? 0: (i * task_size - (pattern_len - 1));
        auto real_size = i == task_size - 1 ? std::min(task_size, file_len - i * task_size) : task_size;
        real_size += pattern_len;
        i++;
        return Task(start, real_size);
    });

    std::cout << "run algorithm in " << processor_count << " threads." << std::endl;

    auto mid_result = std::vector<std::vector<size_t>>(processor_count);
    auto start = std::chrono::high_resolution_clock::now();
    // Assign tasks to threads.
#pragma omp parallel num_threads(processor_count)
    {
        auto index = omp_get_thread_num();
        auto task = tasks[index];

        mid_result[index] = do_task(task, addr, pattern);
    };

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "task finished, costs " << std::dec << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    auto result = std::vector<size_t>();
    for (auto &r: mid_result) {
        result.insert(result.end(), r.begin(), r.end());
    }

    std::cout << std::format("found PATTERN ({}) {} times.", pattern, result.size()) << std::endl;
    std::cout << "checking result..." << std::endl;
    auto checker = check_result_quickly(p, total_length, pattern, result);

    if (checker) {
        std::cout << "result is correct." << std::endl;
    } else {
        std::cout << "result is incorrect." << std::endl;
    }
    return result;
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

    auto result = search_with_openmp(p, total_length, pattern);
    return result;
}

auto do_test_in_memory(const int size, const char *pattern, const int count) {
    auto p = generate_test_data(size, pattern, count);

    auto result1 = search_with_openmp(p, size, pattern);
    auto result2 = search_with_single_thread(p, size, pattern);
    auto result3 = search_with_single_thread_simd(p, size, pattern);

    delete[] p;
}

int main() {
    do_test_in_memory(1024 * 1024 * 1024, "PATTERN", 5);
    return 0;
}
