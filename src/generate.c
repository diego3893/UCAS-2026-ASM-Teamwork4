#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 4096

int main()
{
    const char *fileA = "tests/matrixA.bin";
    const char *fileB = "tests/matrixB.bin";
    FILE *fpA = fopen(fileA, "wb");
    FILE *fpB = fopen(fileB, "wb");
    if (!fpA || !fpB)
    {
        perror("Create failed.");
        return -1;
    }

    int *row = (int *)malloc(N * sizeof(int));
    if (!row)
    {
        perror("Allocate failed.");
        fclose(fpA);
        fclose(fpB);
        return -1;
    }

    /* matrix A */
    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            row[j] = rand() % 10;
        fwrite(row, sizeof(int), N, fpA);
    }

    /* matrix B — different seed so A != B */
    srand((unsigned int)time(NULL) ^ 0x5EED);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            row[j] = rand() % 10;
        fwrite(row, sizeof(int), N, fpB);
    }

    free(row);
    fclose(fpA);
    fclose(fpB);
    printf("Generate successfully.\n");
    return 0;
}
