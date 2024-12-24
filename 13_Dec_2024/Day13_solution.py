import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExample = """
Button A: X+94, Y+34
Button B: X+22, Y+67
Prize: X=8400, Y=5400

Button A: X+26, Y+66
Button B: X+67, Y+21
Prize: X=12748, Y=12176

Button A: X+17, Y+86
Button B: X+84, Y+37
Prize: X=7870, Y=6450

Button A: X+69, Y+23
Button B: X+27, Y+71
Prize: X=18641, Y=10279
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
        # print(f'an empty line: idx {lines.index('')+1}') allowed for this problem
        lines.remove('')

    if len(lines) == 0:
        print('No lines or only empty lines!')
        exit()

    # PART 1

    eq1 = [0,0,0]
    eq2 = [0,0,0]

    count1 = 0
    count2 = 0
    nSys = 0

    for line in lines:
        read = re.findall(r'(^.*?)\:(.*?)X.([\d]+)(.*?)Y.([\d]+).*', line)[0]

        if read[0] == 'Button A':
            eq1[0] = int(read[2])
            eq2[0] = int(read[4])
        elif read[0] == 'Button B':
            eq1[1] = int(read[2])
            eq2[1] = int(read[4])
        else:
            eq1[2] = int(read[2])
            eq2[2] = int(read[4])
            nSys += 1

            # compute intersection
            det = eq1[0]*eq2[1] - eq1[1]*eq2[0]

            if abs(det) < 0.5:
                continue

            matrix = [[eq1[0], eq1[1]], [eq2[0], eq2[1]]]

            C1 = [eq1[2], eq2[2]]

            x = (matrix[1][1] * C1[0] - matrix[0][1] * C1[1]) / det
            y = (matrix[0][0] * C1[1] - matrix[1][0] * C1[0]) / det

            if x == int(x) and x >= 0 and x <= 100:
                if y == int(y) and y >= 0 and y <= 100:
                    count1 += int(3*x + y + 0.1)

            ADD = 10000000000000

            C2 = [C1[0]+ADD, C1[1]+ADD]

            x = (matrix[1][1] * C2[0] - matrix[0][1] * C2[1]) / det
            y = (matrix[0][0] * C2[1] - matrix[1][0] * C2[0]) / det

            if x == int(x) and x >= 0:
                if y == int(y) and y >= 0:
                    count2 += int(3*x + y + 0.1)

    print(f"Result Part1: {count1}")


    # PART 2

    print(f"System number: {nSys}")
    print(f"Result Part2: {count2}")


if __name__ == "__main__":
    main()