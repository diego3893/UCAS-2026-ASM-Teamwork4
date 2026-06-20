# void block_kernel(int *A, int *B, int *C,
#                    int i, int jj, int kk, int k_end)
# System V AMD64 ABI:
#   rdi = A      (flat int *)
#   rsi = B      (flat int *)
#   rdx = C      (flat int *)
#   ecx = i      (row index)
#   r8d = jj     (start column of block)
#   r9d = kk     (start k of block)
#   [rsp+8] = k_end
#
# flat: offset(i, j) = i * N + j         (each int = 4 bytes)
# k-j order, B[k][j] is B + k*N*4 + j*4

    .equ  N, 4096
    .equ  BS, 64

    .text
    .globl block_kernel
    .type  block_kernel, @function

block_kernel:
    push   %rbp
    mov    %rsp, %rbp
    push   %rbx
    push   %r12
    push   %r13
    push   %r14

    mov    16(%rbp), %r13d             # r13d = k_end

    # A_row = A + i * N * 4
    movslq %ecx, %rax
    imulq  $(N * 4), %rax
    add    %rdi, %rax
    mov    %rax, %r12                  # r12 = &A[i][0]

    # C_row = C + i * N * 4
    movslq %ecx, %rax
    imulq  $(N * 4), %rax
    add    %rdx, %rax
    mov    %rax, %r14                  # r14 = &C[i][0]

    mov    %r8d,  %r10d                # r10d = jj
    mov    %r9d,  %r11d                # r11d = kk
    lea    BS(%r10d), %eax             # eax = j_end column

    # pre-scale jj → byte offset, j_end → byte offset
    shl    $2, %r10d                   # r10d = jj * 4
    shl    $2, %eax                    # eax = j_end * 4
    mov    %eax, %r9d                  # r9d = j_end_bytes (safe from B calc)

.Lk:
    cmp    %r13d, %r11d
    jge    .Ldone

    # aik = A_row[k]
    movslq %r11d, %rcx
    mov    (%r12, %rcx, 4), %r8d       # r8d = aik

    # B_row = B + k * N * 4
    movslq %r11d, %rax
    imulq  $(N * 4), %rax
    lea    (%rsi, %rax, 1), %rax       # rax = &B[k][0]

    mov    %r10d, %ebx                 # ebx = j (byte offset)

.Lj:
    cmp    %r9d, %ebx
    jge    .Lj_done

    mov    (%rax, %rbx, 1), %ecx       # ecx = B[k][j]
    imull  %r8d, %ecx
    addl   %ecx, (%r14, %rbx, 1)       # C[i][j] += product

    add    $4, %ebx
    jmp    .Lj
.Lj_done:

    incl   %r11d
    jmp    .Lk

.Ldone:
    pop    %r14
    pop    %r13
    pop    %r12
    pop    %rbx
    pop    %rbp
    ret
