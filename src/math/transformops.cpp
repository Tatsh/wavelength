#include "math/transformops.h"

#include <math.h>

#include "math/sine.h"
#include "math/vector3.h"

namespace {

// A rotation matrix is three rows of four floats and an affine transform is four. A row index
// scales by this stride in both cases.
constexpr int kMatRowStride = 4;

// A quarter turn. The decomposition reports this as the X angle at the gimbal lock limit.
constexpr float kQuarterTurn = 1.570796251f;

// The quarter turn SinApprox() is advanced by to give a cosine.
constexpr float kQuarterTurnApprox = 1.57079637f;

// The X row component above which the Y and Z angles cannot be separated.
constexpr float kGimbalLockLimit = 0.9999998808f;

} // namespace

void Rnd::MakeRotMatrix(const float *pAngles, float *pMat3Rows) {
    const float flSinZ = SinApprox(pAngles[2]);
    const float flCosZ = SinApprox(pAngles[2] + kQuarterTurnApprox);
    const float flSinY = SinApprox(pAngles[1]);
    const float flCosY = SinApprox(pAngles[1] + kQuarterTurnApprox);
    const float flSinX = SinApprox(pAngles[0]);
    const float flCosX = SinApprox(pAngles[0] + kQuarterTurnApprox);

    pMat3Rows[6] = flSinX;
    pMat3Rows[10] = flCosY * flCosX;
    pMat3Rows[2] = -flSinY * flCosX;
    pMat3Rows[4] = -flCosX * flSinZ;
    pMat3Rows[1] = (flCosY * flSinZ) + ((flCosZ * flSinY) * flSinX);
    pMat3Rows[5] = flCosX * flCosZ;
    pMat3Rows[8] = (flCosZ * flSinY) + ((flCosY * flSinZ) * flSinX);
    pMat3Rows[9] = (flSinY * flSinZ) - ((flCosY * flCosZ) * flSinX);
    pMat3Rows[0] = (flCosY * flCosZ) - ((flSinY * flSinZ) * flSinX);
}

void Mat33BuildOrthonormal(const float *pAxisY, const float *pReference, float *pMat3Rows) {
    pMat3Rows[4] = pAxisY[0];
    pMat3Rows[5] = pAxisY[1];
    pMat3Rows[6] = pAxisY[2];
    pMat3Rows[7] = pAxisY[3];
    Vec3Normalize(&pMat3Rows[4], &pMat3Rows[4]);

    // The cross products are an inline vopmula and vopmsub pair rather than a call.
    Vector3 axisX;
    axisX.x = (pMat3Rows[5] * pReference[2]) - (pMat3Rows[6] * pReference[1]);
    axisX.y = (pMat3Rows[6] * pReference[0]) - (pMat3Rows[4] * pReference[2]);
    axisX.z = (pMat3Rows[4] * pReference[1]) - (pMat3Rows[5] * pReference[0]);
    axisX.w = pMat3Rows[7];
    Vec3Normalize(&axisX.x, &axisX.x);

    pMat3Rows[0] = axisX.x;
    pMat3Rows[1] = axisX.y;
    pMat3Rows[2] = axisX.z;
    pMat3Rows[3] = axisX.w;

    pMat3Rows[8] = (pMat3Rows[1] * pMat3Rows[6]) - (pMat3Rows[2] * pMat3Rows[5]);
    pMat3Rows[9] = (pMat3Rows[2] * pMat3Rows[4]) - (pMat3Rows[0] * pMat3Rows[6]);
    pMat3Rows[10] = (pMat3Rows[0] * pMat3Rows[5]) - (pMat3Rows[1] * pMat3Rows[4]);
    pMat3Rows[11] = pMat3Rows[3];
}

void Rnd::Scale(const float *pScale, const float *pMat3Rows, float *pOut) {
    pOut[0] = pMat3Rows[0] * pScale[0];
    pOut[1] = pMat3Rows[1] * pScale[0];
    pOut[2] = pMat3Rows[2] * pScale[0];

    pOut[4] = pMat3Rows[4] * pScale[1];
    pOut[5] = pMat3Rows[5] * pScale[1];
    pOut[6] = pMat3Rows[6] * pScale[1];

    pOut[8] = pMat3Rows[8] * pScale[2];
    pOut[9] = pMat3Rows[9] * pScale[2];
    pOut[10] = pMat3Rows[10] * pScale[2];
}
