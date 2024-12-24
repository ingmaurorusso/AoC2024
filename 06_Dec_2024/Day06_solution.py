import os
import re
import sys

from enum import Enum

sys.path.append("..")
from include import AOC
from include import Space_MR


inputExample = """
....#.....
.........#
..........
..#.......
.......#..
..........
.#..^.....
........#.
#.........
......#...
"""


isShortTest = False

def extractInput(dayLabel):
    inputPath = os.path.join(os.getcwd(), dayLabel + '_input_file.txt')
    #print(f"inputPath: {inputPath}")

    if not os.path.exists(inputPath):
        print(f"Input file <{inputPath}> not existing")
        exit()
    
    global isShortTest
    global inputExample
    if not isShortTest:
        inputText = ''
        with open(inputPath, 'r') as file:
            inputText = inputText + file.read()
    else:
        inputText = inputExample

    #print(inputText)
    return inputText


def findPatternToExit_OrLoop(field2D, startPos, startDir):
    patternMoves = 0

    pos = startPos.copy()
    dir = Space_MR.Direction4(startDir)

    passedPos = set()
    passedPos.add(startPos)

    passedAntiLoopPos = set()
    passedAntiLoopPos.add((startPos,startDir))
    patternMoves = 1

    ended = False
    while not ended:
        prevPoint = pos.copy()

        if not field2D.movePoint(pos,dir):
            ended = True
            break

        if field2D.getP(pos) == '#':
            # go back and rotate direction
            pos = prevPoint.copy()
            dir = Space_MR.rotateDir4(dir, True)
        #elif not pos in passedPos:
        else:
            passedPos.add(pos.copy())
            lenWas = len(passedAntiLoopPos)
            passedAntiLoopPos.add((pos.copy(),Space_MR.Direction4(dir)))
            if lenWas == len(passedAntiLoopPos):
                # found a loop!
                return (None,True)

            patternMoves += 1

    # patternMoves not used
    return (passedPos,False)


def findArrows(line):
    posFound = None
    dirFound = None
    multipleArrows = False
    for dir in Space_MR.Direction4:
        prevArrowPos = -1
        while True:
            arrowPos = prevArrowPos+1 + line[prevArrowPos+1:].find(Space_MR.dir4ToArrow(dir))
            if arrowPos == prevArrowPos: # -1 from .find
                break

            if posFound != None:
                multipleArrows = True
            else:
                posFound = arrowPos
                dirFound = Space_MR.Direction4(dir)

            prevArrowPos = arrowPos

    return (posFound, dirFound, multipleArrows)


def main():
    dayLabel = AOC.extractDayLabel(sys.argv, os.path.dirname(os.path.abspath(__file__)).split('/')[-1])

    if dayLabel == None:
        print("Use one parameter (only): day label, or use day-label as first digits appearing in parent dir")
        exit()

    print(f"Day label is: {dayLabel}")

    inputText = extractInput(dayLabel)

    lines = inputText.split('\n')
    if isShortTest:
        # first and last lines empty
        if len(lines)>0 and len(lines[0])==0 :
            lines = lines[1:]
        if len(lines)>0 and len(lines[-1])==0 :
            lines = lines[:-1]
    print(f"Number of lines {len(lines)}");

    while('' in lines):
        print(f'an empty line: idx {lines.index('')+1}')
        lines.remove('')

    if len(lines) == 0:
        print('No lines or only empty lines!')
        exit()

    # PART 1

    posFound = False
    guardPos = None
    iLine = 0
    for line in lines:
        (posFound, dirFound, multipleArrows) =  findArrows(line)

        if multipleArrows or (posFound != None and guardPos != None):
            raise ValueError('Multiple arrows in the scheme')
        
        if posFound != None:
            guardPos = Space_MR.Point(posFound,iLine)
            guardDir = dirFound

        iLine += 1

    if guardPos == None:
        raise ValueError('No arrow in the scheme')

    field2D = Space_MR.Field2D(lines)
            
    (passedPos, loop) = findPatternToExit_OrLoop(field2D, guardPos, guardDir)
    if loop:
        raise ValueError('Loop by the initial scheme')
    count = len(passedPos)

    print(f"Result Part1: {count}")


    # PART 2
    count = 0
    for p in passedPos:
        if p == guardPos:
            continue
    
        field2D.setP(p,'#')
        (passedPos, loop) = findPatternToExit_OrLoop(field2D, guardPos, guardDir)
        if loop:
            count += 1
        field2D.setP(p,'.')


    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {count}")


if __name__ == "__main__":
    main()