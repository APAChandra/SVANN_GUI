#!/usr/bin/python

import sys


# in: string instruction line in assembly
# out: string instruction line in binary
def get_binary_of(instr):
    regNumLen = 6  # there are 2^6 = 64 GPRs
    numRegs = 5  # there are 5 register fields
    # registerLen = 64        # each GPR has 64 bits
    immedFieldLen = 24  # immediate field is 24 bits

    # parse out all parameters and get instruction name
    instrList = instr.split(' ')
    instrName = instrList[0]

    # get type code
    dataTransferInstrs = ["LOADINT", "INTSTORE"]
    computationalInstrs = ["ADD"]
    controlFlowInstrs = ["CMP"]

    if instrName in dataTransferInstrs:
        typeCode = "001"
    elif instrName in computationalInstrs:
        typeCode = "011"
    elif instrName in controlFlowInstrs:
        typeCode = "011"
    elif instrName == "JUMP":  # JUMP is the only instruction with tyeCode 000
        typeCode = "000"

    # get conditional bit
    condBit = instrList[1]

    # get opcode
    opCodeDict = {
        "LOADINT": "00111",
        "INTSTORE": "01000",
        "ADD": "00000",
        "CMP": "01010",
        "JUMP": "00000"
    }

    opCode = opCodeDict[instrName]

    # get register numbers (in their binary form)
    # grab hex immediate value if present
    instrRegs = []
    immedValHex = "0"
    for param in instrList[2:]:
        if param[0] == 'R':
            # extend register numbers with 0s to always be regNumLen bits long
            binRegNum = str(bin(int(param[1:])))[2:]
            if len(binRegNum) < regNumLen:
                for i in range(regNumLen - len(binRegNum)):
                    binRegNum = "0" + binRegNum
            instrRegs.append(binRegNum)
        elif len(param) > 1 and param[1] == 'x':
            immedValHex = param
    print(instrRegs)

    # grab special compare params if CMP instruction
    if instrName == "CMP":
        cmpType = instrList[-2]
        cmpOperand = instrList[-1]

        # convert compare params to binary and place them in register
        cmpParamsDict = {
            ("", ""): "",
            ("INT", ">"): "000101",
            ("INT", "!="): "000001"
        }
        instrRegs.append(cmpParamsDict[(cmpType, cmpOperand)])

    # add in any registers not used as zero-filled fields
    if len(instrRegs) < numRegs:
        for i in range(numRegs - len(instrRegs)):
            instrRegs.append("000000")

    # convert immediate value to binary if present
    immedValSign = "0"  # will be flipped to 1 if hex is present
    immedValBin = ""
    if immedValHex != "0":
        immedValSign = immedValHex[0]
        immedValBin = str(bin(int(immedValHex[2:], 16)))[2:]

    # fill immediate with correct number of zeroes
    if len(immedValBin) < immedFieldLen:
        for i in range(immedFieldLen - len(immedValBin)):
            immedValBin = "0" + immedValBin

    # concatenate all binary field values into one string
    binaryInstr = typeCode + condBit + opCode
    for reg in instrRegs:
        binaryInstr += reg
    binaryInstr += immedValSign + immedValBin

    # return concatenated string
    return binaryInstr


projectPath = "C:/Users/Chase\'s Laptop/source/repos/SVANN_GUI/"
# projectPath = "C:/Users/Chase's Laptop/PycharmProjects/535Assembler/"

assemblyFile = projectPath + sys.argv[1]
binaryStr = ""
with open(assemblyFile, "r") as a_file:
    for line in a_file:
        stripped_line = line.strip()
        print('assembly line:', stripped_line)
        print('binary line: ', get_binary_of(stripped_line))
        binaryStr += get_binary_of(stripped_line) + "\n"
        # add in exit instruction
    binaryStr += "1110000000000000000000000000000000000000000000000000000000000000\n"

binaryFile = projectPath + sys.argv[1][:-4] + "Binary.txt"
f = open(binaryFile, "w")
f.write(binaryStr)
f.close()
