#!/usr/bin/python

import sys

# in: string instruction line in assembly
# out: string instruction line in binary
def getBinaryOf(instr) :
    regNumLen = 6 # there are 2^6 = 64 GPRs
    registerLen = 64 # each GPR has 64 bits

    # list and dictionary provide map of instructions to op/type code
    dataTransferInstrs = ["liint", "fload", "intstore"]
    computationalInsrs = ["add", "mul", "cmp"]
    constrolInstrs = ["jump"]
    opCodeDict = {
        "liint" : "00111", # same opcode as intload in ISA doc
        "fload" : "00100",
        "add" : "00000",
        "mul" : "00010",
        "intstore" : "01000",
        "jump" : "00000", # technically no opcode for control flow instrs
        "cmp" : "01010"
    }

    # split up instrList for easier access
    instrList = instr.split(' ')
    instrName = instrList[0]

    # get type code
    if instrName in dataTransferInstrs:
        typeCode = "001"
    elif instrName in computationalInsrs:
        typeCode = "011"
    elif instrName == "jump":
        typeCode = "000"

    # get conditional bit
    condBit = instrList[1]

    # get op code
    opCode = opCodeDict[instrName]

    # get register numbers (in their binary form)
    instrRegs = []
    for i in range(2, len(instrList)):
        if instrList[i][0] == '$': # we are at a register
            regNum = str(bin(int(instrList[i][1])))[2:] # [1] ignores $ symbol, [2:] ignores '0b' characters
            # extend register numbers with 0s to always be regNumLen bits long
            if len(regNum) < regNumLen:
                for i in range(regNumLen - len(regNum)):
                    regNum = "0" + regNum
            instrRegs.append(regNum)

    # there should always be 5 registers
    # if there's not, add in zero-filled registers
    if len(instrRegs) < 5 :
        for i in range(5 - len(instrRegs)):
            instrRegs.append("000000")

    # get immediate value, if any
    immedSignVal = 0
    if "x" in instrList[-1]: # assumes immediates are always given as hex values
        if "-" in instrList[-1] : # flip sign bit if negative
            immedSignVal = 1
        immedVal = str(bin(int(instrList[-1], 16)))[2:]
    elif "<" in instrList[-1]: # TODO: add in the rest of the cmp operators here
        instrRegs[3] = str(bin(5))[2:]
    else:
        immedVal = "0" # assume 0 immedVal if none exists

    # concatenate all binary field values into one string
    binaryInstr = typeCode + condBit + opCode
    for reg in instrRegs:
        binaryInstr += reg
    if len(binaryInstr) + len(immedVal) < registerLen:
        # fill up remaining bits in register with 0s in the immediate bit
        for i in range(24 - len(immedVal)):
            immedVal = "0" + immedVal
    binaryInstr += str(immedSignVal) + immedVal

    # return concatenated string
    return binaryInstr

projectPath = "C:/Users/Chase\'s Laptop/source/repos/SVANN_GUI/"

assemblyFile = projectPath + sys.argv[1]
binaryStr = ""
with open(assemblyFile, "r") as a_file:
  for line in a_file:
    stripped_line = line.strip()
    print('assembly line:',stripped_line)
    print('binary line: ',getBinaryOf(stripped_line))
    binaryStr += getBinaryOf(stripped_line) + "\n"

binaryFile = projectPath + sys.argv[1][:-4] + "Binary.txt"
f = open(binaryFile, "w")
f.write(binaryStr)
f.close()