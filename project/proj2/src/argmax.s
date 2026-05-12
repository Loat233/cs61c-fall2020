.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:
    li t0, 1
    blt a1, t0, error

loop_start:
    li t0, 0    # i
    li t1, 0    # max idx
    lw t2, 0(a0)    # max val

loop_continue:
    bge t0, a1, loop_end
    lw t3, 0(a0)    # i val
    bge t2, t3, next
    mv t1, t0   # rp max idx
    mv t2, t3   # rp max val
    j next

next:
    addi t0, t0, 1
    addi a0, a0, 4
    j loop_continue

loop_end:
    # Epilogue

    mv a0, t1
    ret

error:
    li a0, 77
    j exit2
