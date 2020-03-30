#!/usr/bin/python

import sys

# in: string instruction line in assembly
# out: string instruction line in binary
def getBinaryOf(instr) :
    regNumLen = 6 # there are 2^6 = 64 GPRs
    registerLen = 64 # each GPR has 64 bits

    # list and dictionary provide map of instructions to op/type code
    dataTransferInstrs = ["intload", "fload"]
    computationalInsrs = ["add"]
    opCodeDict = {
        "intload" : "0111",
        "fload" : "0100",
        "add" : "000000"
    }

    # split up instrList for easier access
    instrList = instr.split(' ')
    instrName = instrList[0]

    # get type code
    if instrName in dataTransferInstrs:
        typeCode = "001"
    elif instrName in computationalInsrs:
        typeCode = "011"

    # get conditional bit
    condBit = instrList[1]

    # get op code
    opCode = opCodeDict[instrName]

    # get register numbers (in their binary form)
    instrRegs = []
    for i in range(2, len(instrList) - 1):
        # extend register numbers with 0s to always be regNumLen bits long
        regNum = str(bin(int(instrList[i][1])))[2:]
        if len(regNum) < regNumLen:
            for i in range(regNumLen - len(regNum)):
                regNum = "0" + regNum
        instrRegs.append(regNum) # [1] ignores $ symbol

    # get immediate value, if any
    if "$" not in instrList[-1]:
        immedVal = str(bin(int(instrList[-1], 16)))[2:]
    else:
        immedVal = "0000" # assume 0 immedVal if none exists

    # concatenate all binary field values into one string
    binaryInstr = typeCode + condBit + opCode
    for reg in instrRegs:
        binaryInstr += reg
    if len(binaryInstr) + len(immedVal) < registerLen:
        # fill up remaining bits in register with 0s in the immediate bit
        for i in range(registerLen - (len(binaryInstr) + len(immedVal))):
            immedVal = "0" + immedVal
    binaryInstr += immedVal

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