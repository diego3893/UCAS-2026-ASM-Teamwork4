#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define UNROLL_FACTOR 8

void matrix_multiply(int, int**, int**, int**);
int** allocate_matrix(int);

int main(){
    const char *filename = "../test/matrix_input.txt";
    FILE *file = fopen(filename, "r");
    if(!file){
        printf("错误：无法打开文件 %s\n", filename);
        return -1;
    }
    int N;
    if(fscanf(file, "%d", &N) != 1){
        printf("错误：无法读取矩阵规模 N\n");
        fclose(file);
        return -1;
    }
    printf("成功读取矩阵规模 N = %d\n", N);

    int **matrixA = allocate_matrix(N);
    int **matrixB = allocate_matrix(N);
    int **matrixC = allocate_matrix(N);
    if(!matrixA || !matrixB || !matrixC){
        printf("错误：内存分配失败！\n");
        fclose(file);
        return -1;
    }

    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            if(fscanf(file, "%d", &matrixA[i][j]) != 1){
                printf("错误：读取矩阵 A 数据时出错 (行:%d, 列:%d)\n", i, j);
                fclose(file);
                return -1;
            }
        }
    }
    printf("矩阵 A 读入完毕。\n");

    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            if(fscanf(file, "%d", &matrixB[i][j]) != 1){
                printf("错误：读取矩阵 B 数据时出错 (行:%d, 列:%d)\n", i, j);
                fclose(file);
                return -1;
            }
        }
    }
    printf("矩阵 B 读入完毕。\n");
    fclose(file);


    printf("开始执行（OpenMP并行 + 循环展开，线程数 = %d，展开因子 = %d）\n", omp_get_max_threads(), UNROLL_FACTOR);
    clock_t start = clock();
    
    matrix_multiply(N, matrixA, matrixB, matrixC);
    
    clock_t end = clock();
    double cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("计算完成！\n");
    printf("-> 函数执行时间: %f 秒\n", cpu_time_used);

    printf("\n验证结果（前4x4）：\n");
    for(int i=0; i<4 && i<N; ++i){
        for(int j=0; j<4 && j<N; ++j){
            printf("%d ", matrixC[i][j]);
        }
        printf("\n");
    }

    return 0;
}

void matrix_multiply(int N, int **matrixA, int **matrixB, int **matrixC){
    int i, j, k;
    
    #pragma omp parallel for schedule(static) private(j, k)
    for(i=0; i<N; ++i){
        for(j=0; j<N; ++j){
            int sum = 0;
            k = 0;
            
            for(; k <= N - UNROLL_FACTOR; k += UNROLL_FACTOR){
                sum += matrixA[i][k] * matrixB[k][j];
                sum += matrixA[i][k+1] * matrixB[k+1][j];
                sum += matrixA[i][k+2] * matrixB[k+2][j];
                sum += matrixA[i][k+3] * matrixB[k+3][j];
                sum += matrixA[i][k+4] * matrixB[k+4][j];
                sum += matrixA[i][k+5] * matrixB[k+5][j];
                sum += matrixA[i][k+6] * matrixB[k+6][j];
                sum += matrixA[i][k+7] * matrixB[k+7][j];
            }
            
            for(; k < N; ++k){
                sum += matrixA[i][k] * matrixB[k][j];
            }
            
            matrixC[i][j] = sum;
        }
    }
}

int** allocate_matrix(int N){
    int** matrix = (int**)malloc(N*sizeof(int*));
    if(!matrix){
        return NULL;
    }
    int* block = (int*)calloc(N*N, sizeof(int));
    if(!block){
        return NULL;
    }
    for(int i=0; i<N; ++i){
        matrix[i] = block+i*N;
    }
    return matrix;
}
