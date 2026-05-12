.globl matmul

.text
# =======================================================
# FUNCTION: Matrix Multiplication of 2 integer matrices
# 	d = matmul(m0, m1)
# Arguments:
# 	a0 (int*)  is the pointer to the start of m0 
#	a1 (int)   is the # of rows (height) of m0
#	a2 (int)   is the # of columns (width) of m0
#	a3 (int*)  is the pointer to the start of m1
# 	a4 (int)   is the # of rows (height) of m1
#	a5 (int)   is the # of columns (width) of m1
#	a6 (int*)  is the pointer to the the start of d
# Returns:
#	None (void), sets d = matmul(m0, m1)
# Exceptions:
#   Make sure to check in top to bottom order!
#   - If the dimensions of m0 do not make sense,
#     this function terminates the program with exit code 72.
#   - If the dimensions of m1 do not make sense,
#     this function terminates the program with exit code 73.
#   - If the dimensions of m0 and m1 don't match,
#     this function terminates the program with exit code 74.
# =======================================================
matmul:
    # m0 error checks
    blez a1, m0_error
    blez a2, m0_error

    # m1 error checks
    blez a4, m1_error
    blez a5, m1_error

    # m0 != m1 checks
    bne a2, a4, match_error

    # Prologue
    addi sp, sp, -40
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw s5, 20(sp)
    sw s6, 24(sp)
    sw s7, 28(sp)
    sw s8, 32(sp)
    sw ra, 36(sp)

outer_loop_start:
    li t0, 0    # i, i(i-th row of m0)
    mv s1, a3   # m1 base
    slli s2, a2, 2  # byte stride of row ptr = col-num * 4(int size)
    mv s4, a0   # m0 row ptr
    mv s5, a5   # max j
    mv s6, a1   # max i
    mv s7, a6   # d ptr
    mv s8, a2   # len of d = m0 width


outer_loop_continue:
    bge t0, s6, outer_loop_end

# byte stride of column ptr = 1
inner_loop_start:
    mv s3, s1   # m1 column ptr = m1 base
    li t1, 0    # j, j-th col of m1

inner_loop_continue:
    bge t1, s5, inner_loop_end

    # set up arg for dot
    mv a0, s4   # ptr to the start of m0 row
    mv a1, s3   # ptr to the start of m1 col
    mv a2, s8   # len of vec
    li a3, 1    # stride in m0 row
    mv a4, s5   # stride in m1 col

    # Prologue
    addi sp, sp, -8
    sw t0, 0(sp)
    sw t1, 4(sp)

    # cal m0 row * m1 col
    jal ra, dot

    # Epilogue
    lw t1, 4(sp)
    lw t0, 0(sp)
    addi sp, sp, 8

    # save dot result(a0) to d
    sw a0, 0(s7)

    addi s7, s7, 4  # d row ptr += 4
    addi t1, t1, 1  # j++
    addi s3, s3, 4  # m1 col ptr += stride
    j inner_loop_continue

inner_loop_end:
    addi t0, t0, 1  # i++
    add s4, s4, s2 # m0 row ptr += stride
    j outer_loop_continue

outer_loop_end:
    # Epilogue
    lw ra, 36(sp)
    lw s8, 32(sp)
    lw s7, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 40
    ret

m0_error:
    li a0, 72
    j exit2

m1_error:
    li a0, 73
    j exit2

match_error:
    li a0, 74
    j exit2
