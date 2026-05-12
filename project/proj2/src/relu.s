.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Prologue

    li t0, 1
    bge a1, t0, loop_start
    li a0, 78
    j exit2

loop_start:
    li t0, 0

loop_continue:
    bge t0, a1, loop_end
    lw t1, 0(a0)

    bge t1, x0, skip_zero
    li t1, 0

skip_zero:
    sw t1, 0(a0)
    addi t0, t0, 1
    addi a0, a0, 4
    j loop_continue

loop_end:
    # Epilogue

	ret