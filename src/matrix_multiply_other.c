#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define N 4096

extern int simd_dot(int *A_row, int *BT_row, int len);
int** allocate_matrix(void);
void  free_matrix(int **m);
void  transpose_matrix(int **src, int **dst);

int main()
{
    const char *fileA = "tests/matrixA.bin";
    const char *fileB = "tests/matrixB.bin";
    const char *fileC = "tests/output_C.bin";

    FILE *fpA = fopen(fileA, "rb");
    FILE *fpB = fopen(fileB, "rb");
    if (!fpA || !fpB) {
        perror("Input file open failed — run generate first.");
        return -1;
    }

    int **A = allocate_matrix();
    int **B = allocate_matrix();
    int **BT = allocate_matrix();
    int **C = allocate_matrix();
    if (!A || !B || !BT || !C) { perror("Allocate failed."); return -1; }

    for (int i = 0; i < N; i++) {
        fread(A[i], sizeof(int), N, fpA);
        fread(B[i], sizeof(int), N, fpB);
    }
    fclose(fpA);
    fclose(fpB);

    printf("Start (OpenMP + asm simd, threads=%d) ...\n",
           omp_get_max_threads());

    /* transpose B once, then parallel i loop over A and BT */
    transpose_matrix(B, BT);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    int i, j;
    #pragma omp parallel for schedule(static) private(j)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            C[i][j] = simd_dot(A[i], BT[j], N);
        }
        if ((i + 1) % 256 == 0) {
            #pragma omp critical
            printf("  row %d/%d done (thread %d)\n",
                   i + 1, N, omp_get_thread_num());
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    printf("Time: %.2f s\n", t);

    FILE *fpC = fopen(fileC, "wb");
    if (!fpC) { perror("Output file open failed."); return -1; }
    for (int i = 0; i < N; i++)
        fwrite(C[i], sizeof(int), N, fpC);
    fclose(fpC);
    printf("Output successfully.\n");

    free_matrix(A); free_matrix(B); free_matrix(BT); free_matrix(C);
    return 0;
}

void transpose_matrix(int **src, int **dst)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            dst[j][i] = src[i][j];
}

int** allocate_matrix(void)
{
    int **m = (int **)malloc(N * sizeof(int *));
    if (!m) return NULL;
    int *block = (int *)calloc((size_t)N * N, sizeof(int));
    if (!block) { free(m); return NULL; }
    for (int i = 0; i < N; i++) m[i] = block + i * N;
    return m;
}

void free_matrix(int **m)
{
    if (m) free(m[0]);
    free(m);
}
