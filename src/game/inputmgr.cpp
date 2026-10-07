#include "game/inputmgr.h"

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/inputmap.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/system.h"

namespace {

// The START button, which pauses whatever action it is bound to.
constexpr int kButtonStart = 11;

// Actions of the controller buttons.
constexpr int kActionNone = 0;
constexpr int kActionRotatePrevious = 1;
constexpr int kActionRotateNext = 2;
constexpr int kActionFirstGem = 3;
constexpr int kActionLastGem = 5;
constexpr int kActionOption = 6;
constexpr int kActionDeploy = 7;
constexpr int kActionSectionPrevious = 8;
constexpr int kActionSectionNext = 9;
// An action the track of the player acts on.
constexpr int kActionTrack = 10;

// Actions of the analogue sticks.
constexpr int kStickActionExpression = 1;
constexpr int kStickActionPosition = 2;

inline float CyclesToMs(unsigned nCycles) {
    return static_cast<float>(nCycles) * gSystemCycles2Ms;
}

} // namespace

InputMgr::InputMgr(World *pWorld)
    : mWorld(pWorld), mRepeatInitialDelayMs(TheGameConfig->mRotationRepeatInitialDelayMs),
      mRepeatDelayMs(TheGameConfig->mRotationRepeatDelayMs),
      mDemo(TheGameDb->GetDemo() != nullptr) {
    JoypadAddSink(this);
    mControllers.reserve(TheGameDb->GetNumPads());
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (TheGameDb->IsLocalPlayer(i)) {
            mLocalPlayers.push_back(i);
        }
    }
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        InputMap *pMap = TheGameDb->GetInputMap(i);
        ControllerData data;
        data.mStick = new ExpressionStick(i, pMap->FindStick(kStickActionExpression));
        data.mPositionStick = pMap->FindStick(kStickActionPosition);
        data.mRepeatMs = 0.0f;
        mControllers.push_back(data);
    }
}

InputMgr::~InputMgr() {
    JoypadRemoveSink(this);
    for (ControllerData &data : mControllers) {
        delete data.mStick;
    }
}

void InputMgr::Poll() {
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[i]);
        ControllerData &data = mControllers[i];
        RepeatRotation(i);
        if (data.mStick->Poll()) {
            StickEvent<2> expression;
            expression.mPlayer = nPlayer;
            expression.mX = data.mStick->mX;
            expression.mY = data.mStick->mY;
            mWorld->Handle(expression);
        }
        const JoypadStick &stick = JoypadGetState(i)->mSticks[data.mPositionStick];
        StickEvent<6> position;
        position.mPlayer = nPlayer;
        position.mX = stick.mX;
        position.mY = stick.mY;
        mWorld->Handle(position);
    }
}

void InputMgr::RepeatRotation(int nPad) {
    ControllerData &data = mControllers[nPad];
    const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[nPad]);
    data.mRepeatTimer.Stop();
    const float fOverMs = CyclesToMs(data.mRepeatTimer.mCycles) - mRepeatInitialDelayMs;
    if (fOverMs > 0.0f) {
        const float fLastOverMs = data.mRepeatMs - mRepeatInitialDelayMs;
        if (static_cast<int>(fLastOverMs / mRepeatDelayMs) <
            static_cast<int>(fOverMs / mRepeatDelayMs)) {
            RotateEvent rotate;
            rotate.mPlayer = nPlayer;
            rotate.mDirection = data.mDirection;
            mWorld->Handle(rotate);
        }
    }
    data.mRepeatMs = CyclesToMs(data.mRepeatTimer.mCycles);
    data.mRepeatTimer.Start();
}

bool InputMgr::HandleButton(JoypadInputMsg *pMsg) {
    if (mDemo) {
        mWorld->Stop();
        return false;
    }
    const int nPad = pMsg->mPad;
    const int nPressed = pMsg->mPressed;
    if (nPad >= TheGameDb->GetNumPads()) {
        return false;
    }
    const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[nPad]);
    const int nButton = pMsg->mButton;
    if (nButton == kButtonStart) {
        if (nPressed != 0) {
            BtnEvent<3> pause;
            pause.mPlayer = nPlayer;
            mWorld->Handle(pause);
        }
        return false;
    }

    const int nAction = TheGameDb->GetInputMap(nPad)->GetButtonAction(nButton);
    switch (nAction) {
    case kActionNone:
        break;
    case kActionRotatePrevious:
    case kActionRotateNext: {
        ControllerData &data = mControllers[nPad];
        if (nPressed != 0) {
            const int nDirection = nAction != kActionRotatePrevious ?
                                       RotateEvent::kDirectionNext :
                                       RotateEvent::kDirectionPrevious;
            if (mWorld->IsFreestyling(nPad)) {
                break;
            }
            data.mStick->Reset();
            RotateEvent rotate;
            rotate.mPlayer = nPlayer;
            rotate.mDirection = nDirection;
            mWorld->Handle(rotate);
            data.mRepeatTimer.Start();
            data.mDirection = nDirection;
        } else {
            data.mRepeatTimer.Stop();
            if (mWorld->IsFreestyling(nPad)) {
                break;
            }
            const float fHeldMs = CyclesToMs(data.mRepeatTimer.mCycles);
            data.mRepeatTimer.mRunning = 0;
            data.mRepeatTimer.mCycles = 0;
            data.mRepeatTimer.mLastMs = fHeldMs;
        }
        break;
    }
    case kActionFirstGem:
    case kActionFirstGem + 1:
    case kActionLastGem: {
        const ExpressionStick *pStick = mControllers[nPad].mStick;
        const PlayNoteEvent note(nPlayer,
                                 static_cast<unsigned char>(nAction - kActionFirstGem),
                                 nPressed,
                                 pStick->mX,
                                 pStick->mY);
        mWorld->Handle(note);
        break;
    }
    case kActionOption:
        if (nPressed != 0) {
            if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
                BtnEvent<9> event;
                event.mPlayer = nPlayer;
                mWorld->Handle(event);
            } else {
                BtnEvent<4> event;
                event.mPlayer = nPlayer;
                mWorld->Handle(event);
            }
        }
        break;
    case kActionDeploy:
        if (nPressed != 0) {
            if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
                BtnEvent<8> event;
                event.mPlayer = nPlayer;
                mWorld->Handle(event);
            } else {
                BtnEvent<5> event;
                event.mPlayer = nPlayer;
                mWorld->Handle(event);
            }
        }
        break;
    case kActionSectionPrevious:
    case kActionSectionNext:
        if (nPressed != 0) {
            ChangeSectionEvent section;
            section.mPlayer = nPlayer;
            section.mDirection = nAction != kActionSectionPrevious ? 1 : 0;
            mWorld->Handle(section);
        }
        break;
    case kActionTrack:
        if (nPressed != 0) {
            BtnEvent<10> event;
            event.mPlayer = nPlayer;
            mWorld->Handle(event);
        }
        break;
    default:
        DebugWarn("unknown button event type");
        break;
    }
    return false;
}

void InputMgr::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        HandleButton(static_cast<JoypadInputMsg *>(pMsg));
    }
}
