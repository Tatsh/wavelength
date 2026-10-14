#include "math/sphere.h"

#include "math/plane.h"

PrnStream &operator<<(PrnStream &stream, const Sphere &sphere) {
    stream << "\n\tcenter:" << sphere.mCenter << " radius:" << sphere.mRadius;
    return stream;
}

bool IntersectSegmentSphere(const Segment &segment, const Sphere &sphere, float *pflT) {
    const Vector3 &start = segment.mEnds[0];
    const Vector3 &end = segment.mEnds[1];
    const float flDx = end.x - start.x;
    const float flDy = end.y - start.y;
    const float flDz = end.z - start.z;
    const float flToCenter =
        ((flDx * (sphere.mCenter.x - start.x)) + (flDy * (sphere.mCenter.y - start.y))) +
        (flDz * (sphere.mCenter.z - start.z));
    const float flT = flToCenter / (((flDx * flDx) + (flDy * flDy)) + (flDz * flDz));
    *pflT = flT;
    const float flOffX = (start.x + (flDx * flT)) - sphere.mCenter.x;
    const float flOffY = (start.y + (flDy * flT)) - sphere.mCenter.y;
    const float flOffZ = (start.z + (flDz * flT)) - sphere.mCenter.z;
    const float flDistanceSquared = ((flOffX * flOffX) + (flOffY * flOffY)) + (flOffZ * flOffZ);
    return !(sphere.mRadius * sphere.mRadius < flDistanceSquared);
}
