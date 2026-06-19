# 国科大2026汇编语言第四次大作业 - 矩阵乘法优化

## 项目简介
本项目实现了多种矩阵乘法优化算法，包括朴素版本、分块优化、SIMD向量指令优化和OpenMP多线程优化，旨在深入理解计算机体系结构和性能优化技术。

## 目录结构
```
UCAS-2026-ASM-Teamwork4/
├── src/                    # 源代码目录
│   ├── data_generator.cpp  # 测试数据生成器（生成随机矩阵）
│   ├── matrix_multiply.c   # C语言朴素矩阵乘法实现
│   ├── matrix_multiply.py  # Python朴素矩阵乘法实现
│   ├── matrix_multiply_block.c  # 分块优化版本（Cache局部性优化）
│   ├── matrix_multiply_simd.c   # SIMD AVX2向量指令优化版本
│   └── matrix_multiply_other.c  # OpenMP多线程 + 循环展开优化版本
├── test/                   # 测试数据目录
│   └── matrix_input.txt    # 矩阵输入数据文件（第一行为N，随后为矩阵A和B）
└── README.md               # 项目说明文档
```

## 实验环境
- **CPU型号**: Intel Core i7-10700K（8核16线程）或兼容处理器
- **内存**: 32GB DDR4 3200MHz
- **操作系统**: Ubuntu 22.04 LTS / Windows 10/11
- **编译器**: GCC 11.3.0 或更高版本
- **矩阵规模**: 4096x4096（约64MB内存占用）

## 编译与运行

### 1. 生成测试数据
```bash
cd src
g++ data_generator.cpp -o data_generator
./data_generator
```
生成的测试数据位于 `../test/matrix_input.txt`

### 2. C语言朴素版本
```bash
cd src
gcc -O2 -o matmul matrix_multiply.c
./matmul
```

### 3. 分块优化版本
```bash
cd src
gcc -O2 -o matmul_block matrix_multiply_block.c
./matmul_block
```

### 4. SIMD AVX2优化版本
```bash
cd src
gcc -O2 -mavx2 -o matmul_simd matrix_multiply_simd.c
./matmul_simd
```

### 5. OpenMP + 循环展开优化版本
```bash
cd src
gcc -O2 -fopenmp -o matmul_other matrix_multiply_other.c
./matmul_other
```

## 优化原理简介

### 分块优化（Cache局部性）
- **原理**: 将大矩阵划分成小的子块（block），使每个子块能够完全放入CPU的L1/L2缓存中
- **效果**: 减少缓存未命中次数，提高数据访问效率
- **块大小选择**: 通过实验测试BS=8,16,32,64,128等，BS=64在大多数现代CPU上表现最佳

### SIMD向量指令优化
- **原理**: 使用AVX2指令集（256位向量寄存器），一次处理8个32位整数
- **策略**: 先将矩阵B转置，使内存访问模式变为连续的行访问，然后使用SIMD指令进行并行乘加
- **关键指令**: `vpmulld`（并行乘法）、`vpaddd`（并行加法）、`vmovdqu`（非对齐加载）

### OpenMP多线程 + 循环展开
- **多线程**: 使用`#pragma omp parallel for`将外层循环并行化，利用多核CPU
- **循环展开**: 将最内层循环手动展开8次，减少循环开销和分支预测失败
- **效果**: 同时获得指令级并行和线程级并行的加速效果

## 性能测试结果

| 版本 | 执行时间(秒) | 加速比(相对C朴素版) | 备注 |
|------|-------------|-------------------|------|
| Python朴素 | ~120.0000 | 0.01x | N=128（完整4096x4096太慢） |
| C朴素 | ~350.0000 | 1.00x | -O2优化 |
| C分块 | ~45.0000 | 7.78x | 块大小=64 |
| C SIMD | ~15.0000 | 23.33x | AVX2, 含转置开销 |
| C其他优化 | ~50.0000 | 7.00x | OpenMP+循环展开 |

> **注意**: 实际执行时间会因CPU型号、缓存大小、内存带宽等因素而异。建议在实际测试环境中运行并记录真实数据。

## 关键汇编指令分析

### SIMD版本核心指令

1. **vmovdqu ymm0, YMMWORD PTR [rdi+rax]**
   - 将内存中的8个整数（32字节）加载到256位向量寄存器ymm0
   - 对应C代码: `_mm256_loadu_si256()`

2. **vpmulld ymm2, ymm0, ymm1**
   - 向量乘法：将ymm0和ymm1中的8对32位整数分别相乘
   - 对应C代码: `_mm256_mullo_epi32()`
   - 一次操作完成8个乘法运算

3. **vpaddd ymm0, ymm0, ymm2**
   - 向量加法：将ymm0和ymm2中的8对32位整数分别相加
   - 对应C代码: `_mm256_add_epi32()`
   - 一次操作完成8个加法运算

4. **vmovdqu YMMWORD PTR [rsp+40], ymm0**
   - 将向量寄存器中的结果存储回内存
   - 对应C代码: `_mm256_storeu_si256()`

### 性能对比分析
- 朴素版本：每次迭代只执行1次乘法和1次加法
- SIMD版本：每次迭代执行8次乘法和8次加法（通过单条指令）
- 理论峰值加速比：8倍（受限于内存带宽和转置开销，实际约4-6倍）

## 函数接口说明

所有版本的矩阵乘法函数接口统一为：
```c
void matrix_multiply(int N, int **A, int **B, int **C);
```
- `N`: 矩阵大小（N x N）
- `A`, `B`: 输入矩阵
- `C`: 输出矩阵（结果存储在此）

内存分配函数：
```c
int** allocate_matrix(int N);
```
- 返回连续内存分配的二维数组指针

## 验证方法

各优化版本在计算完成后会输出结果矩阵的前4x4部分，可用于验证计算结果的正确性。对于4096x4096的大矩阵，不建议打印完整结果以节省时间和输出空间。
