import os
import re
import sys

from dataclasses import dataclass
from enum import Enum

sys.path.append("..")
from include import AOC


inputExamples = ["""
kh-tc
qp-kh
de-cg
ka-co
yn-aq
qp-ub
cg-tb
vc-aq
tb-ka
wh-tc
yn-cg
kh-ub
ta-co
de-co
tc-td
tb-wq
wh-td
ta-ka
td-qp
aq-cg
wq-ub
ub-vc
de-ta
wq-aq
wq-vc
wh-yn
ka-de
kh-ta
co-tc
wh-qp
tb-vc
td-yn
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

    successors = dict()

    for line in lines:
        pcs = line.split('-')

        for i in range(2):
            if not pcs[i] in successors.keys():
                successors.update({pcs[i]: set()})

            successors[pcs[i]].add(pcs[1-i])

    # PART 1

    n1 = 0
    n2 = 0
    n3 = 0

    for pc, connectedPcs in successors.items():
        if pc[0] != 't':
            continue

        for pc2 in connectedPcs:
            for pc3 in connectedPcs:
                if pc2 == pc3:
                    continue
            
                if pc3 in successors[pc2]:
                    t2 = (pc2[0] == 't')
                    t3 = (pc3[0] == 't')

                    if t2 and t3:
                        n3 += 1
                    elif t2 or t3:
                        n2 += 1
                    else:
                        n1 += 1

    count1 = (n1 + n2//2 + n3//3) // 2

    print(f"Result Part1: {count1}")


    # PART 2

    best = 0
    bestClique = set()
    for pc, connectedPcs in successors.items():
        bestCliqueForPc = set()

        ub = len(connectedPcs)+1

        for pc2 in connectedPcs:
            if ub <= best:
                break # speed-up, useless to continue

            ok = True

            for pcClique in bestCliqueForPc:
                if not (pcClique in successors[pc2]):
                    ok = False
                    ub -= 1
                    break

            if ok:
                bestCliqueForPc.add(pc2)
        
        bestCliqueForPc.add(pc)

        if len(bestCliqueForPc) > best:
            best = len(bestCliqueForPc)
            bestClique = bestCliqueForPc

    bestCliqueList = list()
    for pc in bestClique:
        bestCliqueList.append(pc+',')

    bestCliqueList.sort()
    
    pwd = "".join(bestCliqueList)[:-1]

    print(f"Result Part2: {pwd}")


if __name__ == "__main__":
    main()