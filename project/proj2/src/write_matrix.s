.globl write_matrix

.text
# ==============================================================================
# FUNCTION: Writes a matrix of integers into a binary file
# FILE FORMAT:
#   The first 8 bytes of the file will be two 4 byte ints representing the
#   numbers of rows and columns respectively. Every 4 bytes thereafter is an
#   element of the matrix in row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is the pointer to the start of the matrix in memory
#   a2 (int)   is the number of rows in the matrix
#   a3 (int)   is the number of columns in the matrix
# Returns:
#   None
# Exceptions:
# - If you receive an fopen error or eof,
#   this function terminates the program with error code 93.
# - If you receive an fwrite error or eof,
#   this function terminates the program with error code 94.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 95.
# ==============================================================================
write_matrix:
    # Prologue
    addi sp, sp, -24
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw ra, 20(sp)

    mv s0, a0       # s0: filename / file descriptor
    mv s1, a1       # s1: mat ptr
    mv s2, a2       # s2: row_num
    mv s3, a3       # s3: col_num
    mul s4, s2, s3  # s4: el_num in mat(rows * cols)


    # open file
    #================================================================
    # args:
    #   a1 = filepath
    #   a2 = permissions (0, 1, 2, 3, 4, 5 = r, w, a, r+, w+, a+)
    # return:
    #   a0 = file descriptor
    #================================================================
    mv a1, a0
    li a2, 1
    jal ra, fopen

    li t0, -1
    beq a0, t0, fopen_error
    mv s0, a0       # s0: get file descriptor

    # save col_num and row_num into sp
    addi sp, sp, -8
    sw s2, 0(sp)
    sw s3, 4(sp)

    # write col_num and row_num into file
    #================================================================
    # args:
    #   a1 = file descriptor
    #   a2 = Buffer to read from
    #   a3 = Number of items to read from the buffer.
    #   a4 = Size of each item in the buffer.
    # return:
    #   a0 = Number of elements writen. If this is less than a3,
    #    it is either an error or EOF. You will also need to still flush the fd.
    #================================================================
    mv a1, s0
    mv a2, sp
    li a3, 2
    li a4, 4
    jal ra, fwrite

    addi sp, sp, 8

    li t0, 2
    bne a0, t0, fwrite_error

    # write mat
    mv a1, s0
    mv a2, s1
    mv a3, s4
    li a4, 4
    jal ra, fwrite

    bne a0, s4, fwrite_error

    # close file
    #================================================================
    # args:
    #   a1 = file descriptor
    # return:
    #   a0 = 0 on success, and EOF (-1) otherwise.
    #================================================================
    mv a1, s0
    jal ra, fclose

    bne a0, x0, fclose_error

    # Epilogue
    lw ra, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 24

    ret


fopen_error:
    li a0, 93
    j exit2

fwrite_error:
    li a0, 94
    j exit2

fclose_error:
    li a0, 95
    j exit2