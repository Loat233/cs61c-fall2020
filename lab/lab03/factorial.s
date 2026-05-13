.globl factorial

.data
n: .word 8

.text
main:
    la t0, n
    lw a0, 0(t0)
    jal ra, factorial

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

factorial:
    # Prologue
    addi sp, sp, -8
    sw s0, 0(sp)
    sw ra, 4(sp)

    mv s0, a0 # s0: get result from helper
    jal ra, helper
    mv a0, s0 # return value

    # Epilogue
    lw ra, 4(sp)
    lw s0, 0(sp)
    addi sp, sp, 8

    ret

helper:
    li t0, 1
    ble a0, t0, exit

    # Prologue
    addi sp, sp, -4
    sw ra, 0(sp)

    addi a0, a0, -1 # n = n-1
    mul s0, s0, a0 # s0 =* (n-1)
    jal ra, helper # call helper(n-1)

    # Epilogue
    lw ra, 0(sp)
    addi sp, sp, 4

exit:
    ret
