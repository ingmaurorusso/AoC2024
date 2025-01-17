# import more-itertools
# need:
# sudo apt install pip
# sudo apt install pipx
# python3 -m pip install more-itertools
# mimic below (see https://docs.python.org/3/library/itertools.html)
import collections
import re

# import sys
# sys.path.append(".")
# import .Space_MR

def extractDayLabel(mainArgs, mainParentDir):
    deduce = True;
    if len(mainArgs) >= 2:
        dayLabel = mainArgs[1].strip()
        deduce = len(dayLabel) == 0

    if deduce:
        # try to deduce from working directory

        dayLabel = re.search(r'\d+', mainParentDir)

        if dayLabel == None:
            print("Use one parameter (only): day label, or use day-label as first digits appearing in parent dir")
            return None

        dayLabel = dayLabel[0]
        
    
    return dayLabel


def count(start=0, step=1): # mimic more-itertools
    # count(10) → 10 11 12 13 14 ...
    # count(2.5, 0.5) → 2.5 3.0 3.5 ...
    n = start
    while True:
        yield n
        n += step

def islice(iterable, *args): # mimic more-itertools
    # islice('ABCDEFG', 2) → A B
    # islice('ABCDEFG', 2, 4) → C D
    # islice('ABCDEFG', 2, None) → C D E F G
    # islice('ABCDEFG', 0, None, 2) → A C E G

    s = slice(*args)
    start = 0 if s.start is None else s.start
    stop = s.stop
    step = 1 if s.step is None else s.step
    if start < 0 or (stop is not None and stop < 0) or step <= 0:
        raise ValueError

    indices = count() if stop is None else range(max(start, stop))
    next_i = start
    for i, element in zip(indices, iterable):
        if i == next_i:
            yield element
            next_i += step

def consume(iterator, n=None): # mimic more-itertools
    "Advance the iterator n-steps ahead. If n is None, consume entirely."
    # Use functions that consume iterators at C speed.
    if n is None:
        collections.deque(iterator, maxlen=0)
    else:
        next(islice(iterator, n, n), None)

# from https://stackoverflow.com/questions/51410881/python-equivalent-of-c-find-if
def first_match(iterable, predicate, defaultIfAbsent):
    try:
        return next(idx for idx,n in enumerate(iterable) if predicate(n))
    except StopIteration:
        return defaultIfAbsent
    