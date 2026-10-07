#include "game/pitchtrackbuilder.h"

#include "os/string.h"

namespace {

constexpr int kNoSlot = -1;
constexpr int kNumSlots = 3;

// The gem notes, one per gem button.
constexpr unsigned char kGemNote0 = 96;
constexpr unsigned char kGemNote1 = 100;
constexpr unsigned char kGemNote2 = 103;

// Notes above the lowest gem note are reserved for gem notes.
constexpr unsigned char kHighestRiffNote = 96;

constexpr unsigned char kMidiStatusMask = 0xf0;
constexpr unsigned char kMidiNoteOff = 0x80;
constexpr unsigned char kMidiNoteOn = 0x90;
constexpr unsigned char kMidiControlChange = 0xb0;

// The controllers the riffs ignore.
constexpr unsigned char kFirstIgnoredController = 93;
constexpr unsigned char kNumIgnoredControllers = 2;

} // namespace

int PitchTrackBuilder::GetGemSlot(unsigned char nNote) {
    switch (nNote) {
    case kGemNote0:
        return 0;
    case kGemNote1:
        return 1;
    case kGemNote2:
        return 2;
    default:
        return kNoSlot;
    }
}

PitchTrackBuilder::PitchTrackBuilder(const char *pszName,
                                     bool bValidate,
                                     ErrorHandler pfnError,
                                     const String &riffName,
                                     int nChannel,
                                     int nStartTick,
                                     int nEndTick,
                                     PitchTrackRiffData *pRiffData)
    : TrackBuilder(pszName, bValidate, pfnError), mStartTick(nStartTick), mEndTick(nEndTick),
      mRiffData(pRiffData), mValidator(pszName, pfnError, nChannel, true), mRiffs(), mGems() {
    mRiffData->mName = riffName;
}

void PitchTrackBuilder::OnEndTrack() {
    Muse *riffs[kNumSlots];
    int nSetTick = 0;
    int i = 0;
    for (auto it = mGems.begin(); it != mGems.end(); ++it, ++i) {
        const int nSlot = i % kNumSlots;
        if (nSlot == 0) {
            nSetTick = it->mTick;
        }
        const int nEnd = (mGems.end() - it) == 1 ? mEndTick : (it + 1)->mTick;
        if (mValidate && mRiffs.CountMessages(it->mTick, nEnd) == 0) {
            Error(it->mTick + mStartTick, FormatString("No riffs on button %i", nSlot));
        }
        riffs[nSlot] = mRiffs.Extract(it->mTick, nEnd);
        if (nSlot == kNumSlots - 1) {
            mRiffData->AddRiffs(nSetTick, riffs[0], riffs[1], riffs[2]);
        }
    }
}

void PitchTrackBuilder::OnMidi(int nTick,
                               unsigned char nStatus,
                               unsigned char nData1,
                               unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }

    const unsigned char nType = nStatus & kMidiStatusMask;
    if (nType == kMidiControlChange &&
        static_cast<unsigned int>(nData1 - kFirstIgnoredController) < kNumIgnoredControllers) {
        return;
    }
    if (nTick < mStartTick && (nType == kMidiNoteOn || nType == kMidiNoteOff)) {
        Error(nTick, "No notes allowed in intro");
    }

    const bool bNoteOn = nType == kMidiNoteOn;
    if (!bNoteOn && nType != kMidiNoteOff) {
        mRiffs.AddMidi(nTick - mStartTick, nStatus, nData1, nData2);
        return;
    }

    const int nSlot = GetGemSlot(nData1);
    if (nSlot == kNoSlot) {
        mRiffs.AddMidi(nTick - mStartTick, nStatus, nData1, nData2);
    }
    if (!bNoteOn) {
        return;
    }
    if (nSlot == kNoSlot) {
        if (nData1 > kHighestRiffNote) {
            Error(nTick, FormatString("Note %i is above 96, but is not a gem note", nData1));
        }
        return;
    }

    if (mValidate) {
        if (mGems.empty() && nTick != mStartTick) {
            Error(nTick, "No gem at first tick of song");
        }
        if (!mGems.empty() &&
            (static_cast<unsigned char>(mGems.back().mSlot) + 1) % kNumSlots != nSlot) {
            Error(nTick, "Gem notes not in order");
        }
    }
    mGems.push_back(Gem{static_cast<signed char>(nSlot), nTick - mStartTick});
}
