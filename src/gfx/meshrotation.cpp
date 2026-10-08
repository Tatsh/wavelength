#include "gfx/meshrotation.h"

#include <iterator>
#include <list>
#include <vector>

#include "math/sine.h"
#include "math/vector3.h"

namespace {

constexpr float kHalfPi = 1.57079637f;

using PointsKeys = std::list<Rnd::MeshAnim::PointsKey>;

// Find where a key of a frame goes among keys sorted by frame, after any key of the same frame
// except the last.
inline PointsKeys::iterator KeyPosition(PointsKeys &keys, float flFrame) {
    if (keys.empty() || (flFrame <= keys.front().mFrame)) {
        return keys.begin();
    }
    if (keys.back().mFrame <= flFrame) {
        return (flFrame == keys.back().mFrame) ? std::prev(keys.end()) : keys.end();
    }
    int nLow = 0;
    int nHigh = static_cast<int>(keys.size()) - 1;
    while ((nLow + 1) < nHigh) {
        const int nMiddle = (nLow + nHigh) >> 1;
        const float flMiddle = std::next(keys.begin(), nMiddle)->mFrame;
        if (flFrame == flMiddle) {
            return std::next(keys.begin(), nMiddle);
        }
        if (flMiddle < flFrame) {
            nLow = nMiddle;
        } else {
            nHigh = nMiddle;
        }
    }
    return std::next(keys.begin(), nHigh);
}

} // namespace

void AddRotationKeys(Rnd::Mesh *pMesh,
                     Rnd::MeshAnim *pAnim,
                     int nKeys,
                     float flStartFrame,
                     float flStartAngle,
                     float flEndFrame,
                     float flEndAngle) {
    PointsKeys &keys = pAnim->mKeysOwner->mVertPointsKeys;
    keys.clear();
    const std::vector<Rnd::MeshVert> &verts = pMesh->mVertsOwner->mVerts;
    for (int i = 0; i < nKeys; ++i) {
        const float flT = static_cast<float>(i) / static_cast<float>(nKeys - 1);
        const float flAngle = ((flEndAngle - flStartAngle) * flT) + flStartAngle;
        const float flFrame = ((flEndFrame - flStartFrame) * flT) + flStartFrame;
        const float flCos = SinApprox(flAngle + kHalfPi);
        const float flSin = SinApprox(flAngle);
        const PointsKeys::iterator key =
            keys.insert(KeyPosition(keys, flFrame), Rnd::MeshAnim::PointsKey{{}, flFrame});
        key->mValues.resize(verts.size());
        for (std::size_t j = 0; j < verts.size(); ++j) {
            const Vector3 &point = verts[j].mPoint;
            key->mValues[j] = Vector3{(flCos * point.x) - (flSin * point.y),
                                      (flSin * point.x) + (flCos * point.y),
                                      point.z,
                                      point.w};
        }
    }
}
