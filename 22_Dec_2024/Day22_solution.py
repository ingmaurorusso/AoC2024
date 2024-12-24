import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
123
""",

"""
1
10
100
2024
""",

"""
1
2
3
2024
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

    Nsteps = 2000

    mask = 0xFFFFFF

    buyersBananas = []
    bananas = dict()

    n = 0
    for line in lines:
        secret = int(line)

        oldPrice = secret % 10
        buyersBananas.append(dict())

        changes = []
        for i in range(Nsteps):
            secret = ((secret << 6) ^ secret) & mask # <<6 as *64
            secret = ((secret >> 5) ^ secret) & mask # >>5 as //32
            secret = ((secret << 11) ^ secret) & mask # <<11 as *2048

            newPrice = secret % 10

            changes.append(newPrice - oldPrice)

            if i >= 3:
                # represent the list of 4 changes throuh a decimal number,
                # as lists cannot be put in a dictionary (they are not hashable).
                changes4 = list()
                changes4.append(changes[-4])
                changes4.append(changes[-3])
                changes4.append(changes[-2])
                changes4.append(changes[-1])

                key = 0
                for i in range(4):
                    key *= 100
                    key += (abs(changes4[i]) + (10 if changes4[i] < 0 else 0))

                if not key in buyersBananas[n].keys():
                    buyersBananas[n].update({key: newPrice})

            oldPrice = newPrice

        for key in buyersBananas[n].keys():
            if not key in bananas.keys():
                bananas.update({key: 0})
            
            bananas.update({key: bananas[key]+buyersBananas[n][key]});

        sum1 += secret
        n += 1

    best = 0
    bestComb = None
    for key, number in bananas.items():
        if number > best:
            best = number

            # revert from key to changes4
            changes4 = [0,0,0,0]

            for i in range(4):
                changes4[3-i] = key % 20
                if changes4[3-i] >= 10:
                    changes4[3-i] = -(changes4[3-i]-10)
                key //= 100

            bestComb = changes4

    print(f"Result Part1: {sum1}")


    # PART 2

    print(f"Result Part2: {best}")
    print(f"  best comb: {bestComb}")


if __name__ == "__main__":
    main()