rom_data = [0] * 4096

def add_inst(opcode, funct3, bit25, bit30, pcSel, regWEn, immSel, brUn,
             aSel, bSel, aluSel,
             memRw, wbSel, csrSel, csrWEn,
             brSel):
    address = (bit30 << 11) | (bit25 << 10) | (funct3 << 7) | opcode
    # bit 0: pcSel(pc + 4, alu(pc + imm))
    # bit 1: regWEn(nw, w)
    # bit 4-2: immSel(I, S, B, J, U)
    # bit 5: brUn(sign, unsign)
    # bit 6: aSel(dataA, pc)
    # bit 7: bSel(dataB, imm)
    # bit 11-8: aluSel(aluMode)
    # bit 12: memRw(read, write)
    # bit 14-13: wbSel(Load, alu, pc + 4)
    # bit 15: csrSel(dataA, imm)
    # bit 16: csrWEn(nw, w)
    # bit 17: brSel(nbr, br)

    data = 0
    data |= (pcSel & 0b1) << 0
    data |= (regWEn & 0b1) << 1
    data |= (immSel & 0b111) << 2
    data |= (brUn & 0b1) << 5
    data |= (aSel & 0b1) << 6
    data |= (bSel & 0b1) << 7
    data |= (aluSel & 0b1111) << 8
    data |= (memRw & 0b1) << 12
    data |= (wbSel & 0b11) << 13
    data |= (csrSel & 0b1) << 15
    data |= (csrWEn & 0b1) << 16
    data |= (brSel & 0b1) << 17

    rom_data[address] = data

# ========== R-type ==========
# instR: add(bit30=0, bit25=0)
add_inst(0x33, 0x00, 0,0,
    0, 1, 0, 0,
    0, 0, 0, 0, 1,
    0, 0, 0)

# instR: mul(bit30=0, bit25=1)
add_inst(0x33, 0x0, 1,0,
    0, 1, 0, 0,
    0, 0, 10, 0, 1,
    0, 0, 0)

# instR: sub(bit30=1, bit25=0)
add_inst(0x33, 0x0, 0,1,
         0, 1, 0, 0,
         0, 0, 12, 0, 1,
         0, 0, 0)

# instR: sll(bit30=0, bit25=0)
add_inst(0x33, 0x1, 0,0,
         0, 1, 0, 0,
         0, 0, 6, 0, 1,
         0, 0, 0)

# instR: mulh(bit30=0, bit25=1)
add_inst(0x33, 0x1, 1,0,
         0, 1, 0, 0,
         0, 0, 14, 0, 1,
         0, 0, 0)

# instR: mulhu(bit30=0, bit25=1)
add_inst(0x33, 0x3, 1,0,
         0, 1, 0, 0,
         0, 0, 11, 0, 1,
         0, 0, 0)

# instR: slt(bit30=0, bit25=0)
add_inst(0x33, 0x2, 0,0,
         0, 1, 0, 0,
         0, 0, 7, 0, 1,
         0, 0, 0)

# instR: xor(bit30=0, bit25=0)
add_inst(0x33, 0x4, 0,0,
         0, 1, 0, 0,
         0, 0, 3, 0, 1,
         0, 0, 0)

# instR: srl(bit30=0, bit25=0)
add_inst(0x33, 0x5, 0,0,
         0, 1, 0, 0,
         0, 0, 4, 0, 1,
         0, 0, 0)

# instR: sra(bit30=1, bit25=0)
add_inst(0x33, 0x5, 0,1,
         0, 1, 0, 0,
         0, 0, 5, 0, 1,
         0, 0, 0)

# instR: or(bit30=0, bit25=0)
add_inst(0x33, 0x6, 0,0,
         0, 1, 0, 0,
         0, 0, 2, 0, 1,
         0, 0, 0)

# instR: and(bit30=0, bit25=0)
add_inst(0x33, 0x7, 0,0,
         0, 1, 0, 0,
         0, 0, 1, 0, 1,
         0, 0, 0)



# ========== I-type ==========
# instI: lb(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x03, 0x0, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 0, 0, 0,
                 0, 0, 0)

# instI: lh(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x03, 0x1, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 0, 0, 0,
                 0, 0, 0)

# instI: lw(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x03, 0x2, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 0, 0, 0,
                 0, 0, 0)

# instI: addi(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x13, 0x0, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 0, 0, 1,
                 0, 0, 0)

