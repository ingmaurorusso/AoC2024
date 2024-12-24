import os
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExample = """
125 17
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

    if len(lines) > 1:
        print('Not just one line!')
        exit()

    # PART 1

    values = lines[0].split(' ')

    mapVals = dict()

    for s in values:
        v = int(s)
        was = mapVals.get(v);
        mapVals.update({v: 1+(0 if was == None else was)})

    NBlink1 = 25
    NBlink2 = 75
    for i in range(NBlink2+1):
        if i==NBlink1 or i==NBlink2:
            count = 0
            for v in mapVals.keys():
                count += mapVals[v]

            if i==NBlink1:
                count1 = count
            else:
                count2 = count
                break # avoid a useless further cycle

        newMap = dict()
        for v in mapVals.keys():
            s = str(v)
            n = mapVals[v]
            if v == 0:
                v1 = 1
            elif len(s)%2 == 0:
                v1 = int(s[:len(s)//2])
                v2 = int(s[len(s)//2:])

                was = newMap.get(v2);
                newMap.update({v2: n+(0 if was == None else was)})
            else:
                v1 = (v * 2024)

            was = newMap.get(v1);
            newMap.update({v1: n+(0 if was == None else was)})
        
        mapVals = newMap

    print(f"Result Part1: {count1}")


    # PART 2

    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {count2}")


if __name__ == "__main__":
    main()