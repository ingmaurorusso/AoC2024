import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
Register A: 729
Register B: 0
Register C: 0

Program: 0,1,5,4,3,0
""",
"""
Register A: 729
Register B: 0
Register C: 0

Program: 0,3,5,4,3,0
""",]


idxShortTest = 2
isShortTest = False

def extractInput(dayLabel, idx):
    inputPath = os.path.join(os.getcwd(), dayLabel + '_input_file.txt')
    #print(f"inputPath: {inputPath}")

    if not os.path.exists(inputPath):
        print(f"Input file <{inputPath}> not existing")
        exit()

    global isShortTest
    global inputExamples

    isShortTest = (idx < len(inputExamples))

    if not isShortTest:
        inputText = ''
        with open(inputPath, 'r') as file:
            inputText = inputText + file.read()
    else:
        inputText = inputExamples[idx]

    #print(inputText)
    return inputText


@dataclass
class Point:
    x: int
    y: int

    def __hash__(self):
        return self.x*123 + self.y
    
    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def copy(self):
        return Point(self.x,self.y)
    

def main():
    dayLabel = AOC.extractDayLabel(sys.argv, os.path.dirname(os.path.abspath(__file__)).split('/')[-1])

    if dayLabel == None:
        print("Use one parameter (only): day label, or use day-label as first digits appearing in parent dir")
        exit()

    print(f"Day label is: {dayLabel}")

    global idxShortTest
    for idx in range(idxShortTest, len(inputExamples)+1):
        inputText = extractInput(dayLabel, idx)

        if isShortTest:
            print(f' - - - EXAMPLE {idx+1} - - -')
        else:
            print(' - - - REAL INPUT - - -')

        doJob(inputText)

triplets = []
program = []

def checkTriplets(idxBack):
    # Remember that this code is embedded on the input file,
    # it does not work for any program.

    global prorgam
    N = len(program)

    global triplets

    for treeBits in range(8):
        triplets[N-1 - idxBack] = treeBits

        # look for constraints on higher bits.
        treeBitsInv = 7 - treeBits; # B ^= 7

        baseBitPos = idxBack*3
        beginHigherPos = baseBitPos+treeBitsInv; # C = A >> B

        j = beginHigherPos//3
        if j < N:
            treeBitsHigher = triplets[N-1 - j]

            down = beginHigherPos % 3

            treeBitsHigher2 = 0
            if (j+1 < N) and (down > 0):
                treeBitsHigher2 = triplets[N-1 - (j+1)]

            treeBitsHigher >>= down
            treeBitsHigher |= ((treeBitsHigher2 << (3-down)) & 7)
        else:
            # bits too high, for sure zeros.
            treeBitsHigher = 0

        treeBits ^= treeBitsHigher # B ^= C
        # don't mention second B ^= 7 because the first was applied
        # only to 'treeBitsInv' to support the correct evaluation
        # of "C = A >> B".

        if treeBits != program[idxBack]:
            # This configuration is not good, must be changed.
            pass
        elif idxBack == 0:
            return True
        else:
            if checkTriplets(idxBack-1):
                return True
        
    return False


def doJob(inputText):
    lines = inputText.split('\n')
    if isShortTest:
        # first and last lines empty
        if len(lines)>0 and len(lines[0])==0 :
            lines = lines[1:]
        if len(lines)>0 and len(lines[-1])==0 :
            lines = lines[:-1]
    print(f"Number of lines {len(lines)}");

    while('' in lines):
        # print(f'an empty line: idx {lines.index('')+1}') allowed for this problem
        lines.remove('')

    if len(lines) == 0:
        print('No lines or only empty lines!')
        return
    
    REG_A = 0
    REG_B = 1
    REG_C = 2
    regs = [0,0,0]

    for line in lines:
        parts = line.split(':')

        if parts[0] == 'Register A':
            regs[REG_A] = int(parts[1])
        elif parts[0] == 'Register B':
            regs[REG_B] = int(parts[1])
        elif parts[0] == 'Register C':
            regs[REG_C] = int(parts[1])
        else:
            strProgram = parts[1].split(',')

            global program
            program = []
            for s in strProgram:
                program.append(int(s))

            ADV = 0
            BXL = 1
            BST = 2
            JNZ = 3
            BXC = 4
            OUT = 5
            BDV = 6
            CDV = 7

            output = []

            opIdx = 0
            while opIdx < len(program):
                opCode = program[opIdx]
                litOperand = program[opIdx+1]

                comboOperand = litOperand if litOperand < 4 else (0 if litOperand == 7 else regs[litOperand-4])

                isComboOp = False
                jumped = False

                if opCode == ADV:
                    regs[REG_A] >>= comboOperand
                    isComboOp = True
                
                elif opCode == BXL:
                    regs[REG_B] ^= litOperand

                elif opCode == BST:
                    regs[REG_B] = (comboOperand & 7)
                    isComboOp = True

                elif opCode == JNZ:
                    if regs[REG_A] != 0:
                        opIdx = litOperand
                        jumped = True
 
                elif opCode == BXC:
                    regs[REG_B] ^= regs[REG_C]

                elif opCode == OUT:
                    output.append(comboOperand & 7)
                    isComboOp = True

                elif opCode == BDV:
                    regs[REG_B] = (regs[REG_A] >> comboOperand)
                    isComboOp = True

                elif opCode == CDV:
                    regs[REG_C] = (regs[REG_A] >> comboOperand)
                    isComboOp = True


                if not jumped:
                     opIdx += 2

    # PART 1

    output = str(output).replace(' ','')
    output = output[1:len(output)-1] # remove '[' and ']'

    print(f"Result Part1: {output}")


    # PART 2

    N = len(program)

    global triplets
    triplets = []
    for i in range(N):
        triplets.append(0)


    ok = checkTriplets(N-1)

    if ok:
        A = 0

        for i in range(N):
            treeBits = triplets[i]

            A <<= 3
            A |= treeBits


    print(f"Result Part2: {A if ok else "not found"}")


if __name__ == "__main__":
    main()