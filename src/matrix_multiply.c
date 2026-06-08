#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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


    printf("开始执行\n");
    clock_t start = clock();
    
    matrix_multiply(N, matrixA, matrixB, matrixC);
    
    clock_t end = clock();
    double cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("计算完成！\n");
    printf("-> 函数执行时间: %f 秒\n", cpu_time_used);

    printf("\n计算结果矩阵 C :\n");
    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            printf("%d ", matrixC[i][j]);
        }
        printf("\n");
    }

    return 0;
}

void matrix_multiply(int N, int **matrixA, int **matrixB, int **matrixC){
    for(int i=0; i<N; ++i){
        for(int j=0; j<N; ++j){
            for(int k=0; k<N; ++k){
                matrixC[i][j] += matrixA[i][k]*matrixB[k][j];
            }
        }
    }
    return;
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