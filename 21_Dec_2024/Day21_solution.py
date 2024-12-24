import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
029A
980A
179A
456A
379A
""",]


idxShortTest = 0
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


Anum = 10
avoidNum = 11

posNum = dict()

posNum[7] = [0,3]
posNum[8] = [1,3]
posNum[9] = [2,3]

posNum[4] = [0,2]
posNum[5] = [1,2]
posNum[6] = [2,2]

posNum[1] = [0,1]
posNum[2] = [1,1]
posNum[3] = [2,1]

posNum[avoidNum] = [0,0]
posNum[0] = [1,0]
posNum[Anum] = [2,0]


Up = 0
Right = 1
Down = 2
Left = 3
Adir = 4
avoidDir = 5

def arrowFrom(ch):
    if ch == '^':
        return Up
    if ch == '>':
        return Right
    if ch == 'v':
        return Down
    if ch == '<':
        return Left
    if ch == 'A':
        return Adir
    return avoidDir

posDir = dict()

posDir[avoidDir] = [0,1]
posDir[Up] = [1,1]
posDir[Adir] = [2,1]

posDir[Left] = [0,0]
posDir[Down] = [1,0]
posDir[Right] = [2,0]


def buildSeqFromTo(pBegin, pEnd, xFirst, pAvoid):
    bad = (pEnd == pAvoid)
    for k in range(2):
        seq = ''

        pMove = pBegin.copy()
        for i in range(2):
            isX = (i == 0) == xFirst

            c = 0 if isX else 1

            while pMove[c] < pEnd[c]:
                if pMove == pAvoid:
                    bad = True
                pMove[c] += 1
                seq += '>' if isX else '^'

            while pMove[c] > pEnd[c]:
                if pMove == pAvoid:
                    bad = True
                pMove[c] -= 1
                seq += '<' if isX else 'v'

        if not bad:
            break
    
        xFirst = not xFirst

    return seq


def buildDirSeqNested(seq, nestedSeqs):
    l = 0

    prev = Adir
    for ch in seq:
        curr = Adir if ch == 'A' else arrowFrom(ch)

        l += nestedSeqs[prev][curr]

        prev = curr

    return l



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

    # PART 1

    sum1 = 0
    sum2 = 0


    NumRobotKeyPad = 25
    NumRobotKeyPad_p1 = 2

    bestSeqsForDir = []
    for keyIdx in range(NumRobotKeyPad):
        bestSeqsForDir.append([])

        for i in range(Adir+1):
            bestSeqsForDir[keyIdx].append([])
            for j in range(Adir+1):
                seq1 = buildSeqFromTo(posDir[i],posDir[j],True,posDir[avoidDir]) + "A"

                seq2 = buildSeqFromTo(posDir[i],posDir[j],False,posDir[avoidDir]) + "A"

                l1 = len(seq1) if keyIdx == 0 else buildDirSeqNested(seq1, bestSeqsForDir[keyIdx-1])

                l2 = len(seq2) if keyIdx == 0 else buildDirSeqNested(seq2, bestSeqsForDir[keyIdx-1])

                bestSeqsForDir[keyIdx][i].append(l1 if l1 <= l2 else l2)


    for code in lines:
        l_p1 = 0
        l_p2 = 0

        prev = Anum
        for ch in code:
            curr = Anum if ch == 'A' else int(ch)

            seq1 = buildSeqFromTo(posNum[prev],posNum[curr],True,posNum[avoidNum]) + "A"
            seq2 = buildSeqFromTo(posNum[prev],posNum[curr],False,posNum[avoidNum]) + "A"
        
            l1_p1 = len(seq1) if NumRobotKeyPad_p1 == 0 else buildDirSeqNested(seq1, bestSeqsForDir[NumRobotKeyPad_p1-1])
            l2_p1 = len(seq2) if NumRobotKeyPad_p1 == 0 else buildDirSeqNested(seq2, bestSeqsForDir[NumRobotKeyPad_p1-1])
        
            l1_p2 = len(seq1) if NumRobotKeyPad == 0 else buildDirSeqNested(seq1, bestSeqsForDir[NumRobotKeyPad-1])
            l2_p2 = len(seq2) if NumRobotKeyPad == 0 else buildDirSeqNested(seq2, bestSeqsForDir[NumRobotKeyPad-1])

            l_p1 += (l1_p1 if l1_p1 <= l2_p1 else l2_p1)
            l_p2 += (l1_p2 if l1_p2 <= l2_p2 else l2_p2)

            prev = curr


        val = int(code[:-1])

        sum1 += (val * l_p1)
        sum2 += (val * l_p2)

    print(f"Result Part1: {sum1}")


    # PART 2

    print(f"Result Part2: {sum2}")


if __name__ == "__main__":
    main()