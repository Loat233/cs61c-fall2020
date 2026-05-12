.globl read_matrix

.text
# ==============================================================================
# FUNCTION: Allocates memory and reads in a binary file as a matrix of integers
#
# FILE FORMAT:
#   The first 8 bytes are two 4 byte ints representing the # of rows and columns
#   in the matrix. Every 4 bytes afterwards is an element of the matrix in
#   row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is a pointer to an integer, we will set it to the number of rows
#   a2 (int*)  is a pointer to an integer, we will set it to the number of columns
# Returns:
#   a0 (int*)  is the pointer to the matrix in memory
# Exceptions:
# - If malloc returns an error,
#   this function terminates the program with error code 88.
# - If you receive an fopen error or eof, 
#   this function terminates the program with error code 90.
# - If you receive an fread error or eof,
#   this function terminates the program with error code 91.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 92.
# ==============================================================================
read_matrix:
    # Prologue
    addi sp, sp, -28
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw s5, 20(sp)
    sw ra, 24(sp)

    mv s0, a0       # s0: filename ptr / file descriptor
    mv s1, a1       # s1: row_num ptr
    mv s2, a2       # s2: col_num ptr
    li s3, 4        # s3: byte size of num
                    # s4: byte size of mat
                    # s5: mat ptr

    # open file
    #============================================================
    # args:
    #   a1 = filepath
    #   a2 = permissions (0, 1, 2, 3, 4, 5 = r, w, a, r+, w+, a+)
    # return:
    #   a0 = file descriptor
    #============================================================
    mv a1, s0
    mv a2, x0
    jal ra, fopen

    li t0, -1
    beq a0, t0, fopen_error
    mv s0, a0       # s0: get file descriptor

    # read row_num
    #============================================================
    # args:
    #   a1 = file descriptor
    #   a2 = pointer to the buffer you want to write the read bytes to.
    #   a3 = Number of bytes to be read.
    # return:
    #   a0 = Number of bytes actually read.
    #============================================================
    mv a1, s0
    mv a2, s1
    mv a3, s3
    jal ra, fread

    bne a0, s3, fread_error

    # read col_num
    mv a1, s0
    mv a2, s2
    mv a3, s3
    jal ra, fread

    bne a0, s3, fread_error

    # cal byte size of mat
    lw t1, 0(s1)
    lw t2, 0(s2)
    mul t0, t1, t2
    slli s4, t0, 2  # s4: get size of mat

    # malloc for matrix
    #============================================================
    # args:
    #   a0 is the # of bytes to allocate heap memory for
    # return:
    #   a0 is the pointer to the allocated heap memory
    #============================================================
    mv a0, s4
    jal ra, malloc

    beq a0, x0, malloc_error
    mv s5, a0       # s5: get mat ptr

    # read matrix
    mv a1, s0
    mv a2, s5
    mv a3, s4
    jal ra, fread

    bne a0, s4, fread_error


    # close file
    #============================================================
    # args:
    #   a1 = file descriptor
    # return:
    #   a0 = 0 on success, and EOF (-1) otherwise.
    #============================================================
    mv a1, s0
    jal ra, fclose

    bne a0, x0, fclose_error
    mv a0, s5       # return mat ptr

    # Epilogue
    lw ra, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 28

    ret

malloc_error:
    li a0, 88
    j exit2

fopen_error:
    li a0, 90
    j exit2

fread_error:
    li a0, 91
    j exit2

fclose_error:
    li a0, 92
    j exit2
