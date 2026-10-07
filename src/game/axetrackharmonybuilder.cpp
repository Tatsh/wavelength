#include "game/axetrackharmonybuilder.h"

namespace {

constexpr char kNoHarmoniesError[] = "no harmonies in track";
constexpr char kIntroNoteError[] = "no notes allowed in intro";
constexpr char kOnlyNotesError[] = "only notes are allowed in axe harmony track";
constexpr char kOverlapError[] =
    "harmony notes shouldn't overlap, and should all start at the same tick";

// The tick an error that concerns the whole track is reported at.
constexpr int kTrackErrorTick = 0;

constexpr int kNoTick = -1;

constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusNoteOff = 0x80;
constexpr unsigned char kStatusNoteOn = 0x90;

} // namespace

AxeTrackHarmonyBuilder::AxeTrackHarmonyBuilder(const char *pszName,
                                               bool bValidate,
                                               ErrorHandler pfnError,
                                               int nChannel,
                                               int nIntroTicks,
                                               AxeTrackData *pData)
    : TrackBuilder(pszName, bValidate, pfnError), mData(pData),
      mValidator(pszName, pfnError, nChannel, false), mIntroTicks(nIntroTicks), mStartTick(kNoTick),
      mHarmony(nullptr) {
}

AxeTrackHarmonyBuilder::~AxeTrackHarmonyBuilder() {
}

void AxeTrackHarmonyBuilder::OnEndTrack() {
    if (mHarmony == nullptr) {
        Error(kTrackErrorTick, kNoHarmoniesError);
    }
    if (mHarmony != nullptr) {
        mData->AddHarmony(mStartTick - mIntroTicks, mHarmony);
    }
}

void AxeTrackHarmonyBuilder::OnMidi(int nTick,
                                    unsigned char nStatus,
                                    unsigned char nData1,
                                    unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }

    const unsigned char nType = nStatus & kStatusTypeMask;
    const bool bNoteOff = nType == kStatusNoteOff;
    const bool bNoteOn = nType == kStatusNoteOn;
    if ((nTick < mIntroTicks) && (bNoteOn || bNoteOff)) {
        Error(nTick, kIntroNoteError);
    }

    if (bNoteOn) {
        if (nTick != mStartTick) {
            if (mNotes.find(nData1) != mNotes.end()) {
                Error(nTick, kOverlapError);
            }
            if (mHarmony != nullptr) {
                mData->AddHarmony(mStartTick - mIntroTicks, mHarmony);
            }
            mStartTick = nTick;
            mHarmony = new AxeHarmony();
            mNotes.clear();
        }
        mHarmony->AddNote(nData1);
        mNotes.insert(nData1);
        return;
    }

    if (!bNoteOff) {
        Error(nTick, kOnlyNotesError);
        return;
    }
    mNotes.erase(nData1);
}
