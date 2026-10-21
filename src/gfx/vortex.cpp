#include "gfx/vortex.h"

#include <cmath>

#include "gfx/unreadvector.h"
#include "math/transformops.h"
#include "math/vector3.h"
#include "os/joypad.h"
#include "os/system.h"
#include "rnd/rndmanager.h"

namespace {

// The frames of mFrame and mAmbientFrame before the first run.
constexpr float kNoFrame = -1.0f;

// The pad whose left stick tilts the vortex.
constexpr int kStickPad = 0;

// The frames the wiggle animation advances for a full stick.
constexpr float kWiggleFrames = 1000.0f;

// The rows of a basis.
constexpr int kBasisRows = 3;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

// Find the view an entry of the configuration identifies.
RndView *FindView(DataArray *pConfig, const char *pszEntry) {
    const char *pszName;
    pConfig->FindSymbol(pszEntry, &pszName, true);
    return FindObject<RndView>(pszName);
}

} // namespace

// The static initialiser of the unit, and the routine that runs it.
// NTSC-U/C: 0x001e55b0, PAL: 0x001ee350
// NTSC-U/C: 0x001e55e8, PAL: 0x001ee388

Vortex::Vortex(const char *pszName)
    : mTopView(nullptr), mTransitionView(nullptr), mAmbientView(nullptr), mMode(kModeIdle),
      mPrevMode(kModeIdle), mInterp(0.0f, 0.0f, 0.0f, 1.0f) {
    mAmbientFrame = kNoFrame;
    mStartMs = kNotStarted;
    mFrame = kNoFrame;
    mOutroFrames = Vector2{0.0f, 0.0f};
    mInFrames = Vector2{0.0f, 0.0f};
    mOutFrames = Vector2{0.0f, 0.0f};
    mStick.y = 0.0f;
    mStick.x = 0.0f;
    mConfig = SystemConfig()
                  ->FindArray("gfx", true)
                  ->FindArray("vortices", true)
                  ->FindArray(pszName, true);
    mConfig->AddRef();
    mConfig->FindVector("motion_range", &mMotionRange, true);
    mConfig->FindVector("black_frames", &mOutroFrames, false);
    mConfig->FindVector("start_frames", &mInFrames, true);
    mConfig->FindVector("end_frames", &mOutFrames, true);
    Load();
}

Vortex::~Vortex() {
    if (mConfig != nullptr) {
        mConfig->Release();
    }
    Unload();
}

void Vortex::Load() {
    const char *pszFile;
    mConfig->FindSymbol("rnd", &pszFile, true);
    mLoader = TheManager.AddLoader(pszFile, RndLoader::kDeleteObjects, nullptr, nullptr);
    mTopView = FindView(mConfig, "top_view");
    mWiggle = FindView(mConfig, "wiggle");
    mWiggleXAnim = FindView(mConfig, "wiggle_x_anim");
    mWiggleYAnim = FindView(mConfig, "wiggle_y_anim");
    mTransitionView = FindView(mConfig, "transition_view");
    mAmbientView = FindView(mConfig, "ambient_view");
}

void Vortex::Unload() {
    mMode = kModeIdle;
    mAmbientView = nullptr;
    mTransitionView = nullptr;
    mTopView = nullptr;
    mWiggle = nullptr;
    mLoader.reset();
}

float Vortex::GetOutroLength() const {
    return std::fabs(mOutroFrames.y - mOutroFrames.x);
}

float Vortex::GetInLength() const {
    return std::fabs(mInFrames.y - mInFrames.x);
}

float Vortex::GetOutLength() const {
    return std::fabs(mOutFrames.y - mOutFrames.x);
}

void Vortex::Start(float fLength, int eMode) {
    mMode = eMode;
    switch (eMode) {
    case kModeOutro: {
        mStartMs = kNotStarted;
        const float fRun = (fLength < 0.0f) ? GetOutroLength() : fLength;
        mInterp.Reset(mOutroFrames.x, mOutroFrames.y, 0.0f, fRun);
        break;
    }
    case kModeIn: {
        mStartMs = kNotStarted;
        const float fRun = (fLength < 0.0f) ? GetInLength() : fLength;
        mInterp.Reset(mInFrames.x, mInFrames.y, 0.0f, fRun);
        break;
    }
    case kModeOut: {
        const float fNow = SystemMs();
        const float fRun = (fLength < 0.0f) ? GetOutLength() : fLength;
        mInterp.Reset(mOutFrames.x, mOutFrames.y, fNow, fNow + fRun);
        break;
    }
    default:
        break;
    }
}

void Vortex::Poll() {
    mPrevMode = mMode;
    const float fNow = SystemMs();
    if (mStartMs == kNotStarted) {
        mStartMs = fNow;
        mInterp.Reset(
            mInterp.GetY0(), mInterp.GetY1(), fNow, fNow + (mInterp.GetX1() - mInterp.GetX0()));
    }
    if (mMode != kModeHold) {
        if ((mMode <= kModeIdle) || ((mMode > kModeHold) && (mMode != kModeOut))) {
            return;
        }
        if (mInterp.GetX1() < fNow) {
            mFrame = mInterp.GetY1();
            if (mMode == kModeIn) {
                mMode = kModeHold;
            } else if (mMode != kModeOutro) {
                mMode = kModeIdle;
            }
        } else {
            mFrame = mInterp.Interp(fNow);
        }
        if (mTransitionView != nullptr) {
            mTransitionView->SetFrame(mFrame);
        }
    }
    mAmbientFrame = fNow - mStartMs;
    if (mAmbientView != nullptr) {
        mAmbientView->SetFrame(mAmbientFrame);
    }
}

void Vortex::Draw() {
    if ((mMode == kModeIdle) || (mTopView == nullptr)) {
        return;
    }
    const JoypadStick &stick = JoypadGetState(kStickPad)->mSticks[0];
    mStick = Vector2{stick.mX, stick.mY};
    const Vector3 up{mStick.x * mMotionRange.x, 1.0f, mStick.y * mMotionRange.y};
    const Vector3 forward{0.0f, 0.0f, 1.0f};
    Vector3 basis[kBasisRows];
    Mat33BuildOrthonormal(&up.x, &forward.x, &basis[0].x);
    mWiggle->mLocalXfm.mBasisX = basis[0];
    mWiggle->mLocalXfm.mBasisY = basis[1];
    mWiggle->mLocalXfm.mBasisZ = basis[2];
    mWiggle->mDirty = 1;
    // Yes, the binary drives the X animation with both axes and never the Y animation.
    mWiggleXAnim->SetFrame(mStick.x * kWiggleFrames);
    mWiggleXAnim->SetFrame(mStick.y * kWiggleFrames);
    mTopView->UpdateWorldXfm(nullptr, 0);
    mTopView->Draw();
}
