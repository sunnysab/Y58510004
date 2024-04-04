
# 并行字符串匹配实验

本项目是“分布式并行计算”课程的一部分，用以比较串行和不同并行方法在字符串匹配上的性能。在本项目中，我们实（fu）现（zhi）了 KMP 字符串匹配算法，并引入 OpenMP 支持的多线程技术，对其执行效率进行了显著的提升，同时对比了单线程和多线程执行环境下的性能差异。
进一步地，我们还研究了应用 SIMD 指令集对匹配流程进行加速的相关工作，并将其纳入我们的性能评估对比中。

鉴于项目时间紧迫，代码中可能缺乏详尽的注释。但函数与变量命名的直观性，仍旧保障了代码的可读性和理解性。
需要指出的是，由于项目周期和本人专业水平所限，这仅是对并行优化的一次初步尝试，并呈现出一种结课项目的“玩具级”实现。

另外，有些资料介绍了利用 OpenCL 和 GPU 进行字符串匹配加速的方案，表现出较大的潜力和优势。如果对此有进一步的兴趣和探索欲，你可以扩充项目，将这部分内容融入我们的探索之旅。

## 如何阅读

项目基于 CMake 构建，
本项目依赖了 Google Test 作为单元测试套件。同时提供了一个可选的 mmap 封装（位于 `file_mapper.h`），该封装要求你使用 Linux 系统。

```shell
$ tree .
.
├── CMakeLists.txt    # CMake 构建文件
├── exception.h       # 异常类（便于抛出错误信息）
├── file_mapper.h     # FileMapper, 用于将文件映射到内存
├── kmp.cpp           # KMP 算法实现
├── kmp.h
├── main.cpp          # 实验主体
├── memory.cpp        # 测试数据的生成，及实验结果的检查
├── memory.h
├── README.md
├── simd_search.cpp   # 基于 SIMD 的搜索的实现
├── simd_search.h
├── test              # 算法的单元测试
│   ├── test_kmp.cpp
│   └── test_simd.cpp
├── util.cpp          # 用于输出相关格式转换
└── util.h
```

阅读 `main.cpp`，不难发现，程序的执行流程分为三部分：生成测试数据、执行、检查测试结果。下面简单地介绍。

- **生成测试数据**

   ```cpp
  auto generate_test_data(size_t size, const char *pattern, size_t count) -> const uint8_t*;
  ```
  函数申请一段指定长度的内存。由于我们希望尽量均匀地在存储空间中生成测试数据，不妨将可用内存分为 `count` 个区块，在每个区块中放置一个模式串，也能起到使其均匀分布的作用。
  注意，函数中使用了 `new` 申请内存区域，需要配合 `delete` 释放内存。

- **并行查找**

   在 `main.cpp` 中，我们创建了以下三个函数，分别通过单线程、SIMD、OpenMP 实现字符串匹配：
   ```cpp
  auto search_with_single_thread(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t>;
  auto search_with_single_thread_simd(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t>;
  auto search_with_openmp(const uint8_t* p, size_t total_length, const char *pattern) -> std::vector<size_t>;
  ```

   在实现基于 OpenMP 的方法时需要注意，`kmp_search` 函数所返回的子串偏移量是相对于该任务的起始位置的，因此我们需要将结果换算成相对于整个查找区域的偏移量。
   此外，对于并行的任务，我们在启动线程前将查找工作分解为多个任务（Task），并计算其起始偏移量和任务长度。对于非第一个任务，它们的起始偏移量要向前一点点（`pattern_len - 1`），以保证搜索过程中不会有遗漏。任务划分的代码如下：
   ```cpp
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
   ```

- **检查测试结果**

## 测试

作者使用的环境为 Archlinux (2024.4.3, linux 6.7.2.arch2-1)，CMake 3.29.0。 如果你使用 Google Test，建议不低于 1.14.0-1 版本。
推荐使用 CLion 作为开发环境。

### 测试过程

在终端下的测试方法：

```shell
$ mkdir build
$ cd build; cmake ..
$ cmake --build . --config Release
```

编译 & 链接完成后，目录下会存在 `parallel`、`test_kmp`、`test_simd_search` 三个文件，执行 `./parallel` 即可。

## 并行效果

我们在不同大小的测试数据上，测试了串行、多线程、SIMD、多线程+SIMD下字符串匹配的用时。测试机器为：AMD 5800H（内存频率 3200MHz）。
我们在内存中放置 5 个模式串。由于该变量不影响字符串匹配效率，这里不对其进行测试。

单线程测试：

| 大小 | 串行    | SIMD  | 加速比 |
| ---- | ------- | ----- | ------ |
| 128M | 158ms   | 4ms   |        |
| 256M | 413ms   | 9ms   | 4589%  |
| 512M | 829ms   | 19ms  | 4363%  |
| 1G   | 1s673ms | 39ms  | 4289%  |
| 2G   | 2448ms  | 68ms  |        |
| 4G   | 2448ms  | 138ms |        |
| 8G   | 2456ms  | 280ms |        |

多线程测试：

| 大小 | 多线程       | 多线程 + SIMD | Cores |
| ---- | ------------ | ------------- | ----- |
| 128M | 46ms         | 1ms           | 2     |
| 256M | 210ms (196%) | 5ms (8260%)   | 2     |
| 512M | 421ms (196%) | 10ms (8290%)  | 2     |
| 1G   | 878ms (190%) | 19ms (8805%)  | 2     |
| 2G   | 749ms        | 25ms          | 2     |
| 4G   | 1485ms       | 50ms          | 2     |
| 8G   | 2961ms       | 94ms          | 2     |
| 128M | 46ms         | 1ms           | 4     |
| 256M | 119ms (347%) | 3ms (13767%)  | 4     |
| 512M | 245ms (338%) | 5ms (16580%)  | 4     |
| 1G   | 515ms (324%) | 10ms (16730%) | 4     |
| 2G   | 745ms        | 26ms          | 4     |
| 4G   | 1482ms       | 49ms          | 4     |
| 8G   | 2954ms       | 94ms          | 4     |
| 128M | 47ms         | 1ms           | 8     |
| 256M | 93ms         | 3ms           | 8     |
| 512M | 186ms        | 6ms           | 8     |
| 1G   | 375ms        | 11ms          | 8     |
| 2G   | 745ms        | 25ms          | 8     |
| 4G   | 1482ms       | 48ms          | 8     |
| 8G   | 2952ms       | 92ms          | 8     |
| 128M | 46ms         | 1ms           | 16    |
| 256M | 95ms         | 3ms           | 16    |
| 512M | 188ms        | 6ms           | 16    |
| 1G   | 375ms        | 11ms          | 16    |
| 2G   | 744ms        | 24ms          | 16    |
| 4G   | 1485ms       | 49ms          | 16    |
| 8G   | 2948ms       | 94ms          | 16    |



## 声明

本仓库代码为烟台大学研究生课程“并行与分布式计算”的课程项目。如果你是计控学院的学弟学妹，欢迎你参考本项目。
希望你能抛开那二十年前的幻灯片，多查阅资料，真正学到一些有用的东西。	