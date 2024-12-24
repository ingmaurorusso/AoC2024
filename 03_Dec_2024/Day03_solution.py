import os
import re
import sys

sys.path.append("..")
from include import AOC

from dataclasses import dataclass


inputExample = """
xmul(2,4)&mul[3,7]!^don't()_mul(5,5)+mul(32,64](mul(11,8)undo()?mul(8,5))
"""

# xmul(2,4)%&mul[3,7]!@^do_not_mul(5,5)+mul(32,64]then(mul(11,8)mul(8,5))

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

    
    sum = 0

    mulR = r'mul\([0-9]{1,3}\,[0-9]{1,3}\)'
    valR = r'[0-9]{1,3}'

    # PART 1

    for line in lines:
        # print(line)
        muls = re.findall(mulR, line)

        for mul in muls:
            twoVals = re.findall(valR, mul)
            v1 = int(twoVals[0])
            v2 = int(twoVals[1])

            sum += (v1*v2)

        # print(values)

    print(f"Result Part1: {sum}")


    # PART 2

    doS = "do()"
    dontS = "don\'t()"

    superline = doS # start with enabled
    for line in lines:
        superline = superline + line + '_'
        # intermediate underscore to avoid 'mul' over multiple lines
        # It seems this was not needed in the context.

    # METHOD 1
    sum = 0

    doParts = superline.split(doS)

    for do in doParts:
        two = do.split(dontS)

        values = re.findall(mulR, two[0])

        for mul in values:
            twoVals = re.findall(valR, mul)
            v1 = int(twoVals[0])
            v2 = int(twoVals[1])

            sum += (v1*v2)

    print(f"Result Part2: {sum}")


    # METHOD 2: fully use regex
    sum = 0

    doR = r"do\(\)"
    dontR = r"don\'t\(\)"

    # Add a final doS or dontS, will simplify the regex,
    # avoiding the need to stop even to the end.
    # Remind that there is at least 1 do() as artificially
    # added on the ehad of superline.
    # -> instead, replaced ?={dontR} with $ | (?={dontR})
    #    in regex and the same for ?={doR}
    #dos = re.findall(rf'{doR}|{dontR}', superline)
    #if dos[-1] == doS:
    #    superline += dontS
    #else:
    #    superline += doS

    doG = "doGroup"
    dontG = "dontGroup"

    matchDos = re.finditer(rf'(?P<{doG}>{doR}(.*?($|(?={dontR}))))|(?P<{dontG}>{dontR}(.*?($|(?={doR}))))', superline)
    # match either do() or don't() but in each case the following (any) carachter must stop
    # before next opposite do(nt) or before end, without capturing such.
    # exploit named groups through the prefix ?P<{group_name}>

    # TODO: using findall returns a list of tuples. In each tuple I saw index 0 for 'do group
    #       as well as index 2 for the part following do(), and similarly indexes 4 and 6 for
    #       the don't parts. -> study how indexes are generated, as I do not see more than 5
    #       capturing groups, including the overall match that should have been at index 0

    i = 0                                
    for match in matchDos:
        doMatch = match.groupdict()[doG]

        if doMatch != None:
            values = re.findall(mulR, doMatch)

            for mul in values:
                twoVals = re.findall(valR, mul)
                v1 = int(twoVals[0])
                v2 = int(twoVals[1])

                sum += (v1*v2)

    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {sum}")

if __name__ == "__main__":
    main()