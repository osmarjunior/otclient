#ifndef POSITION_H
#define POSITION_H

#include <framework/util/types.h>

enum Direction
{
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST
};

class Position
{
public:
    Position() : x(-1), y(-1), z(-1) { }
    Position(int x, int y, int z) : x(x), y(y), z(z) { }

    bool isValid() const { return x >= 0 && y >= 0 && z >= 0 && x < 65536 && y < 65536 && z < 15; }

    Position operator+(const Position& other) const { return Position(x + other.x, y + other.y, z + other.z);   }
    Position& operator+=(const Position& other) { x+=other.x; y+=other.y; z +=other.z; return *this; }
    Position operator-(const Position& other) const { return Position(x - other.x, y - other.y, z + other.z);   }
    Position& operator-=(const Position& other) { x-=other.x; y-=other.y; z-=other.z; return *this; }

    Position& operator=(const Position& other) { x = other.x; y = other.y; z = other.z; return *this; }
    bool operator==(const Position& other) const { return other.x == x && other.y == y && other.z == z; }
    bool operator!=(const Position& other) const { return other.x!=x || other.y!=y || other.z!=z; }

    int x;
    int y;
    int z;
};

template<class T>
std::ostream& operator<<(std::ostream& out, const Position& pos)
{
    out << pos.x << " " << pos.y << " " << pos.z;
    return out;
}

template<class T>
std::istream& operator>>(std::istream& in, Position& pos)
{
    in >> pos.x >> pos.y >> pos.z;
    return in;
}

#endif