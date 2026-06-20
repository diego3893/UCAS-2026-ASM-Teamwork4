import struct
import time
import sys

N = 4096

def main():
    fileA = "../tests/matrixA.bin"
    fileB = "../tests/matrixB.bin"
    fileC = "../tests/output_C.bin"

    # ---------- read ----------
    A = []
    with open(fileA, "rb") as f:
        for _ in range(N):
            row = list(struct.unpack(f"{N}i", f.read(N * 4)))
            A.append(row)

    B = []
    with open(fileB, "rb") as f:
        for _ in range(N):
            row = list(struct.unpack(f"{N}i", f.read(N * 4)))
            B.append(row)

    C = [[0] * N for _ in range(N)]

    print("Start computing ...")

    start = time.time()

    for i in range(N):
        for k in range(N):
            aik = A[i][k]
            if aik == 0:
                continue
            rowB = B[k]
            rowC = C[i]
            for j in range(N):
                rowC[j] += aik * rowB[j]

        if (i + 1) % 256 == 0:
            print(f"  row {i + 1}/{N} done")

    elapsed = time.time() - start
    print(f"Time: {elapsed:.2f} s")

    # ---------- write ----------
    with open(fileC, "wb") as f:
        for row in C:
            f.write(struct.pack(f"{N}i", *row))

    print("Output successfully.")

if __name__ == "__main__":
    main()
