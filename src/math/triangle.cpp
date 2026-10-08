#include "math/triangle.h"

namespace {

// Two coordinates of a point or an edge on the plane a triangle is projected onto.
struct Projected {
    float a;
    float b;
};

inline float Dot(const Vector3 &first, const Vector3 &second) {
    return ((first.x * second.x) + (first.y * second.y)) + (first.z * second.z);
}

} // namespace

PrnStream &operator<<(PrnStream &stream, CullMode nMode) {
    switch (nMode) {
    case kCullClockwise:
        stream << "CW";
        break;
    case kCullCounterClockwise:
        stream << "CCW";
        break;
    case kCullNone:
        stream << "No";
        break;
    }
    return stream;
}

bool IntersectSegmentTriangle(const Segment &segment,
                              const Triangle &triangle,
                              CullMode nCull,
                              float *pflT) {
    const Vector3 &start = segment.mEnds[0];
    const Vector3 &end = segment.mEnds[1];
    Vector3 direction{end.x - start.x, end.y - start.y, end.z - start.z};
    if (nCull != kCullNone) {
        const float flFacing = Dot(triangle.mNormal, direction);
        if (nCull == kCullClockwise ? (flFacing > 0.0F) : (flFacing < 0.0F)) {
            return false;
        }
    }

    const Vector3 toOrigin{
        triangle.mOrigin.x - start.x, triangle.mOrigin.y - start.y, triangle.mOrigin.z - start.z};
    const float flT = Dot(toOrigin, triangle.mNormal) / Dot(direction, triangle.mNormal);
    *pflT = flT;
    if (flT < 0.0F || flT > 1.0F) {
        return false;
    }

    const Vector3 hit{(start.x + (direction.x * flT)) - triangle.mOrigin.x,
                      (start.y + (direction.y * flT)) - triangle.mOrigin.y,
                      (start.z + (direction.z * flT)) - triangle.mOrigin.z};
    const Vector3 &edge1 = triangle.mEdge1;
    const Vector3 &edge2 = triangle.mEdge2;

    Projected point{hit.x, hit.y};
    Projected first{edge1.x, edge1.y};
    Projected second{edge2.x, edge2.y};
    float flDeterminant = (second.a * first.b) - (first.a * second.b);
    if (flDeterminant == 0.0F) {
        point = Projected{hit.x, hit.z};
        first = Projected{edge1.x, edge1.z};
        second = Projected{edge2.x, edge2.z};
        flDeterminant = (second.a * first.b) - (first.a * second.b);
        if (flDeterminant == 0.0F) {
            point = Projected{hit.y, hit.z};
            first = Projected{edge1.y, edge1.z};
            second = Projected{edge2.y, edge2.z};
            flDeterminant = (second.a * first.b) - (first.a * second.b);
        }
    }

    const float flV = ((point.a * first.b) - (first.a * point.b)) / flDeterminant;
    if (flV < 0.0F || flV > 1.0F) {
        return false;
    }
    float flU;
    if (first.a == 0.0F) {
        flU = (point.b - (second.b * flV)) / first.b;
    } else {
        flU = (point.a - (second.a * flV)) / first.a;
    }
    if (flU < 0.0F) {
        return false;
    }
    return !(flU + flV > 1.0F);
}
