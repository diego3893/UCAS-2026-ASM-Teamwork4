CC       = gcc
CXX      = g++
CFLAGS   = -O2 -m64 -I $(SRC_DIR)
LDFLAGS  =

SRC_DIR  = src
BIN_DIR  = bin
TEST_DIR = tests

# ── helper targets ──────────────────────────────────────────
.PHONY: all clean generate check \
        matmul matmul_block matmul_simd matmul_other \
        run_all run_py

all: generate check matmul matmul_block matmul_simd matmul_other

$(BIN_DIR):
	mkdir -p $@

# ── generate ────────────────────────────────────────────────
$(BIN_DIR)/generate: $(SRC_DIR)/generate.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

generate: $(BIN_DIR)/generate
	$<

# ── check ───────────────────────────────────────────────────
$(BIN_DIR)/check: $(SRC_DIR)/check.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

check: $(BIN_DIR)/check
	$<

# ── C naive ─────────────────────────────────────────────────
$(BIN_DIR)/matmul: $(SRC_DIR)/matrix_multiply.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

matmul: $(BIN_DIR)/matmul
	$<
	$(MAKE) check

# ── C block (asm core) ──────────────────────────────────────
$(BIN_DIR)/matmul_block: $(SRC_DIR)/matrix_multiply_block.c \
                         $(SRC_DIR)/block_multiply.s | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

matmul_block: $(BIN_DIR)/matmul_block
	$<
	$(MAKE) check

# ── SIMD AVX2 (asm dot) ─────────────────────────────────────
$(BIN_DIR)/matmul_simd: $(SRC_DIR)/matrix_multiply_simd.c \
                        $(SRC_DIR)/simd_dot.s | $(BIN_DIR)
	$(CC) $(CFLAGS) -mavx2 -o $@ $^

matmul_simd: $(BIN_DIR)/matmul_simd
	$<
	$(MAKE) check

# ── OpenMP + asm simd ────────────────────────────────────
$(BIN_DIR)/matmul_other: $(SRC_DIR)/matrix_multiply_other.c \
                         $(SRC_DIR)/simd_dot.s | $(BIN_DIR)
	$(CC) $(CFLAGS) -fopenmp -mavx2 -o $@ $^

matmul_other: $(BIN_DIR)/matmul_other
	$<
	$(MAKE) check

# ── Python ──────────────────────────────────────────────────
run_py:
	cd $(SRC_DIR) && python3 matrix_multiply.py
	$(MAKE) check

# ── all in sequence ─────────────────────────────────────────
run_all: generate
	cd $(SRC_DIR) && python3 matrix_multiply.py
	$(MAKE) check
	$(BIN_DIR)/matmul
	$(MAKE) check
	$(BIN_DIR)/matmul_block
	$(MAKE) check
	$(BIN_DIR)/matmul_simd
	$(MAKE) check
	$(BIN_DIR)/matmul_other
	$(MAKE) check

# ── cleanup ─────────────────────────────────────────────────
clean:
	rm -rf $(BIN_DIR)
	rm -f $(TEST_DIR)/matrixA.bin $(TEST_DIR)/matrixB.bin $(TEST_DIR)/output_C.bin
