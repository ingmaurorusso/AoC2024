import os
import sys

sys.path.append("..")
from include import AOC

from dataclasses import dataclass


inputExample = """
7 6 4 2 1
1 2 7 8 9
9 7 6 2 1
1 3 2 4 5
8 6 4 4 1
1 3 6 7 9
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
class ManageCheck:
    val: int = 0 # don't care
    oldVal: int = 0 # don't care
    increase: bool = False # don't care
    reinitForSecond: bool = False

    # def __init__(Self):
    #     pass

@dataclass
class ManageRecovery:
    oldVal2: int = 0 # don't care
    oldVal3: int = 0 # don't care
    secondBad: bool = False # don't care
    recoveryVal: int = 0 # don't care
    idxBad: int = 0
    idx: int = 0
    firstBad: bool = True
    

    # def __init__(Self):
    #    pass
    
def check(mc):
    if mc.reinitForSecond:
        mc.increase = (mc.val > mc.oldVal)
        mc.reinitForSecond = False
    
    newInc = (mc.val > mc.oldVal);
    if mc.increase != newInc:
        return False

    if mc.increase:
        diff = mc.val - mc.oldVal
    else:
        diff = mc.oldVal - mc.val

    if diff == 0 or diff > 3:
        return False

    return True


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

    
    # PART 1
    nSafeRports = 0
    
    for report in lines:
        values = report.split()

        if len(values) < 2:
            nSafeRports += 1
            continue
        # else

        m = ManageCheck()

        it = iter(values)
        m.oldVal = int(next(it))
        m.val = int(next(it))
        m.reinitForSecond = True 

        goodReport = True

        while goodReport and m.val != None:
            goodReport = check(m)

            m.oldVal = m.val
            m.val = next(it,None)
            if m.val != None :
                m.val = int(m.val)

        if goodReport:
            nSafeRports += 1

    print(f"Result Part1: {nSafeRports}")


    # PART 2
    nSafeRports = 0
    
    for report in lines:
        values = report.split()

        if len(values) < 2:
            nSafeRports += 1
            continue
        # else

        mc = ManageCheck()
        mr = ManageRecovery()

        it = iter(values)
        mc.oldVal = int(next(it))
        mc.val = int(next(it))
        mc.reinitForSecond = True

        mr.idx = 2
        
        goodReport = True

        while goodReport and mc.val != None:
            goodReport = check(mc)

            if not goodReport and mr.firstBad:
                # three chances: remove 'val' or remove 'oldVal',
                #                or remove 'oldVal2'
                # (_1_)
                # Start removing 'val'.
                mr.recoveryVal = mc.val
                mr.firstBad = False
                mr.secondBad = True
                goodReport = True
                mr.idxBad = mr.idx

                mc.val = next(it,None)
                if mc.val != None:
                    mc.val = int(mc.val)

                if mr.idx == 2: # remember that len > 2
                    mc.reinitForSecond = True

                mr.idx += 1

                continue # avoid update oldVal, as 'val' has been discarded.
            elif not goodReport and (mr.idx == mr.idxBad+1): # and mr.secondBad
                # (_2_)
                newVal = mc.val;
                goodReport = True
                frozenOldVal = mc.oldVal

                # Try replacing oldVal with recoveryVal
                reinitForSecond = False
                if mr.idx > 3:
                    mc.oldVal = mr.oldVal2;
                    mc.val = mr.recoveryVal;
                    if mr.idx == 4:
                        mc.reinitForSecond = True
                      
                    goodReport = check(mc)
                else: # idx == 3
                    mc.reinitForSecond = True

                if goodReport: # not strictly needed
                    mc.oldVal = mr.recoveryVal
                    mc.val = newVal
                    goodReport = check(mc)

                if not goodReport and (mr.idx > 3):
                    # Another chance is to use both
                    # recoveryVal and the previous one,
                    # removing the second previous.
                    goodReport = True
                    if mr.idx > 4: # else: oldVal3 does not exist
                        mc.oldVal = mr.oldVal3
                        mc.val = frozenOldVal
                        goodReport = check(mc)
                    else:
                       mc.reinitForSecond = True

                    if goodReport: # not strictly needed
                        mc.oldVal = frozenOldVal; # = val (if idx > 4)
                        mc.val = mr.recoveryVal
                        goodReport = check(mc)

                    if goodReport: # not strictly needed
                        # useless to set mr.oldVal2, as oldVal3 will be never used again.
                        # mr.oldVal2 = frozenOldVal; // = oldVal
                     
                        mc.oldVal = mr.recoveryVal # = val
                        mc.val = newVal
                        goodReport = check(mc)
            elif not goodReport and (mr.idx == 4) and mr.secondBad:
                newVal = mc.val
                goodReport = True
                frozenOldVal = mc.oldVal;

                # idx==4 => idxBad==2 (cannot be 1 and, if it was 3,
                # the code (_2_) would have been executed).
                # There still is the chance that second_to_third was
                # in opposite order compared to first_to_third, as
                # well as in the same order of third_to_fourth,
                # where the second was the discarded one in (_1_).
                # Note that idx > 4 does not give any chance to
                # switch direction. Moreover, this try does not make sense
                # if the new bad couple third_to_fourth is because
                # of bad distance.

                mc.oldVal = mr.recoveryVal;
                mc.val = frozenOldVal;
                mc.reinitForSecond = True
                goodReport = check(mc)

                if goodReport: # not strictly needed
                   # useless to set oldVal2, as oldVal3 will be never used again.
                   # oldVal2 = recoveryVal; // = oldVal

                   mc.oldVal = frozenOldVal
                   mc.val = newVal
                   goodReport = check(mc)
            
            mr.oldVal3 = mr.oldVal2
            mr.oldVal2 = mc.oldVal
            mc.oldVal = mc.val
            mc.val = next(it,None)
            mr.idx += 1
            if mc.val != None:
                mc.val = int(mc.val)

        if goodReport:
            nSafeRports += 1

    print(f"Result Part2: {nSafeRports}")

if __name__ == "__main__":
    main()