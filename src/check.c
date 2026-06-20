#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 4096
#define SAMPLE_COUNT 200

int** allocate_matrix(int n);
void free_matrix(int **m, int n);
int  compute_dot(int n, int **A, int **B, int i, int j);

int main()
{
    const char *fileA   = "tests/matrixA.bin";
    const char *fileB   = "tests/matrixB.bin";
    const char *fileC   = "tests/output_C.bin";

    FILE *fpA = fopen(fileA, "rb");
    FILE *fpB = fopen(fileB, "rb");
    FILE *fpC = fopen(fileC, "rb");
    if (!fpA || !fpB || !fpC)
    {
        perror("File open failed — run generate first.");
        return -1;
    }

    int **A = allocate_matrix(N);
    int **B = allocate_matrix(N);
    int **C = allocate_matrix(N);
    if (!A || !B || !C)
    {
        perror("Allocate failed.");
        return -1;
    }

    /* read row by row */
    for (int i = 0; i < N; i++)
    {
        fread(A[i], sizeof(int), N, fpA);
        fread(B[i], sizeof(int), N, fpB);
        fread(C[i], sizeof(int), N, fpC);
    }
    fclose(fpA);
    fclose(fpB);
    fclose(fpC);

    /* ---------- top-left 8×8 ---------- */
    printf("Top-left 8x8 check:\n");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int expected = compute_dot(N, A, B, i, j);
            if (C[i][j] != expected)
            {
                printf("FAIL at (%d,%d): got %d, expected %d\n",
                       i, j, C[i][j], expected);
                goto cleanup;
            }
        }
    }
    printf("  OK\n");

    /* ---------- random samples ---------- */
    srand((unsigned int)time(NULL));
    for (int k = 0; k < SAMPLE_COUNT; k++)
    {
        int i = rand() % N;
        int j = rand() % N;
        int expected = compute_dot(N, A, B, i, j);
        if (C[i][j] != expected)
        {
            printf("FAIL at (%d,%d): got %d, expected %d\n",
                   i, j, C[i][j], expected);
            goto cleanup;
        }
    }
    printf("Random %d samples: OK\n", SAMPLE_COUNT);
    printf("Multiply correctly.\n");

cleanup:
    free_matrix(A, N);
    free_matrix(B, N);
    free_matrix(C, N);
    return 0;
}

int compute_dot(int n, int **A, int **B, int i, int j)
{
    long long sum = 0;               /* 64-bit to avoid overflow */
    for (int k = 0; k < n; k++)
        sum += (long long)A[i][k] * B[k][j];
    return (int)sum;
}

int** allocate_matrix(int n)
{
    int **m = (int **)malloc(n * sizeof(int *));
    if (!m) return NULL;
    int *block = (int *)malloc((size_t)n * n * sizeof(int));
    if (!block) { free(m); return NULL; }
    for (int i = 0; i < n; i++)
        m[i] = block + i * n;
    return m;
}

void free_matrix(int **m, int n)
{
    if (m) free(m[0]);
    free(m);
}
