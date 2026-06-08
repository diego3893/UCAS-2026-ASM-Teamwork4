import time
import random

def matrixmultiply(N, matrixA, matrixB, matrixC):
    for i in range(N):
        for j in range(N):
            for k in range(N):
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j]

def main():
    N = 128  # 4096跑py的三重循环慢的要死，先用小数据
    
    print(f"正在生成大小为 {N}x{N} 的矩阵...")
    matrixA = [[random.randint(0, 10) for _ in range(N)] for _ in range(N)]
    matrixB = [[random.randint(0, 10) for _ in range(N)] for _ in range(N)]
    matrixC = [[0 for _ in range(N)] for _ in range(N)]

    print("开始执行")
    start_time = time.time()
    
    matrixmultiply(N, matrixA, matrixB, matrixC)
    
    end_time = time.time()
    execution_time = end_time - start_time
    
    print("计算完成！")
    print(f"-> 函数执行时间: {execution_time:.4f} 秒")

if __name__ == "__main__":
    main()