#include "gfx/tnlgemcapfx.h"

#include "game/gamedb.h"
#include "math/rand.h"
#include "math/sine.h"

namespace {

// The track of an idle burst.
constexpr char kIdle = -1;

// The turns of a burst are multiples of a quarter of kPi, drawn below this count.
constexpr int kTurnChoices = 7;

// The quarter turns the angle of a burst advances per period of GfxTunnel::BurstSpinPeriod().
constexpr float kTurnsPerPeriod = 8.0f;

constexpr float kPi = 3.14159274f;
constexpr float kHalfPi = 1.57079637f;
constexpr float kQuarter = 0.25f;

// A burst ends once it falls this many ticks behind the song.
constexpr float kTrailTicks = 960.0f;

// Apply the basis of a transform to a row.
inline Vector3 RotateRow(const Transform &xfm, const Vector3 &row) {
    return Vector3{(xfm.mBasisX.x * row.x) + (xfm.mBasisY.x * row.y) + (xfm.mBasisZ.x * row.z),
                   (xfm.mBasisX.y * row.x) + (xfm.mBasisY.y * row.y) + (xfm.mBasisZ.y * row.z),
                   (xfm.mBasisX.z * row.x) + (xfm.mBasisY.z * row.y) + (xfm.mBasisZ.z * row.z),
                   row.w};
}

} // namespace

float TnlGemCapFX::sAnimFrames = 1000.0f;
float TnlGemCapFX::sLengths[2] = {1000.0f, 333.0f};
float TnlGemCapFX::sTravels[2] = {360.0f, 0.0f};

TnlGemCapFX::TnlGemCapFX()
    : mView(nullptr), mLateral(-1.0f), mFrameCurve(0.0f, 0.0f, 0.0f, 1.0f),
      mTickCurve(0.0f, 0.0f, 0.0f, 1.0f) {
    mTrack = kIdle;
}

TnlGemCapFX::~TnlGemCapFX() = default;

void TnlGemCapFX::Start(GfxTunnel *,
                        char nTrack,
                        Rnd::View *pView,
                        float flTick,
                        float flLateral,
                        const Vector3 *pScale,
                        bool bErase) {
    const int nKind = bErase ? 1 : 0;
    mTrack = nTrack;
    mView = pView;
    mLateral = flLateral;
    const float flNow = TheGameDb->mSongTime;
    mFrameCurve.Reset(0.0f, sAnimFrames, flNow, flNow + sLengths[nKind]);
    mTickCurve.Reset(flTick, flTick + sTravels[nKind], flNow, mFrameCurve.mX1);

    const float flPeriod = GfxTunnel::BurstSpinPeriod();
    const int nTurn = RandomInt(0, kTurnChoices);
    const float flAngle =
        ((((flNow * kTurnsPerPeriod) / flPeriod) + static_cast<float>(nTurn)) * kPi) * kQuarter;
    const float flCos = SinApprox(flAngle + kHalfPi);
    const float flSin = SinApprox(flAngle);
    mSpin.mBasisX.x = flCos;
    mSpin.mBasisX.z = 0.0f;
    mSpin.mBasisX.y = flSin;
    mSpin.mBasisY.x = -flSin;
    mSpin.mBasisY.y = flCos;
    mSpin.mBasisY.z = 0.0f;
    mSpin.mBasisZ.x = 0.0f;
    mSpin.mBasisZ.y = 0.0f;
    mSpin.mBasisZ.z = 1.0f;
    Vector3 *rows[] = {&mSpin.mBasisX, &mSpin.mBasisY, &mSpin.mBasisZ};
    const float scales[] = {pScale->x, pScale->y, pScale->z};
    for (int i = 0; i < 3; ++i) {
        rows[i]->x *= scales[i];
        rows[i]->y *= scales[i];
        rows[i]->z *= scales[i];
    }
}

void TnlGemCapFX::Stop() {
    if (mTrack >= 0) {
        mTrack = kIdle;
    }
}

bool TnlGemCapFX::Draw(TnlGeom *pGeom) {
    if (mTrack < 0) {
        return false;
    }
    const float flNow = TheGameDb->mSongTime;
    if (mFrameCurve.mX1 <= flNow) {
        mTrack = kIdle;
        return true;
    }
    if ((flNow - mFrameCurve.mX0) <= 0.0f) {
        return false;
    }
    const float flTick = mTickCurve.Interp(flNow);
    if (kTrailTicks < (TheGameDb->mSongTick - flTick)) {
        mTrack = kIdle;
        return true;
    }
    pGeom->CellXfm(mTrack, &mXfm, true, true, flTick, mLateral);
    const Vector3 axisX = RotateRow(mXfm, mSpin.mBasisX);
    const Vector3 axisY = RotateRow(mXfm, mSpin.mBasisY);
    const Vector3 axisZ = RotateRow(mXfm, mSpin.mBasisZ);
    mXfm.mBasisX = axisX;
    mXfm.mBasisY = axisY;
    mXfm.mBasisZ = axisZ;
    mView->SetLocalXfm(mXfm);
    mView->SetFrame(mFrameCurve.Interp(flNow));
    mView->UpdateWorldXfm(nullptr, 0);
    mView->Draw();
    return false;
}

bool TnlGemCapFX::IsIdle() const {
    return mTrack < 0;
}
