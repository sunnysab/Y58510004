/// Find all occurrences of the pattern P, in a file F, based on OpenMP.
///
/// 2024.3.21
/// sunnysab

#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <sys/mman.h>
#include <omp.h>
#include "kmp.h"
#include "util.h"
#include "file_mapper.h"


auto check_result_quickly(const uint8_t* p, size_t len, const char *pattern, const std::vector<size_t>  &result) -> bool {
    return std::all_of(result.begin(), result.end(), [&](size_t i) {
        return memcmp(p + i, pattern, strlen(pattern)) == 0;
    });
}

auto do_kmp_in_parallel(const char *file, const char *pattern) {
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
    std::cout << "task finished, costs " << duration << " microseconds (" << display_time(duration) << ")" << std::endl;

    auto result = std::vector<size_t>();
    for (auto &r: mid_result) {
        result.insert(result.end(), r.begin(), r.end());
    }

    std::cout << std::format("found PATTERN ({}) {} times.", pattern, result.size());
    std::cout << "checking result..." << std::endl;
    auto checker = check_result_quickly(p, total_length, pattern, result);

    if (checker) {
        std::cout << "result is correct." << std::endl;
    } else {
        std::cout << "result is incorrect." << std::endl;
    }
}


int main() {
    auto device_count = omp_get_num_procs();
    std::cout << "device count = " << device_count << std::endl;

    do_kmp_in_parallel("test.txt", "PATTERN");

    return 0;
}
