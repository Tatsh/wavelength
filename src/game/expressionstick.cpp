#include "game/expressionstick.h"

#include "os/system.h"

namespace {

// The speed a held directional button moves the position at, in units per millisecond.
constexpr float kButtonSpeed = 0.002f;

constexpr float kMaxPosition = 1.0f;
constexpr float kMinPosition = -1.0f;

float ClampPosition(float fPosition) {
    if (kMaxPosition < fPosition) {
        return kMaxPosition;
    }
    if (fPosition < kMinPosition) {
        return kMinPosition;
    }
    return fPosition;
}

} // namespace

ExpressionStick::ExpressionStick(int nPad, int nStick)
    : mX(0.0f), mY(0.0f), mSpeedX(0.0f), mSpeedY(0.0f), mLastStick{0.0f, 0.0f}, mLastPollMs(0.0f) {
    mPad = nPad;
    mStick = nStick;
    JoypadAddSink(this);
}

ExpressionStick::~ExpressionStick() {
    JoypadRemoveSink(this);
}

bool ExpressionStick::Poll() {
    const JoypadStick stick = JoypadGetState(mPad)->mSticks[mStick];
    const bool bMoved = !((mLastStick.mX == stick.mX) && (mLastStick.mY == stick.mY));
    const float fNowMs = SystemMs();
    if (bMoved) {
        mX = stick.mX;
        mY = stick.mY;
        mSpeedX = 0.0f;
        mSpeedY = 0.0f;
    } else {
        const float fElapsedMs = fNowMs - mLastPollMs;
        mX = ClampPosition(mX + (mSpeedX * fElapsedMs));
        mY = ClampPosition(mY + (mSpeedY * fElapsedMs));
    }
    mLastStick = stick;
    mLastPollMs = fNowMs;
    return bMoved || (mSpeedX != 0.0f) || (mSpeedY != 0.0f);
}

void ExpressionStick::Reset() {
    mY = 0.0f;
    mSpeedX = 0.0f;
    mSpeedY = 0.0f;
    mX = 0.0f;
}

bool ExpressionStick::HandleButton(JoypadInputMsg *pMsg) {
    if (pMsg->mPad != mPad) {
        return false;
    }

    if (pMsg->mPressed != 0) {
        switch (pMsg->mButton) {
        case kPadDRight:
            mSpeedX = kButtonSpeed;
            break;
        case kPadDUp:
            mSpeedY = -kButtonSpeed;
            break;
        case kPadDDown:
            mSpeedY = kButtonSpeed;
            break;
        case kPadDLeft:
            mSpeedX = -kButtonSpeed;
            break;
        default:
            break;
        }
    } else {
        switch (pMsg->mButton) {
        case kPadDRight:
        case kPadDLeft:
            mSpeedX = 0.0f;
            break;
        case kPadDUp:
        case kPadDDown:
            mSpeedY = 0.0f;
            break;
        default:
            break;
        }
    }
    return false;
}

bool ExpressionStick::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleButton(static_cast<JoypadInputMsg *>(pMsg));
    }
    return false;
}
