#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 4096
#define BS 64

extern void block_kernel(int *A, int *B, int *C,
                         int i, int jj, int kk, int k_end);
int* flat_alloc(void);
void flat_free(int *m);

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

    int *A = flat_alloc();
    int *B = flat_alloc();
    int *C = flat_alloc();
    if (!A || !B || !C) { perror("Allocate failed."); return -1; }

    fread(A, sizeof(int), (size_t)N * N, fpA);
    fread(B, sizeof(int), (size_t)N * N, fpB);
    fclose(fpA);
    fclose(fpB);

    printf("Start (block, BS=%d, asm kernel, flat) ...\n", BS);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int ii = 0; ii < N; ii += BS) {
        for (int jj = 0; jj < N; jj += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                int k_end = (kk + BS < N) ? kk + BS : N;
                for (int i = ii; i < ii + BS && i < N; i++) {
                    block_kernel(A, B, C, i, jj, kk, k_end);
                }
            }
        }
        if ((ii + BS) % 256 == 0)
            printf("  ii=%d done\n", ii + BS);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    printf("Time: %.2f s\n", t);

    FILE *fpC = fopen(fileC, "wb");
    if (!fpC) { perror("Output file open failed."); return -1; }
    fwrite(C, sizeof(int), (size_t)N * N, fpC);
    fclose(fpC);
    printf("Output successfully.\n");

    flat_free(A); flat_free(B); flat_free(C);
    return 0;
}

int* flat_alloc(void)
{
    return (int *)calloc((size_t)N * N, sizeof(int));
}

void flat_free(int *m)
{
    free(m);
}
