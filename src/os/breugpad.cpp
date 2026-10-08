#include "os/breugpad.h"

#include <cstdio>

#include <libpad.h>

namespace {

// Identifiers scePadInfoMode() reports that Read() recognises.
constexpr int kPadIdAnalog = 4;
constexpr int kPadIdDualShock2 = 7;

// The main mode Read() requests, analog and locked.
constexpr int kMainModeAnalog = 1;
constexpr int kMainModeLock = 3;

// The actuator scePadInfoAct() takes to report the actuator count.
constexpr int kInfoActCount = -1;

// A successful request of the calls that report success as 1.
constexpr int kPadCallSucceeded = 1;

// Setup phases. Every other index of the image's 78-entry table does nothing.
enum Phase {
    kPhaseProbe = 0,
    kPhaseAnalogProbe = 40,
    kPhaseAnalogSetMode = 41,
    kPhaseAnalogWait = 42,
    kPhaseActuatorAlign = 70,
    kPhaseActuatorWait = 71,
    kPhasePressureProbe = 72,
    kPhasePressureEnter = 76,
    kPhasePressureWait = 77,
    kPhaseTableSize = 78,
    kPhaseDone = 99
};

// Levels of mReadyLevel.
constexpr int kReadyDigital = 1;
constexpr int kReadyAnalog = 2;
constexpr int kReadyPressure = 3;

// Layout of a report.
constexpr int kReportSize = 32;
enum ReportByte {
    kReportStatus = 0,
    kReportMode = 1,
    kReportButtonsHigh = 2,
    kReportButtonsLow = 3,
    kReportRightX = 4,
    kReportRightY = 5,
    kReportLeftX = 6,
    kReportLeftY = 7,
    kReportPressures = 8
};
constexpr int kReportOk = 0;
constexpr int kBitsPerByte = 8;

// The value an analog byte reads at rest.
constexpr int kAnalogCentre = 0x80;

// The largest magnitude of an axis centred on zero.
constexpr int kAxisMax = 127;
constexpr float kAxisMaxFloat = 127.0F;

// Byte of the unused alignment entries.
constexpr unsigned char kActuatorUnused = 0xff;

// Report bytes the first two actuators take, which leaves the other four unused.
constexpr unsigned char kSmallMotorByte = 0;
constexpr unsigned char kBigMotorByte = 1;

// Actuator entries of the small and big motors.
enum ActuatorIndex { kSmallMotor = 0, kBigMotor = 1 };

// Set once the first record has started libpad.
// NTSC-U/C: 0x003b2270
int gPadDriverStarted;

// The linear map ApplyDeadzone() applies, set by the latest Init().
// NTSC-U/C: 0x003b2268
float gDeadzoneScale;
// NTSC-U/C: 0x003b226c
float gDeadzoneBias;

// Replace a failed request's phase with the request that failed, and report completion.
inline bool WaitForRequest(BreugPad *pPad) {
    if (scePadGetReqState(pPad->mPort, pPad->mSlot) == scePadReqStateFaild) {
        --pPad->mPhase;
    }
    return scePadGetReqState(pPad->mPort, pPad->mSlot) == scePadReqStateComplete;
}

// The setup step for the current phase. Read() expands it inline.
inline void AdvancePhase(BreugPad *pPad, int nState) {
    switch (pPad->mPhase) {
    case kPhaseProbe: {
        if (nState != scePadStateStable && nState != scePadStateFindCTP1) {
            break;
        }
        int nId = scePadInfoMode(pPad->mPort, pPad->mSlot, InfoModeCurID, 0);
        if (nId == 0) {
            break;
        }
        const int nExtendedId = scePadInfoMode(pPad->mPort, pPad->mSlot, InfoModeCurExID, 0);
        if (nExtendedId > 0) {
            nId = nExtendedId;
        }
        if (nId == kPadIdAnalog) {
            pPad->mReadyLevel = kReadyDigital;
            pPad->mPhase = kPhaseAnalogProbe;
        } else if (nId == kPadIdDualShock2) {
            pPad->mPhase = kPhaseActuatorAlign;
        } else {
            pPad->mPhase = kPhaseDone;
        }
        break;
    }
    case kPhaseAnalogProbe:
        if (scePadInfoMode(pPad->mPort, pPad->mSlot, InfoModeCurExID, 0) == 0) {
            pPad->mPhase = kPhaseDone;
            break;
        }
        ++pPad->mPhase;
        [[fallthrough]];
    case kPhaseAnalogSetMode:
        if (scePadSetMainMode(pPad->mPort, pPad->mSlot, kMainModeAnalog, kMainModeLock) ==
            kPadCallSucceeded) {
            ++pPad->mPhase;
        }
        break;
    case kPhaseAnalogWait:
        if (WaitForRequest(pPad)) {
            pPad->mPhase = kPhaseProbe;
            pPad->mReadyLevel = kReadyAnalog;
        }
        break;
    case kPhaseActuatorAlign:
        if (scePadInfoAct(pPad->mPort, pPad->mSlot, kInfoActCount, 0) == 0) {
            pPad->mPhase = kPhaseDone;
            break;
        }
        if (scePadSetActAlign(pPad->mPort, pPad->mSlot, pPad->mActAlign) == 0) {
            printf("BreugPad: Set actAlign failed!!!!!!!!!!!!\n");
            break;
        }
        ++pPad->mPhase;
        if (scePadGetReqState(pPad->mPort, pPad->mSlot) != scePadReqStateBusy) {
            printf("BreugPad: Set actAlign warning!!!!!!!!!!!!\n");
        }
        break;
    case kPhaseActuatorWait:
        if (WaitForRequest(pPad)) {
            ++pPad->mPhase;
        }
        break;
    case kPhasePressureProbe:
        pPad->mPhase = scePadInfoPressMode(pPad->mPort, pPad->mSlot) == kPadCallSucceeded ?
                           kPhasePressureEnter :
                           kPhaseDone;
        break;
    case kPhasePressureEnter:
        if (scePadEnterPressMode(pPad->mPort, pPad->mSlot) == kPadCallSucceeded) {
            ++pPad->mPhase;
        }
        break;
    case kPhasePressureWait:
        if (WaitForRequest(pPad)) {
            pPad->mPhase = kPhaseDone;
            pPad->mReadyLevel = kReadyPressure;
        }
        break;
    default:
        break;
    }
}

} // namespace

