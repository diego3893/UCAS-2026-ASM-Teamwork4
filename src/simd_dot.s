# int simd_dot(int *A_row, int *BT_row, int N)
# System V AMD64 ABI:  rdi = A_row,  rsi = BT_row,  edx = N
#
# Returns the dot product A_row[0..N-1] ⋅ BT_row[0..N-1].
# AVX2: 8 ints at a time with vpmulld + vpaddd.

    .text
    .globl simd_dot
    .type  simd_dot, @function

simd_dot:
    push   %rbp
    mov    %rsp, %rbp

    mov    %edx, %ecx                 # ecx = N
    vpxor  %xmm0, %xmm0, %xmm0       # sum = 0
    xor    %eax,  %eax                # k = 0

    # --- AVX2 8-way -------------------------------------------------
.Lavx:
    lea    8(%rax), %r8d
    cmp    %ecx, %r8d
    jg     .Lreduce

    vmovdqu  (%rdi, %rax, 4), %ymm1
    vmovdqu  (%rsi, %rax, 4), %ymm2
    vpmulld  %ymm2, %ymm1, %ymm3
    vpaddd   %ymm3, %ymm0, %ymm0

    add    $8, %eax
    jmp    .Lavx

    # --- horizontal reduction ---------------------------------------
.Lreduce:
    vextracti128  $1, %ymm0, %xmm1
    vpaddd        %xmm1, %xmm0, %xmm0
    vpshufd       $0x0E, %xmm0, %xmm1
    vpaddd        %xmm1, %xmm0, %xmm0
    vphaddd       %xmm0, %xmm0, %xmm0
    vmovd         %xmm0, %r8d

    # --- scalar tail -------------------------------------------------
.Ltail:
    cmp    %ecx, %eax
    jge    .Ldone
    mov    (%rdi, %rax, 4), %r9d
    imull  (%rsi, %rax, 4), %r9d
    addl   %r9d, %r8d
    incl   %eax
    jmp    .Ltail

.Ldone:
    mov    %r8d, %eax
    pop    %rbp
    ret
