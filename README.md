# 国科大2026汇编语言第四次大作业

## 题目

- 使用 python 写一个矩阵乘法程序
- 使用 C 语言改写上述矩阵乘法程序
- 使用矩阵分块方法改写上述程序，考虑 cache 局部性机制优化该程序
- 使用 SIMD 向量指令优化上述矩阵乘法程序
- 使用其他优化方法优化该矩阵乘法程序
- 给出程序源码，执行结果，函数执行时间，加速倍数等的分析报告，并分析关键汇编指令或者宏
- `void matrixmultiply(int N, int **matrixA, int **matrixB, int **matrixC)`
  - 矩阵的大小为 4096×4096 (每个矩阵约 64MB)

## 项目简介

本项目实现了五种 4096×4096 整数矩阵乘法算法：Python 朴素、C 朴素（ikj）、分块（BS=64）、SIMD AVX2 向量化、以及 OpenMP + SIMD AVX2 多线程。所有 C 版本的核心计算函数均采用 x86-64 AT&T 语法手写汇编，编译运行通过根目录 Makefile 一站式管理。

## 统一工作流

沿用汇编大作业2的二进制流水线：`generate.c` 生成数据 → 计算程序读写 `.bin` 文件 → `check.c` 统一验证。

```
generate.c ──→ tests/matrixA.bin  (64MB)
           ──→ tests/matrixB.bin
                        │
         ┌──────────────┤
         ▼              ▼
  matrix_multiply.c   matrix_multiply.py
  (及各优化版本)       (ikj, 稀疏跳过)
         │              │
         └──────┬───────┘
                ▼
       tests/output_C.bin
                │
                ▼
            check.c  →  Multiply correctly ✓
```

## 目录结构

```
UCAS-2026-ASM-Teamwork4/
├── Makefile                         # 根目录，编译运行一站式
├── src/
│   ├── generate.c                   # 二进制数据生成器
│   ├── check.c                      # 验证程序（8×8 全检 + 200 随机抽样）
│   ├── matrix_multiply.py           # Python 朴素（ikj）
│   ├── matrix_multiply.c            # C 朴素（ikj，纯 C）
│   ├── matrix_multiply_block.c      # C 分块驱动 → block_kernel
│   ├── block_multiply.s             # 汇编核心：flat 数组分块乘加（k-j 顺序）
│   ├── matrix_multiply_simd.c       # C SIMD 驱动 → simd_dot
│   ├── simd_dot.s                   # 汇编核心：AVX2 向量点积（vpmulld+vpaddd）
│   ├── matrix_multiply_other.c      # C OpenMP 驱动 → simd_dot（多线程）
│   ├── unroll_dot.s                 # 汇编核心：8× 循环展开点积（备用）
│   └── data_generator.cpp           # (旧) 文本格式生成器
├── tests/                           # 测试数据目录（.bin 文件由 generate 生成）
├── docs/
│   └── report.tex                   # 实验报告
└── README.md
```

## 编译与运行

在 Linux 环境下，项目根目录执行：

```bash
make all           # 编译全部 + 生成测试数据
make matmul        # C 朴素 → 自动接 check
make matmul_block  # 分块（asm 核心）→ 自动接 check
make matmul_simd   # SIMD AVX2（asm 核心）→ 自动接 check
make matmul_other  # OpenMP + SIMD（asm 核心）→ 自动接 check
make run_py        # Python（Windows 宿主机运行）→ 自动接 check
make run_all       # 一键全流程
make clean         # 删除 bin/ 和 tests/*.bin
```

每个 `make matmul*` 目标编译后自动运行计算程序并接 `check` 验证。

## 性能测试结果

测试环境：Ubuntu 24.04 VM (VMware, 4 核, 6 GB) on Windows 11, Intel Core i9-14900HX。编译器 GCC 11.4.0，`-O2 -m64`。Python 版本在 Windows 宿主机运行。计时方式 `clock_gettime(CLOCK_MONOTONIC)`。

| 版本 | 时间(s) | 加速比 | 备注 |
|------|---------|--------|------|
| Python 朴素 | 3541.89 | — | ikj, 稀疏跳过 |
| C 朴素 | 29.50 | 1.00x | ikj, 稀疏跳过, 纯 C |
| C 分块 | 33.89 | 0.87x | BS=64, asm block_kernel, flat 数组 |
| C SIMD AVX2 | 15.77 | 1.87x | B 转置 + asm simd_dot (vpmulld+vpaddd) |
| C OpenMP + SIMD | 4.50 | 6.56x | 4 线程 + B 转置 + asm simd_dot |

所有版本均通过 `check.c` 正确性验证。

## 汇编核心说明

| 汇编文件 | 导出函数 | 功能 |
|----------|----------|------|
| `block_multiply.s` | `block_kernel(A,B,C,i,jj,kk,k_end)` | flat 数组分块乘加，k-j 顺序，手动计算 `i*N+j` 偏移 |
| `simd_dot.s` | `simd_dot(ARow,BTRow,N)` | AVX2 256-bit 向量点积，8 路并行 `vpmulld`+`vpaddd`，水平归约到标量 |
| `unroll_dot.s` | `unroll_dot(ARow,B,j,N)` | 8× 循环展开标量点积（备用，当前 OpenMP 版本使用 simd_dot） |

## 接口约定

- C 朴素版仍使用 `int **A` 双重指针；分块版使用 flat `int *` 数组 + 手动 `i*N+j` 寻址；SIMD/OpenMP 版使用双重指针但 B 转置后按行访问
- 所有版本固定 `N=4096`（`#define`），C 矩阵 `calloc` 零初始化
- 输出为 `tests/output_C.bin`，行主序二进制格式
