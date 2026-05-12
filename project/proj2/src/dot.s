.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:
    # Prologue
    addi sp, sp, -8
    sw s0, 0(sp)
    sw s1, 4(sp)

    li t0, 1
    blt a2, t0, len_error
    blt a3, t0, std_error
    blt a4, t0, std_error

    # convert to byte stride
    mv t5, a3
    mv t6, a4
    slli t5, t5, 2  # t5 = t5 * 4
    slli t6, t6, 2  # t6 = t6 * 4

    j loop_start

len_error:
    li a0, 75
    j exit2

std_error:
    li a0, 76
    j exit2

loop_start:
    li t0, 0    # i
    li t4, 0    # sum
    mv s0, a0   # v0 ptr
    mv s1, a1   # v1 ptr


loop_continue:
    bge t0, a2, loop_end
    lw t1, 0(s0)  # v0[i]
    lw t2, 0(s1)  # v1[i]

    mul t3, t1, t2  # t3 = v0[i] * v1[i]
    add t4, t4, t3  # accumulate t3 to sum

    addi t0, t0, 1  # i++
    add s0, s0, t5  # v0 ptr + v0 stride
    add s1, s1, t6  # v1 ptr + v1 stride

    j loop_continue

loop_end:
    # Epilogue
    lw s1, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 8
    mv a0, t4
    ret
