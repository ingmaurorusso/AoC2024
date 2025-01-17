from dataclasses import dataclass
from enum import Enum

class Direction4(Enum):
    Up = 0
    Right = 1
    Down = 2
    Left = 3

def dir4ToArrow(dir4, revertYfor2dField = True):
    if dir4 == Direction4.Up:
        return '^' if not revertYfor2dField else 'v'
    if dir4 == Direction4.Right:
        return '>'
    if dir4 == Direction4.Down:
        return 'v' if not revertYfor2dField else '^'
    if dir4 == Direction4.Left:
        return '<'
    raise ValueError('Inconsistent direction')


def dir4FromArrow(dirChar, revertYfor2dField = True):
    # not very performant...
    for dir4 in Direction4:
        if dir4ToArrow(dir4) == dirChar:
            return dir4;

    raise ValueError('Inconsistent arrow')


def rotateDir4(dir4, clockwise, steps = 1, revertYfor2dField = True):
    d = int(dir4.value)

    if revertYfor2dField:
        clockwise = not clockwise
    
    if clockwise:
        d += steps
    else:
        d += (steps*3)
    
    d %= 4

    return Direction4(d)


@dataclass
class Point:
    x: int
    y: int

    def __hash__(self):
        return self.x*123 + self.y
    
    def __eq__(self, other):
        if other == None: return False
        return self.x == other.x and self.y == other.y

    def copy(self):
        return Point(self.x,self.y)


@dataclass
class Field2D:
    lines: list

    def __init__(Self, lines):
        for i in range(len(lines)):
            if len(lines[i]) != len(lines[0]):
                raise ValueError('lines with different lengths')

        Self.lines = lines.copy()


    def getP(Self, point):
        return Self.lines[point.y][point.x]
    def setP(Self, point, ch):
        line = Self.lines[point.y]
        Self.lines[point.y] = line[:point.x] + str(ch) + line[point.x+1:]

    def getC(Self, coordX, coordY):
        return Self.lines[coordY][coordX]
    def setC(Self, coordX, coordY, ch):
        line = Self.lines[coordY];
        Self.lines[coordY] = line[:coordX] + str(ch) + line[coordX+1:]

    def dimX(Self):
        return len(Self.lines[0]) if len(Self.lines) > 0 else 0

    def dimY(Self):
        return len(Self.lines)
    
    def movePoint(Self, point, dir4):
        if dir4 == Direction4.Up:
            if point.y < Self.dimY()-1:
                point.y += 1
                return True
        elif dir4 == Direction4.Right:
            if point.x < Self.dimX()-1:
                point.x += 1
                return True
        elif dir4 == Direction4.Down:
            if point.y > 0:
                point.y -= 1
                return True
        elif dir4 == Direction4.Left:
            if point.x > 0:
                point.x -= 1
                return True
        else:
            raise ValueError('incorrect direction')
    
        return False