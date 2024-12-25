import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
#####
.####
.####
.####
.#.#.
.#...
.....

#####
##.##
.#.##
...##
...#.
...#.
.....

.....
#....
#....
#...#
#.#.#
#.###
#####

.....
.....
#.#..
###..
###.#
###.#
#####

.....
.....
.....
#....
#.#..
#.#.#
#####
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

    # empty lines needed in this problem
    #while('' in lines):
    #    # print(f'an empty line: idx {lines.index('')+1}') allowed for this problem
    #    lines.remove('')

    if len(lines) == 0:
        print('No lines or only empty lines!')
        return
    
    lines.append('')

    keys = []
    locks = []

    elem = []

    for line in lines:
        if (line == '') and (len(elem) > 0):
            if elem[-1].find('.') != -1 :
                locks.append(elem)
            else:
                keys.append(elem)

            elem = []
        else:
            elem.append(line)

    # PART 1

    if len(keys) == 0:
        return 0

    Y = len(keys[0])
    X = len(keys[0][0])

    count1 = 0
    count2 = 0

    for key in keys:
        hsKey = []
        for x in range(X):
            hsKey.append(0)
            for y in range(Y):
                if key[Y-1 - y][x] == '#':
                    hsKey[x] = y

        for lock in locks:
            hsLock = []
            for x in range(X):
                hsLock.append(0)
                for y in range(Y):
                    if lock[y][x] == '#':
                        hsLock[x] = y


            ok = True
            for x in range(X):
                if hsKey[x] + hsLock[x] + 2 > Y:
                    ok = False
                    break

            if ok:
                count1 += 1

    print(f"Result Part1: {count1}")


    # PART 2

    print(f"Result Part2: {count2}")


if __name__ == "__main__":
    main()