void BreugPad::SetActuators(int nSmallMotor, unsigned char nBigMotor) {
    if (mReadyLevel < kReadyAnalog) {
        return;
    }
    mActDirect[kSmallMotor] = nSmallMotor > 0;
    mActDirect[kBigMotor] = nBigMotor;
    scePadSetActDirect(mPort, mSlot, mActDirect);
}

void BreugPad::Init(int nPort, int nSlot, int nDeadZone) {
    for (int i = 0; i < kActuatorByteCount; ++i) {
        mActDirect[i] = 0;
        mActAlign[i] = kActuatorUnused;
    }
    for (int i = kPressureByteCount - 1; i >= 0; --i) {
        mPressureBaseline[i] = 0;
    }
    mActAlign[kSmallMotor] = kSmallMotorByte;
    mActAlign[kBigMotor] = kBigMotorByte;
    mPort = nPort;
    mSlot = nSlot;
    mPhase = 0;
    mRawButtons = 0;
    mReadCount = 0;
    mReportMode = 0;
    if (gPadDriverStarted == 0) {
        scePadInit(0);
        gPadDriverStarted = 1;
    }
    for (auto &byte : mLastAxes) {
        byte = 0;
    }
    for (auto &value : mAxisDeltas) {
        value = 0;
    }
    scePadPortOpen(nPort, nSlot, mDmaArea);
    mDeadZone = nDeadZone;
    mButtons = 0;
    mHeldButtonsSeen = 0;
    mToggledButtons = 0;
    mPreviousButtons = 0;
    gDeadzoneScale = kAxisMaxFloat / static_cast<float>(kAxisMax - nDeadZone);
    gDeadzoneBias = -gDeadzoneScale * static_cast<float>(nDeadZone);
}

