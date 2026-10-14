#include "gfx/meshrotation.h"

#include <vector>

#include "math/key.h"
#include "math/sine.h"
#include "math/vector3.h"

namespace {

constexpr float kHalfPi = 1.57079637f;

using PointsKey = Key<std::vector<Vector3>>;

} // namespace

void AddRotationKeys(RndMesh *pMesh,
                     RndMeshAnim *pAnim,
                     int nKeys,
                     float flStartFrame,
                     float flStartAngle,
                     float flEndFrame,
                     float flEndAngle) {
    std::vector<PointsKey> &keys = pAnim->mKeysOwner->mVertPointsKeys;
    keys.clear();
    const std::vector<RndMesh::Vert> &verts = pMesh->mGeomOwner->mVerts;
    for (int i = 0; i < nKeys; ++i) {
        const float flT = static_cast<float>(i) / static_cast<float>(nKeys - 1);
        const float flAngle = ((flEndAngle - flStartAngle) * flT) + flStartAngle;
        const float flFrame = ((flEndFrame - flStartFrame) * flT) + flStartFrame;
        const float flCos = SinApprox(flAngle + kHalfPi);
        const float flSin = SinApprox(flAngle);
        const PointsKey *pPrev;
        const PointsKey *pNext;
        float flRatio;
        const int nIndex = AtFrame(keys, flFrame, pPrev, pNext, flRatio);
        const auto key = keys.insert(keys.begin() + nIndex, PointsKey{{}, flFrame});
        key->value.resize(verts.size());
        for (std::size_t j = 0; j < verts.size(); ++j) {
            const Vector3 &point = verts[j].mPos;
            key->value[j] = Vector3{(flCos * point.x) - (flSin * point.y),
                                    (flSin * point.x) + (flCos * point.y),
                                    point.z,
                                    point.w};
        }
    }
}
