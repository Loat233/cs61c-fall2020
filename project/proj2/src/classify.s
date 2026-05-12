.globl classify

.text
classify:
    # =====================================
    # COMMAND LINE ARGUMENTS
    # =====================================
    # Args:
    #   a0 (int)    argc
    #   a1 (char**) argv
    #   a2 (int)    print_classification, if this is zero, 
    #               you should print the classification. Otherwise,
    #               this function should not print ANYTHING.
    # Returns:
    #   a0 (int)    Classification
    # Exceptions:
    # - If there are an incorrect number of command line args,
    #   this function terminates the program with exit code 89.
    # - If malloc fails, this function terminats the program with exit code 88.
    #
    # Usage:
    #   main.s <M0_PATH> <M1_PATH> <INPUT_PATH> <OUTPUT_PATH>

    # check arg_num
    mv t0 a0 # t0: arg_num

    li t1, 5
    bne t0, t1, arg_num_error

    # Prologue
    addi sp, sp, -52
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw s5, 20(sp)
    sw s6, 24(sp)
    sw s7, 28(sp)
    sw s8, 32(sp)
    sw s9, 36(sp)
    sw s10, 40(sp)
    sw s11, 44(sp)
    sw ra, 48(sp)

    # Load filename
    mv s4, a1 # save argv
    lw s0 4(s4) # s0: m0_filename / m0_ptr
    lw s1 8(s4) # s1: m1_filename / m1_ptr
    lw s2 12(s4) # s2: input_filename / input_ptr
    lw s3 16(s4) # s3: output_filename
    # s4: mul0_row / argv for load filename
    # s5: mul0_col
    # s6: mul0_ptr
    # s7: mul0_size
    # s8: mul1_ptr
    # s9: mul1_size
    # s10: mul1_row / m1_row / ret_value
    mv s11, a2 # s11: classification

    # Load pretrained m0
    addi sp, sp, -8
    addi a1, sp, 0
    addi a2, sp, 4
    mv a0, s0
    jal ra read_matrix

    mv s0, a0 # s0: get m0_ptr

    # Load pretrained m1
    addi sp, sp, -8
    addi a1, sp, 0
    addi a2, sp, 4
    mv a0, s1
    jal ra read_matrix

    mv s1, a0 # s1: get m1_ptr

    # Load input matrix
    addi sp, sp, -8
    addi a1, sp, 0
    addi a2, sp, 4
    mv a0, s2
    jal ra, read_matrix

    mv s2, a0 # s2: get input_ptr

    # sp:
    # 0(sp): input_row
    # 4(sp): input_col
    # 8(sp): m1_row
    # 12(sp): m1_col
    # 16(sp): m0_row
    # 20(sp): m0_col

    # 0. malloc for mul0(m0 * input)
    # get m0 row and col
    lw s4, 16(sp) # s4: get mul0_row(m0_row)
    lw s5, 4(sp) # s5: get mul0_col(input_col)

    # cal byte_size of mul0
    mul s7, s4, s5 # s7: get mul0_size
    slli t0, s7, 2 # t0: m0_row * input_col * 4

    mv a0, t0
    jal ra, malloc

    beq a0, x0, malloc_error
    mv s6, a0 # s6: get mul0_ptr

    # 1. LINEAR LAYER:    mul0(m0 * input)
    mv a0, s0
    mv a1, s4
    lw a2, 20(sp)
    mv a3, s2
    lw a4, 0(sp)
    mv a5, s5
    mv a6, s6

    jal ra, matmul # s6: ptr of mul0

    # 2. NONLINEAR LAYER: ReLU(m0 * input)
    mv a0, s6
    mv a1, s7
    jal ra, relu # s6: ptr of ReLU(mul0)

    # 3. malloc for mul1
    lw s10, 8(sp) # s: get mul1_row / m1_row

    # cal byte_size of mul1
    mul s9, s10, s5 # s9: get mul1_size
    slli t0, s9, 2 # t0: mul1_size * 4

    mv a0, t0
    jal ra, malloc

    beq a0, x0, malloc_error
    mv s8, a0 # s8: get mul1_ptr

    # 4. LINEAR LAYER:    m1 * ReLU(mul0)
    mv a0, s1 # m1_ptr
    mv a1, s10 # m1_row
    lw a2, 12(sp) # m1_col
    mv a3, s6 # relu(mul0)_ptr
    mv a4, s4 # mul0_row
    mv a5, s5 # mul0_col
    mv a6, s8 # mul1_ptr

    jal ra, matmul # s8: m1 * ReLU(mul0)

    addi sp, sp, 24 # move back sp

    # Write output matrix
    mv a0, s3 # output_filename
    mv a1, s8 # mul1_ptr
    mv a2, s10 # mul1_row / m1_row
    mv a3, s5 # mul1_col / mul0_col / input_col
    jal ra, write_matrix

    # Call argmax
    mv a0, s8 # mul1_ptr
    mv a1, s9 # mul1_size
    jal ra, argmax
    mv s10, a0 # s10: max_num

    # Print classification
    bne s11, x0, skip_print

    mv a1, s10 # max_num
    jal ra, print_int

    # Print newline afterwards for clarity
    li a1, '\n'
    jal ra, print_char

skip_print:
    # free m0, m1, input, mul0 and mul1
    mv a0, s0 # m0_ptr
    jal ra, free

    mv a0, s1 # m1_ptr
    jal ra, free

    mv a0, s2 # input_ptr
    jal ra, free

    mv a0, s6 # mul0_ptr
    jal ra, free

    mv a0, s8 # mul1_ptr
    jal ra, free

    mv a0, s10 # return classification

    # Epilogue
    lw ra, 48(sp)
    lw s11, 44(sp)
    lw s10, 40(sp)
    lw s9, 36(sp)
    lw s8, 32(sp)
    lw s7, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 52

    ret

arg_num_error:
    li a0, 89
    j exit2

malloc_error:
    li a0, 88
    j exit2