short BreugPad::ApplyDeadzone(short nAxis, int nDeadZone) {
    const short nSign = nAxis > -1 ? 1 : -1;
    const short nMagnitude = static_cast<short>(nAxis * nSign);
    if (nMagnitude < nDeadZone) {
        return 0;
    }
    const float fScaled = static_cast<float>(nMagnitude) * gDeadzoneScale + gDeadzoneBias;
    const short nRounded = static_cast<short>(static_cast<int>(fScaled + 0.5));
    return static_cast<short>(nSign * nRounded);
}

int BreugPad::Read(int *pButtons,
                   unsigned char *pAxis0,
                   unsigned char *pAxis1,
                   unsigned char *pAxis2,
                   unsigned char *pAxis3,
                   unsigned char *pPressures,
                   short *pPressureDeltas) {
    int nLeftX = 0;
    int nLeftY = 0;
    int nRightX = 0;
    int nRightY = 0;
    ++mReadCount;
    const int nState = scePadGetState(mPort, mSlot);
    if (nState == scePadStateDiscon) {
        mPhase = kPhaseProbe;
        mReadyLevel = 0;
    }
    if (static_cast<unsigned>(mPhase) < kPhaseTableSize) {
        AdvancePhase(this, nState);
    }

    if (nState != scePadStateStable && nState != scePadStateFindCTP1) {
        if (pButtons != nullptr) {
            *pButtons = mButtons;
        }
        if (pAxis0 != nullptr) {
            *pAxis0 = static_cast<unsigned char>(nLeftX);
        }
        if (pAxis1 != nullptr) {
            *pAxis1 = static_cast<unsigned char>(nLeftY);
        }
        if (pAxis2 != nullptr) {
            *pAxis2 = static_cast<unsigned char>(nRightX);
        }
        if (pAxis3 != nullptr) {
            *pAxis3 = static_cast<unsigned char>(nRightY);
        }
        return mReadyLevel;
    }

    // Yes, the binary reads the status and mode bytes below even when mReadyLevel is zero and the
    // report was never read.
    unsigned char abReport[kReportSize];
    if (mReadyLevel > 0) {
        mPreviousButtons = mButtons;
        if (scePadRead(mPort, mSlot, abReport) == 0) {
            return 0;
        }
        const unsigned short nNow = static_cast<unsigned short>(
            ~((abReport[kReportButtonsHigh] << kBitsPerByte) | abReport[kReportButtonsLow]));
        const unsigned short nPrevious = mRawButtons;
        mRawButtons = nNow;
        mToggledButtons ^= nNow & ~nPrevious;
        mHeldButtonsSeen |= mRawButtons;
        mButtons = mRawButtons;
    }

    if (mReadyLevel >= kReadyAnalog) {
        nLeftX =
            ApplyDeadzone(static_cast<short>(abReport[kReportLeftX] - kAnalogCentre), mDeadZone);
        nLeftY =
            ApplyDeadzone(static_cast<short>(abReport[kReportLeftY] - kAnalogCentre), mDeadZone);
        nRightX =
            ApplyDeadzone(static_cast<short>(abReport[kReportRightX] - kAnalogCentre), mDeadZone);
        nRightY =
            ApplyDeadzone(static_cast<short>(abReport[kReportRightY] - kAnalogCentre), mDeadZone);
    }

    if (pButtons != nullptr) {
        *pButtons = mButtons;
    }
    if (pAxis0 != nullptr) {
        *pAxis0 = static_cast<unsigned char>(nLeftX);
    }
    if (pAxis1 != nullptr) {
        *pAxis1 = static_cast<unsigned char>(nLeftY);
    }
    if (pAxis2 != nullptr) {
        *pAxis2 = static_cast<unsigned char>(nRightX);
    }
    if (pAxis3 != nullptr) {
        *pAxis3 = static_cast<unsigned char>(nRightY);
    }

    if (abReport[kReportStatus] == kReportOk && mReadyLevel == kReadyPressure) {
        for (int i = 0; i < kPressureByteCount; ++i) {
            const unsigned char nPressure = abReport[kReportPressures + i];
            if (pPressureDeltas != nullptr) {
                pPressureDeltas[i] = static_cast<short>(nPressure - mPressureBaseline[i]);
            }
            if (pPressures != nullptr) {
                pPressures[i] = nPressure;
            }
            mPressureBaseline[i] = nPressure;
        }
    }
    mReportMode = abReport[kReportMode];
    return mReadyLevel;
}
