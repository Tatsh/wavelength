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
    : mWorld(pWorld), mRepeatInitialDelayMs(TheGameConfig->GetRotationRepeatInitialDelayMs()),
      mRepeatDelayMs(TheGameConfig->GetRotationRepeatDelayMs()),
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
        data.SetStick(
            std::make_unique<ExpressionStick>(i, pMap->FindStick(kStickActionExpression)));
        data.SetPositionStick(pMap->FindStick(kStickActionPosition));
        data.SetRepeatMs(0.0f);
        mControllers.push_back(std::move(data));
    }
}

InputMgr::~InputMgr() {
    JoypadRemoveSink(this);
    // Preserve the original deletion point, after the sink is removed.
    for (ControllerData &data : mControllers) {
        data.ResetStick();
    }
}

void InputMgr::Poll() {
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[i]);
        ControllerData &data = mControllers[i];
        RepeatRotation(i);
        if (data.GetStick()->Poll()) {
            StickEvent<2> expression;
            expression.SetPlayer(nPlayer);
            expression.SetX(data.GetStick()->GetX());
            expression.SetY(data.GetStick()->GetY());
            mWorld->Handle(expression);
        }
        const JoypadStick &stick = JoypadGetState(i)->GetSticks()[data.GetPositionStick()];
        StickEvent<6> position;
        position.SetPlayer(nPlayer);
        position.SetX(stick.mX);
        position.SetY(stick.mY);
        mWorld->Handle(position);
    }
}

void InputMgr::RepeatRotation(int nPad) {
    ControllerData &data = mControllers[nPad];
    const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[nPad]);
    data.GetRepeatTimer().Stop();
    const float fOverMs = CyclesToMs(data.GetRepeatTimer().GetCycles()) - mRepeatInitialDelayMs;
    if (fOverMs > 0.0f) {
        const float fLastOverMs = data.GetRepeatMs() - mRepeatInitialDelayMs;
        if (static_cast<int>(fLastOverMs / mRepeatDelayMs) <
            static_cast<int>(fOverMs / mRepeatDelayMs)) {
            RotateEvent rotate;
            rotate.SetPlayer(nPlayer);
            rotate.SetDirection(data.GetDirection());
            mWorld->Handle(rotate);
        }
    }
    data.SetRepeatMs(CyclesToMs(data.GetRepeatTimer().GetCycles()));
    data.GetRepeatTimer().Start();
}

bool InputMgr::HandleButton(JoypadInputMsg *pMsg) {
    if (mDemo) {
        mWorld->Stop();
        return false;
    }
    const int nPad = pMsg->GetPad();
    const int nPressed = pMsg->GetPressed();
    if (nPad >= TheGameDb->GetNumPads()) {
        return false;
    }
    const unsigned char nPlayer = static_cast<unsigned char>(mLocalPlayers[nPad]);
    const int nButton = pMsg->GetButton();
    if (nButton == kButtonStart) {
        if (nPressed != 0) {
            BtnEvent<3> pause;
            pause.SetPlayer(nPlayer);
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
            data.GetStick()->Reset();
            RotateEvent rotate;
            rotate.SetPlayer(nPlayer);
            rotate.SetDirection(nDirection);
            mWorld->Handle(rotate);
            data.GetRepeatTimer().Start();
            data.SetDirection(nDirection);
        } else {
            data.GetRepeatTimer().Stop();
            if (mWorld->IsFreestyling(nPad)) {
                break;
            }
            const float fHeldMs = CyclesToMs(data.GetRepeatTimer().GetCycles());
            data.GetRepeatTimer().SetRunning(0);
            data.GetRepeatTimer().SetCycles(0);
            data.GetRepeatTimer().SetLastMs(fHeldMs);
        }
        break;
    }
    case kActionFirstGem:
    case kActionFirstGem + 1:
    case kActionLastGem: {
        const ExpressionStick *pStick = mControllers[nPad].GetStick();
        const PlayNoteEvent note(nPlayer,
                                 static_cast<unsigned char>(nAction - kActionFirstGem),
                                 nPressed,
                                 pStick->GetX(),
                                 pStick->GetY());
        mWorld->Handle(note);
        break;
    }
    case kActionOption:
        if (nPressed != 0) {
            if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix) {
                BtnEvent<9> event;
                event.SetPlayer(nPlayer);
                mWorld->Handle(event);
            } else {
                BtnEvent<4> event;
                event.SetPlayer(nPlayer);
                mWorld->Handle(event);
            }
        }
        break;
    case kActionDeploy:
        if (nPressed != 0) {
            if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix) {
                BtnEvent<8> event;
                event.SetPlayer(nPlayer);
                mWorld->Handle(event);
            } else {
                BtnEvent<5> event;
                event.SetPlayer(nPlayer);
                mWorld->Handle(event);
            }
        }
        break;
    case kActionSectionPrevious:
    case kActionSectionNext:
        if (nPressed != 0) {
            ChangeSectionEvent section;
            section.SetPlayer(nPlayer);
            section.SetDirection(nAction != kActionSectionPrevious ? 1 : 0);
            mWorld->Handle(section);
        }
        break;
    case kActionTrack:
        if (nPressed != 0) {
            BtnEvent<10> event;
            event.SetPlayer(nPlayer);
            mWorld->Handle(event);
        }
        break;
    default:
        DebugWarn("unknown button event type");
        break;
    }
    return false;
}

bool InputMgr::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleButton(static_cast<JoypadInputMsg *>(pMsg));
    }
    return false;
}
