import bisect
import copy
import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
2333133121414131402
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

    regionsFree = []
    regionsFull = []

    pos = 0
    free = False
    id = 0
    for ch in lines[0]:
        size = int(ch)

        if free:
            if size > 0: # useless otherwise
                regionsFree.append([pos, size])
        else:
            if size > 0: # useless otherwise
                regionsFull.append([pos, size, id])
            id += 1

        free = not free
        pos += size

    # prepare for part 2 before modifying the input regions
    regionsFull_save = copy.deepcopy(regionsFull)

    regionsFreeDict = dict()
    maxFreeSize = 0
    for free in regionsFree:
        pos = free[0]
        size = free[1]
        if not size in regionsFreeDict.keys():
            regionsFreeDict[size] = list()
        regionsFreeDict[size].append(pos)
        if size > maxFreeSize:
            maxFreeSize = size


    # PART 1

    initialFullNumber = len(regionsFull)
    iBackFull = len(regionsFull)
    iFrontFree = 0
    while (iBackFull > 0) and (iFrontFree < len(regionsFree)):
        iBackFull -= 1

        # look for free space in a position on the left

        posToMove = regionsFull[iBackFull][0]
        sizeToMove = regionsFull[iBackFull][1]
        idToMove = regionsFull[iBackFull][2]

        posFree = regionsFree[iFrontFree][0]
        sizeFree = regionsFree[iFrontFree][1]
        if posToMove < posFree:
            iBackFull += 1 # in order to keep this full region
            break

        sizeMove = min(sizeFree,sizeToMove)
        regionsFull.append([posFree, sizeMove, idToMove])

        fullMove = (sizeMove == sizeToMove)
        fullUsed = (sizeMove == sizeFree)

        regionsFree[iFrontFree][0] += sizeMove
        regionsFree[iFrontFree][1] -= sizeMove
        if fullUsed:
            iFrontFree += 1

        regionsFull[iBackFull][1] -= sizeMove
        if not fullMove:
            iBackFull += 1 # anti "-= 1" at the begin of next cycle

    regionsFree = regionsFree[iFrontFree:len(regionsFree)-1]
    regionsFull = regionsFull[0:iBackFull] + regionsFull[initialFullNumber:]

    sum1 = 0
    for full in regionsFull:
        pos = full[0]
        size = full[1]
        id = full[2]

        add = (id * (pos*size + size*(size-1)//2))
        sum1 += add

    print(f"Result Part1: {sum1}")


    # PART 2

    regionsMoved = []
    regionsFull = regionsFull_save

    initialFullNumber = len(regionsFull)
    iBackFull = len(regionsFull)
    while (iBackFull > 0) and (len(regionsFreeDict) > 0):
        iBackFull -= 1

        posToMove = regionsFull[iBackFull][0]
        sizeToMove = regionsFull[iBackFull][1]
        idToMove = regionsFull[iBackFull][2]

        # look for free space in a position on the left
        # for each size >= sizeToMove, search the leftest.

        # iFreeKey = bisect.bisect_left( regionsFreeDict.keys(), sizeToMove )
        # if iFreeKey != -1:
        #     # iFreeKey is the position of the first >= sizeToMove

        leftestFreePos = posToMove
        sizeFree = 0
        for size in range(sizeToMove,maxFreeSize+1):
            if size in regionsFreeDict.keys():
                if regionsFreeDict[size][0] < leftestFreePos:
                    leftestFreePos = regionsFreeDict[size][0]
                    sizeFree = size

        if leftestFreePos == posToMove: # there is no suitable space
            regionsMoved.append(regionsFull[iBackFull]) # as it is
        else:
            regionsMoved.append([leftestFreePos, sizeToMove, idToMove]) # moved left

            posFreeNew = regionsFreeDict[sizeFree][0] + sizeToMove

            fullUsed = (sizeToMove == sizeFree)

            if len(regionsFreeDict[sizeFree]) == 1:
                del regionsFreeDict[sizeFree]
            else:
                regionsFreeDict[sizeFree] = regionsFreeDict[sizeFree][1:] # remove first

            if not fullUsed:
                # add residual in the proper list
                residualSize = sizeFree - sizeToMove
                if residualSize in regionsFreeDict.keys():
                    # search the position where to put i
                    iInsert = bisect.bisect_left( regionsFreeDict[residualSize], posFreeNew )
                    if iInsert == -1:
                        regionsFreeDict[residualSize].append(posFreeNew)
                    else:
                        regionsFreeDict[residualSize].insert(iInsert, posFreeNew)
                else:
                    regionsFreeDict.insert({residualSize: [posFreeNew]})


    regionsFull = regionsMoved

    sum2 = 0
    for full in regionsFull:
        pos = full[0]
        size = full[1]
        id = full[2]

        add = (id * (pos*size + size*(size-1)//2))
        sum2 += add

    print(f"Result Part2: {sum2}")


if __name__ == "__main__":
    main()
