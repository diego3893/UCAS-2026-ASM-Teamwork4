#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>

void matrix_multiply(int, int**, int**, int**);
int** allocate_matrix(int);
void transpose_matrix(int, int**, int**);

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
    int **matrixBT = allocate_matrix(N);
    int **matrixC = allocate_matrix(N);
    if(!matrixA || !matrixB || !matrixBT || !matrixC){
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


    printf("开始执行（SIMD AVX2优化，含转置）\n");
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

void transpose_matrix(int N, int **src, int **dst){
    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            dst[j][i] = src[i][j];
        }
    }
}

void matrix_multiply(int N, int **matrixA, int **matrixB, int **matrixC){
    int **matrixBT = allocate_matrix(N);
    if(!matrixBT) return;
    
    transpose_matrix(N, matrixB, matrixBT);

    int block_size = 8;
    int k;
    
    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            __m256i sum = _mm256_setzero_si256();
            k = 0;
            
            for(; k <= N - block_size; k += block_size){
                __m256i a = _mm256_loadu_si256((__m256i*)&matrixA[i][k]);
                __m256i b = _mm256_loadu_si256((__m256i*)&matrixBT[j][k]);
                __m256i mul = _mm256_mullo_epi32(a, b);
                sum = _mm256_add_epi32(sum, mul);
            }
            
            int result = 0;
            int temp[8];
            _mm256_storeu_si256((__m256i*)temp, sum);
            for(int t=0; t<8; ++t){
                result += temp[t];
            }
            
            for(; k < N; ++k){
                result += matrixA[i][k] * matrixBT[j][k];
            }
            
            matrixC[i][j] = result;
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
