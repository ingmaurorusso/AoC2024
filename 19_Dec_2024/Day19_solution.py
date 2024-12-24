import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
r, wr, b, g, bwu, rb, gb, br

brwrr
bggr
gbbr
rrbgbr
ubwu
bwurrg
brgr
bbrgwb
"""]


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

towels = []
already = set()

def check(residualLine, idxStart):
    global towels
    global already

    if idxStart in already:
        return False    
    already.add(idxStart)

    for towel in towels:
        if residualLine.find(towel) == 0:
            if check(residualLine[len(towel):], idxStart+len(towel)):
                return True
    
    return len(residualLine) == 0

alreadyCount = dict()
def count(residualLine, idxStart):
    global towels
    global alreadyCount

    if idxStart in alreadyCount.keys():
        return alreadyCount[idxStart]

    partial = 1 if len(residualLine) == 0 else 0

    for towel in towels:
        if residualLine.find(towel) == 0:
            partial += count(residualLine[len(towel):], idxStart+len(towel))
    
    alreadyCount.update({idxStart: partial})

    return partial


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

    global towels
    towels = lines[0].split(',')
    for i in range(len(towels)):
        towels[i] = towels[i].strip()
        if len(towels[i]) == 0:
            raise 'Empty towel(s)!'

    count1 = 0
    count2 = 0

    global already
    global alreadyCount
    for i in range(1,len(lines)):
        line = lines[i]
        if check(line,0):
            count1 += 1
        already.clear()

        count2 += count(line,0)
        alreadyCount.clear()


    print(f"Result Part1: {count1}")


    # PART 2

    print(f"Result Part2: {count2}")


if __name__ == "__main__":
    main()