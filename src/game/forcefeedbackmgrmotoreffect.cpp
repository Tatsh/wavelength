#include "game/forcefeedbackmgrmotoreffect.h"

#include "game/forcefeedbackmgrmotoreffectoffcmd.h"

ForceFeedbackMgr::MotorEffect::MotorEffect(int nController,
                                           int nSmallMotor,
                                           int nBigMotor,
                                           int nDuration)
    : mController(nController), mSmallMotor(nSmallMotor), mBigMotor(nBigMotor),
      mDuration(nDuration), mOffCmd(new OffCmd(this)), mActive(0) {
}
