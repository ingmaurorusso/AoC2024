import os
import sys

from enum import Enum
from functools import cmp_to_key

sys.path.append("..")
from include import AOC


inputExample = """
47|53
97|13
97|61
97|47
75|29
61|13
75|53
29|13
97|29
53|29
61|53
97|53
61|29
47|13
75|47
97|75
47|61
75|61
47|29
75|13
53|13

75,47,61,53,29
97,61,53,29,13
75,29,13
75,97,47,61,53
61,13,29
97,13,75,29,47
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


def checkOrdered(update, withPredecessors, reorderIfNot):

    goodUpdate = True
    for i in range(len(update)-1):
        if update[i] in withPredecessors.keys():
            for j in range(i+1,len(update)):
                if update[j] in withPredecessors[update[i]]:
                    goodUpdate = False
                    break
            if not goodUpdate:
                break

    if goodUpdate:
        return True
    
    if not reorderIfNot:
        return False
    
    # generate labels
    labels = dict()

    # suppose there is a unique ordering: for each node compute the
    # max distance from at least a sink or a source. Each max distance
    # (all from sources or all from sinks) must be assigned to a
    # unique node.
    filteredSucc = dict()
    filteredPred = dict()

    # filter rules
    for prev in withPredecessors.keys():
        for succ in withPredecessors[prev]:
            if prev in update and succ in update:
                if prev in filteredPred.keys():
                    filteredPred[prev].append(succ)
                else:
                    filteredPred.update({prev: [succ]})

                if succ in filteredSucc.keys():
                    filteredSucc[succ].append(prev)
                else:
                    filteredSucc.update({succ: [prev]})

    # look for a single soruce or a single sink
    doForward = True
    begin = None

    for elem in update:
        if not(elem in filteredPred):
            if begin != None:
                begin = None # to push repeat cycle
                break
            else:
                begin = elem

    if begin == None:
        raise ValueError('Circular ordering for the update (no source): ' + update)

    # compute max distances
    maxDistance = dict()
    maxDistance[begin] = 1

    propagate = [begin]

    while len(propagate) > 0:
        newPropagate = []

        for node in propagate:
            nodeDist = maxDistance[node]

            if node in filteredSucc.keys():
                for nextNode in filteredSucc[node]:
                    doPropagate = True
                    if nextNode in maxDistance.keys():
                        maxDist = maxDistance[nextNode]
                        if maxDist > nodeDist:
                            doPropagate = False

                    if doPropagate:
                        maxDistance[nextNode] = nodeDist+1
                        newPropagate.append(nextNode)
                        if nodeDist >= len(update):
                            raise ValueError(f'Circular ordering for the update (node{node}): ' + update)

        propagate = newPropagate

    if len(maxDistance) != len(update):
        raise ValueError('Non-connected update: ' + update)

    checkUniqueDist = []
    for elem in update:
        maxDist = maxDistance[elem]
        if maxDist in checkUniqueDist:
            raise ValueError('Non-unique ordering in: ' + update)
        checkUniqueDist.append(maxDist)

    update.sort(key=lambda x: maxDistance[x], reverse=doForward)

    return False
    


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

    okOneEmpty = True # between rules and updates
    while('' in lines):
        if not okOneEmpty:
            print(f'an empty line: idx {lines.index('')+1}')
        okOneEmpty = False
        lines.remove('')

    if len(lines) == 0:
        print('No lines or only empty lines!')
        exit()

    sum1 = 0
    sum2 = 0

    # PART 1 and 2 together

    withPredecessors = dict()

    readingRules = True
    for line in lines:
        if "|" in line:
            # another rule
            if not readingRules:
                raise ValueError('A rule after that updates began')
        
            values = line.split('|')
            if len(values) != 2:
                raise ValueError('unexcpected line-rule format: ' + line)

            v1 = int(values[0])
            v2 = int(values[1])

            if v2 in withPredecessors.keys():
                withPredecessors[v2].append(v1)
            else:
                withPredecessors.update({v2: [v1]})
        else:
            if len(withPredecessors) == 0:
                raise ValueError('updates before any rule: ' + line)

            readingRules = False

            # an update
            update = [int(v) for v in line.split(',')]

            if len(update)%2 == 0:
                raise ValueError('line-update with an even number of values: ' + line)

            if checkOrdered(update,withPredecessors,True):
                sum1 += update[len(update)//2]
            else:
                sum2 += update[len(update)//2]


    if len(withPredecessors) == 0:
        print('WARNING: no predecessors rules')

    print(f"Line count: {len(lines)}")
    print(f"Result Part1: {sum1}")

    # PART 2

    print(f"Line count: {len(lines)}")
    print(f"Result Part2: {sum2}")


if __name__ == "__main__":
    main()