# instI: slli(bit30=0, bit25=0)
add_inst(0x13, 0x1, 0, 0,
         0, 1, 0, 0,
         0, 1, 6, 0, 1,
         0, 0, 0)

# instI: slti(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x13, 0x2, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 7, 0, 1,
                 0, 0, 0)

# instI: xori(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x13, 0x4, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 3, 0, 1,
                 0, 0, 0)

# instI: srli(bit30=0, bit25=0)
add_inst(0x13, 0x5, 0, 0,
         0, 1, 0, 0,
         0, 1, 4, 0, 1,
         0, 0, 0)

# instI: srai(bit30=1, bit25=0)
add_inst(0x13, 0x5, 0, 1,
         0, 1, 0, 0,
         0, 1, 5, 0, 1,
         0, 0, 0)

# instI: ori(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x13, 0x6, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 2, 0, 1,
                 0, 0, 0)

# instI: andi(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x13, 0x7, bit25, bit30,
                 0, 1, 0, 0,
                 0, 1, 1, 0, 1,
                 0, 0, 0)

# instI: jalr/jarl(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x67, 0x0, bit25, bit30,
                 1, 1, 0, 0,
                 0, 1, 0, 0, 2,
                 0, 0, 0)



# ========== J-type ==========
# instJ: jal(bit30=*, bit25=*)
for funct3 in (0, 1, 2, 3, 4, 5, 6, 7):
    for bit25 in (0,1):
        for bit30 in (0,1):
            add_inst(0x6f, funct3, bit25, bit30,
                     1, 1, 3, 0,
                     1, 1, 0, 0, 2,
                     0, 0, 0)



# ========== S-type ==========
# instS: sb(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x23, 0x0, bit25, bit30,
                 0, 0, 1, 0,
                 0, 1, 0, 1, 1,
                 0, 0, 0)

# instS: sh(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x23, 0x1, bit25, bit30,
                 0, 0, 1, 0,
                 0, 1, 0, 1, 1,
                 0, 0, 0)

# instS: sw(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x23, 0x2, bit25, bit30,
                 0, 0, 1, 0,
                 0, 1, 0, 1, 1,
                 0, 0, 0)



# ========== U-type ==========
# instU auipc(bit30=*, bit25=*)
for funct3 in (0, 1, 2, 3, 4, 5, 6, 7):
    for bit25 in (0,1):
        for bit30 in (0,1):
            add_inst(0x17, funct3, bit25, bit30,
                     0, 1, 4, 0,
                     1, 1, 0, 0, 1,
                     0, 0, 0)

# instU lui(bit30=*, bit25=*)
for funct3 in (0, 1, 2, 3, 4, 5, 6, 7):
    for bit25 in (0,1):
        for bit30 in (0,1):
            add_inst(0x37, funct3, bit25, bit30,
                     0, 1, 4, 0,
                     0, 1, 13, 0, 1,
                     0, 0, 0)



# ========== B-type ==========
# instB: beq(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x0, bit25, bit30,
                 0, 0, 2, 0,
                 1, 1, 0, 0, 1,
                 0, 0, 1)

# instB: bne(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x1, bit25, bit30,
                 0, 0, 2, 0,
                 1, 1, 0, 0, 1,
                 0, 0, 1)

# instB: blt(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x4, bit25, bit30,
                 0, 0, 2, 0,
                 1, 1, 0, 0, 1,
                 0, 0, 1)

# instB: bge(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x5, bit25, bit30,
                 0, 0, 2, 0,
                 1, 1, 0, 0, 1,
                 0, 0, 1)

# instB: bltu(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x6, bit25, bit30,
                 0, 0, 2, 1,
                 1, 1, 0, 0, 1,
                 0, 0, 1)

# instB: bgeu(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x63, 0x7, bit25, bit30,
                 0, 0, 2, 1,
                 1, 1, 0, 0, 1,
                 0, 0, 1)



# ========== CSR ==========
# instI: csrw(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x73, 0x1, bit25, bit30,
                 0, 0, 0, 0,
                 0, 0, 0, 0, 0,
                 0, 1, 0)

# instI: csrw(bit30=*, bit25=*)
for bit25 in (0,1):
    for bit30 in (0,1):
        add_inst(0x73, 0x5, bit25, bit30,
                 0, 0, 0, 0,
                 0, 0, 0, 0, 0,
                 1, 1, 0)




# 生成 .hex 文件
with open("control_logic.hex", "w") as f:
    f.write("v2.0 raw\n")
    for val in rom_data:
        f.write(f"{val:04x}\n")

print("control_logic.hex 生成成功")