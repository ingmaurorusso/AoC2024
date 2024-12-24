import os
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExample = """
............
........0...
.....0......
.......0....
....0.......
......A.....
............
............
........A...
.........A..
............
............
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


    count = 0

    # PART 1

    dimY = len(lines)
    dimX = len(lines[0]) if dimY > 0 else 0

    antennas = dict()
    for y in range(len(lines)):
        line = lines[y]
        for x in range(len(line)):
            ch = line[x]
            if ch != '.':
                if not ch in antennas.keys():
                    antennas[ch] = []
                antennas[ch].append(Point(x,y))

    antiNodesPart1 = set()
    antiNodesPart2 = set()

    for ant in antennas.keys():
        pos = antennas[ant]
        for i in range(len(pos)):
            p1 = pos[i]
            
            for j in range(i+1,len(pos)):
                p2 = pos[j]

                delta = Point(p1.x-p2.x, p1.y-p2.y)

                antiNode = p1.copy()
                for k in range(2):
                    c = 0
                    while antiNode.x >= 0 and antiNode.x < dimX and antiNode.y >= 0 and antiNode.y < dimY:
                        if c == 1 and not antiNode in antiNodesPart1: # by values for set, would be by ref for []
                            antiNodesPart1.add(antiNode.copy())

                        if not antiNode in antiNodesPart2: # by values for set, would be by ref for []
                            antiNodesPart2.add(antiNode.copy())

                        antiNode.x += delta.x
                        antiNode.y += delta.y
                        c += 1

                    antiNode = p2.copy()
                    delta.x = -delta.x
                    delta.y = -delta.y

    count = len(antiNodesPart1)
    print(f"Result Part1: {count}")


    # PART 2

    count = len(antiNodesPart2)
    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {count}")


if __name__ == "__main__":
    main()