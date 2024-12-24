import os
import re
import sys

sys.path.append("..")
from include import AOC

inputExample = """
3   4
4   3
2   5
1   3
3   9
3   3
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

    l1 = []
    l2 = []

    for line in lines:
        #print(line)
        values = re.findall(r'[0-9]+', line)

        error = False;
        if len(values) != 2:
            error = True
        else:
            if values[0]+values[1] != line.replace(" ",""):
                error = True

        if error:
            print(f"line {line} does not have exactly two numbers with intermediate spaces")

        v1 = int(values[0])
        v2 = int(values[1])
        l1.append(v1);
        l2.append(v2);

    l1.sort()
    l2.sort()

    # PART 1

    sum = 0
    for i in range(len(l1)):
        v1 = l1[i]
        v2 = l2[i]

        sum += abs(v1-v2)

    print(f"Result Part1: {sum}")


    # PART 2 (re-use lists)
    sum = 0

    next_i1 = 0
    next_i2 = 0
    
    next_it1 = iter(l1)
    next_it2 = iter(l2)

    v2 = 0
    
    if len(l1) > 0:
        v1 = next(next_it1)
        next_i1 = 1

        v2 = next(next_it2)
        next_i2 = 1

    while next_i1 <= len(l1) and next_i2 <= len(l2):

        if v1 > v2:
            adv2 = AOC.first_match(next_it2, lambda v: v >= v1, len(l2)-next_i2) + 1
            # next_it2 already after the matching element

            next_i2 += adv2
            if next_i2 > len(l2):
                break
            v2 = l2[next_i2-1]

        if v1 == v2:
            d1 = AOC.first_match(next_it1, lambda v: v > v1, len(l1)-next_i1) + 1
            next_i1 += d1

            d2 = AOC.first_match(next_it2, lambda v: v > v2, len(l2)-next_i2) + 1
            next_i2 += d2

            sum += (v1*d1*d2);

            if (next_i1 <= len(l1)):
                v1 = l1[next_i1-1]

            if (next_i2 <= len(l2)):
                v2 = l2[next_i2-1]
        else: # v1 < v2:
            adv1 = AOC.first_match(next_it1, lambda v: v >= v2, len(l1)-next_i1)+1
            next_i1 += adv1

            if (next_i1 <= len(l1)):
                v1 = l1[next_i1-1]


    print(f"Result Part2: {sum}")

if __name__ == "__main__":
    main()