import os
import sys

from enum import Enum

sys.path.append("..")
from include import AOC


inputExample = """
MMMSXXMASM
MSAMXMSMSA
AMXSXMAAMM
MSAMASMSMX
XMASAMXAMM
XXAMMXXAMA
SMSMSASXSS
SAXAMASAAA
MAMMMXMMMM
MXMXAXMASX
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

XMAS = 'XMAS'
LenXS = len(XMAS)

def xmasCheck(lines, i, j, dx, dy):
    # from string to list
    sample = list(XMAS) # need to fix the lenggth, will be updated

    if ( i+1 >= -LenXS*dx if dx < 0 else (i <= len(lines[0]) - LenXS*dx) ):
         if ( j+1 >= -LenXS*dy if dy < 0 else (j <= len(lines) - LenXS*dy) ):
            for k in range(LenXS):
               sample[k] = lines[j][i]
               i += dx
               j += dy

            sample = ''.join(sample) # from list to string

            if sample == XMAS:
                return True
            if sample[::-1] == XMAS:
                return True
    
    return False

MAS = 'MAS'
LenMS = len(MAS)

def pivotMasCheck(lines, i, j):
    # from string to list
    sampleL = list(MAS) # need to fix the lenggth, will be updated
      
    f = -1
    for twice in range(2):
        m = 0
        for k in range(-(LenMS//2), 1 + (LenMS//2)):
            sampleL[m] = lines[j+k*f][i+k]
            m += 1
        
        sample = ''.join(sampleL) # from list to string

        if sample != MAS:
          if sample[::-1] != MAS:
            return False

        f = 1
    
    return True


class AheadDir(Enum):
    UP = 1
    STRAIGHT = 2
    DOWN = 3

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

    for i in range(len(lines)):
        if len(lines[i]) != len(lines[0]):
            print(f"lines with different lengths: first has {len(lines[0])}, n. {i+1} has {len(lines[i])}")
            exit()
            
    
    count = 0

    # PART 1

    for i in range(len(lines[0])):
        for j in range(len(lines)):
            dx = 1
            for d in AheadDir:
                if d == AheadDir.UP:
                    dy = -1
                elif d == AheadDir.STRAIGHT:
                    dy = 0
                else: # AheadDir.DOWN
                    dy = 1

                if xmasCheck(lines,i,j,dx,dy):
                    count += 1

            # vertical
            if xmasCheck(lines,i,j,0,1):
                count += 1

    print(f"Result Part1: {count}")


    # PART 2

    count = 0
    for i in range(1, len(lines[0])-1):
        for j in range(1, len(lines)-1):
            if lines[j][i] == 'A'  and  pivotMasCheck(lines, i,j):
                count += 1

    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {count}")


if __name__ == "__main__":
    main()