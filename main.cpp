#include <cstdio>
#include <omp.h>
#include <cmath>


/// @brief 计算 x^2 一部分的面积
/// @param start 线程开始计算的位置
/// @param end   线程结束计算的位置
/// @param delta 长方形的边长
/// @return 计算出来的面积
double x_square_partial_integral(double start, double end, double delta) {
    double s = 0;

    for(double i = start; i < end; i += delta) {
        s += pow(i, 2) * delta;
    }
    return s;
}



int main() {
    int s = 0;
    int e = 10;
    double sum = 0;

    #pragma omp parallel num_threads(32) reduction(+:sum)
    {
        // 根据线程号进行计算区间的分配
        // omp_get_thread_num() 返回的线程 id 从 0 开始计数 ：0, 1, 2, 3, 4, ..., 31
        double start = (double)(e - s) / 32 * omp_get_thread_num();
        double end   = (double)(e - s) / 32 * (omp_get_thread_num() + 1);
        sum = x_square_partial_integral(start, end, 0.0000001);
    }

    printf("sum = %lf\n", sum);
    return 0;
}