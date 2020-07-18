op = {
    0: "add",
    1: "sub",
    2: "and",
    3: "or",
    4: "sll",
    5: "sra",
    6: "srl",
    7: "beq",
    8: "bne",
    9: "blt",
    10: "bgt",
    11: "ble",
    12: "bge",
    13: "jal",
    14: "lw",
    15: "sw",
    16: "reti",
    17: "in",
    18: "out",
    19: "halt"
}
reg = {
    0: "$zero",
    1: "$imm ",
    2: "$v0",
    3: "$a0",
    4: "$a1",
    5: "$t0",
    6: "$t1",
    7: "$t2",
    8: "$t3",
    9: "$s0",
    10: "$s1",
    11: "$s2",
    12: "$gp",
    13: "$sp",
    14: "$fp",
    15: "$ra"
}

memin = open("memin.txt", 'r')
trans = open("translatedMemin.txt", 'w')
linecount = -1
for line in memin:
    linecount += 1
    lop = op[int(line[0:2], 16)]
    lrd = reg[int(line[2:3], 16)]
    lrs = reg[int(line[3:4], 16)]
    lrt = reg[int(line[4:5], 16)]
    limm = int(line[5:8], 16)
    w = 12
    if limm & (1 << (w - 1)):
        limm = limm - (1 << w)
    limm = str(limm)
    translated =str(linecount) + ": "+line[:8]+" -> " + lop + " " + lrd + " " + lrs + " " + lrt + " " + limm + "\n"
    if int(line, 16) == 0:
        continue
    if int(line[:5], 16) == 0:
        translated = ".word " + str(linecount) + " " + limm + "\n"
    trans.write(translated)
memin.close()
trans.close()
