/// Find all occurrences of the pattern P, in a file F, based on OpenMP.
///
/// 2024.3.21
/// sunnysab

#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include "kmp.h"
#include "util.h"
#include "file_mapper.h"


/// Allocate a piece of memory and place *count* patterns in it randomly.
auto generate_test_data(size_t size, const char *pattern, size_t count) -> const uint8_t* {
    auto pattern_len = strlen(pattern);
    if (size < pattern_len * count) {
        throw std::runtime_error("size is too small.");
    }

    auto p = new uint8_t[size];

    std::random_device rd;
    std::mt19937 gen(rd());
    // Designing a perfect placement algorithm can be complex. Here, we will simply evenly distribute the PATTERN
    // throughout the memory region.
    auto positions = std::vector<uint8_t*>(count);
    auto block_size = size / count;
    std::uniform_int_distribution<int> dis(0, block_size - pattern_len);
    std::generate(positions.begin(), positions.end(), [&, i = 0, step = size / count]() mutable {
        return p + i++ * step + dis(gen);
    });

    for (auto pos: positions) {
        std::cout << "place pattern at 0x" << std::hex << reinterpret_cast<std::uintptr_t>(pos) << std::endl;
        memcpy(pos, pattern, pattern_len);
    }

    return p;
}

auto check_result_quickly(const uint8_t* p, size_t len, const char *pattern, const std::vector<size_t>  &result) -> bool {
    auto pattern_length = strlen(pattern);

    return !std::any_of(result.begin(), result.end(), [&](size_t i) {
        if (result[i] >= len) {
            std::cout << "incorrect position, found a position out of range." << std::endl;
            return true;
        }

        auto flag = memcmp(p + i, pattern, pattern_length) != 0;
        if (flag) {
            std::cout << "incorrect position, found a position that does not match the pattern." << std::endl;
            for (auto pc = p + i; pc < p + i + pattern_length; pc++) {
                std::cout << std::format("{:02x} ", *pc);
            }
            std::cout << std::endl;
        }
        return flag;
    });
}

auto search_with_openmp(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t> {
    auto processor_count = omp_get_num_procs();
    auto task_size = total_length / processor_count;
    std::vector<std::pair<const uint8_t*, size_t>> tasks(task_size);

    // Generate tasks.
    // Assume that total size is 395, we split it into 4 tasks. And the length to the pattern is 5.
    //  |__100__|__100__|__100__|__95__|
    // tasks are: [0, 100), [96, 200), [196, 300), [296, 395]
    auto addr = p;
    auto file_len = total_length;
    auto pattern_len = strlen(pattern);
    std::generate(tasks.begin(), tasks.end(), [&, i = 0]() mutable {
        auto start = i == 0 ? addr: (addr + i * task_size - (pattern_len - 1));
        auto real_size = i == task_size - 1 ? std::min(task_size, file_len - i * task_size) : task_size;
        real_size += pattern_len;
        i++;
        return std::make_pair(start, real_size);
    });

    std::cout << "run algorithm in " << processor_count << " threads." << std::endl;

    auto mid_result = std::vector<std::vector<size_t>>(processor_count);
    auto start = std::chrono::high_resolution_clock::now();
    // Assign tasks to threads.
#pragma omp parallel
    {
        auto index = omp_get_thread_num();
        auto [_start, _size] = tasks[index];

        mid_result[index] = do_kmp_algorithm(reinterpret_cast<const char *>(_start), _size, pattern, pattern_len);
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
}

auto search_in_memory() {
    constexpr int SIZE = 1024 * 1024 * 1024;
    auto p = generate_test_data(1024 * 1024 * 1024, "PATTERN", 10);
    auto result = search_with_openmp(p, 1024 * 1024 * 1024, "PATTERN");
    delete[] p;
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
}


int main() {
    auto device_count = omp_get_num_procs();
    std::cout << "device count = " << device_count << std::endl;

    search_in_memory();
    return 0;
}
