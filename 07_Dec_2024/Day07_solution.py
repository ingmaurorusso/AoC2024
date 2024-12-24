import os
import sys

from enum import Enum

sys.path.append("..")
from include import AOC


inputExample = """
190: 10 19
3267: 81 40 27
83: 17 5
156: 15 6
7290: 6 8 6 15
161011: 16 10 13
192: 17 8 14
21037: 9 7 18 13
292: 11 6 16 20
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


def numDigits10(v):
    count = 1
    while v >= 10:
        count += 1
        v //= 10
    
    return count


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


    count1 = 0
    sum1 = 0

    count2 = 0
    sum2 = 0

    # PART 1

    for line in lines:
        testAndValues = line.split(':')

        testValue = int(testAndValues[0].strip())

        values = [int(s) for s in testAndValues[1].strip().split(' ')]

        firstValue = values[0]
        values = values[1:]

        alternatives = '+*|'

        v_ops = []
        v_res = []
        res = firstValue
        for i in range(len(values)):
            v_ops.append(0);
            res += values[i]
            v_res.append(res)

        okEq1 = (res == testValue)
        okEq2 = okEq1
        ended = False

        while not ((okEq1 and okEq2) or ended):
            changeIdx = len(values)-1

            while (v_ops[changeIdx] == len(alternatives)-1):
                if changeIdx==0:
                    ended = True
                    break;
                changeIdx -= 1
    
            if not ended:
                v_ops[changeIdx] += 1

                for i in range(changeIdx+1, len(values)):
                    v_ops[i] = 0

                res = firstValue if changeIdx == 0 else v_res[changeIdx-1]
                for i in range(changeIdx, len(values)):
                    if alternatives[v_ops[i]] == '+':
                        res += values[i]
                    elif alternatives[v_ops[i]] == '*':
                        res *= values[i]
                    else:
                        # res = res * pow(10, numDigits10(values[i])) + values[i]
                        res = int(str(res) + str(values[i]))

                    v_res[i] = res

                if res == testValue:
                    okEq2 = True

                    if not okEq1:
                        okEq1 = True # except '|' is found
                        for i in range(len(values)):
                            if alternatives[v_ops[i]] == '|':
                                okEq1 = False
                                break


        if okEq1:
            count1 += 1
            sum1 += testValue

        if okEq2:
            count2 += 1
            sum2 += testValue

    print(f"Line count: {len(lines)}")
    print(f"N. good values Part1: {count1}")
    print(f"Result Part1: {sum1}")


    # PART 2


    print(f"Line count: {len(lines)}")
    print(f"N. good values Part2: {count2}")
    print(f"Result Part2: {sum2}")


if __name__ == "__main__":
    main()