# int unroll_dot(int *A_row, int **B, int j, int len)
# System V AMD64 ABI:  rdi = A_row,  rsi = B,  edx = j,  ecx = len
#
# Returns sum_{k=0}^{len-1} A_row[k] * B[k][j]
# Inner loop manually unrolled 8x.

    .text
    .globl unroll_dot
    .type  unroll_dot, @function

unroll_dot:
    push   %rbp
    mov    %rsp, %rbp
    push   %rbx
    push   %r12
    push   %r13

    mov    %rdi, %r8                  # r8  = A_row
    mov    %rsi, %r9                  # r9  = B
    mov    %edx, %r10d                # r10d = j
    mov    %ecx, %r11d                # r11d = len

    xor    %r12d, %r12d               # sum = 0
    xor    %eax,  %eax                # k = 0

    mov    $-8, %ebx
    add    %r11d, %ebx                # ebx = len - 8
    js     .Lscalar                   # len < 8 → skip

.Lunroll:
    cmp    %ebx, %eax
    jg     .Lscalar

    # k+0
    mov    (%r8, %rax, 4), %r13d
    movslq %eax, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+1
    mov    4(%r8, %rax, 4), %r13d
    lea    1(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+2
    mov    8(%r8, %rax, 4), %r13d
    lea    2(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+3
    mov    12(%r8, %rax, 4), %r13d
    lea    3(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+4
    mov    16(%r8, %rax, 4), %r13d
    lea    4(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+5
    mov    20(%r8, %rax, 4), %r13d
    lea    5(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+6
    mov    24(%r8, %rax, 4), %r13d
    lea    6(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    # k+7
    mov    28(%r8, %rax, 4), %r13d
    lea    7(%rax), %ecx
    movslq %ecx, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d

    add    $8, %eax
    jmp    .Lunroll

.Lscalar:
    cmp    %r11d, %eax
    jge    .Ldone
    mov    (%r8, %rax, 4), %r13d
    movslq %eax, %rcx
    mov    (%r9, %rcx, 8), %rcx
    imull  (%rcx, %r10, 4), %r13d
    addl   %r13d, %r12d
    incl   %eax
    jmp    .Lscalar

.Ldone:
    mov    %r12d, %eax
    pop    %r13
    pop    %r12
    pop    %rbx
    pop    %rbp
    ret
