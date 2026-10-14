#include "math/plane.h"

PrnStream &operator<<(PrnStream &stream, const Plane &plane) {
    stream << "(a:" << plane.a << " b:" << plane.b << " c:" << plane.c << " d:" << plane.d << ")";
    return stream;
}
