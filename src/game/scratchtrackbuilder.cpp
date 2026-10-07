#include "game/scratchtrackbuilder.h"

#include "game/gameconfig.h"
#include "gs/muse.h"
#include "gs/scratcher.h"
#include "os/ptr.h"
#include "os/string.h"

namespace {

constexpr char kNoGemsError[] = "Scratch track has no gems";
constexpr char kSetSizeError[] = "Scratchers must be in sets of 3";
constexpr char kMissingScratcherFormat[] = "No scratcher for button %i";
constexpr char kIntroNoteError[] = "no notes allowed in intro";
constexpr char kButtonOrderError[] = "Button notes out of order";

// The tick an error that concerns the whole track is reported at.
constexpr int kTrackErrorTick = 0;

// The tick of the first scratcher set, and the end of a note that has not ended yet.
constexpr int kFirstSetTick = 0;
constexpr int kOpenNoteEnd = -1;

// The value ScratchTrackData::GetScratchers() returns after the last set.
constexpr int kNoNextSet = -1;

// The position of the last SCRATCH track. Its builder checks the finished sets.
constexpr int kLastIndex = ScratchTrackData::kSetSize - 1;

// The channel message types the scratch buttons use.
constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusNoteOff = 0x80;
constexpr unsigned char kStatusNoteOn = 0x90;

// The notes of the three scratch buttons.
enum ButtonNote {
    kButtonNoteFirst = 96,
    kButtonNoteSecond = 100,
    kButtonNoteThird = 103,
};

// The value ButtonForNote() returns for a note that is not a scratch button.
constexpr int kNoButton = -1;

} // namespace

int ScratchTrackBuilder::ButtonForNote(unsigned char nNote) {
    switch (nNote) {
    case kButtonNoteFirst:
        return 0;
    case kButtonNoteSecond:
        return 1;
    case kButtonNoteThird:
        return 2;
    default:
        return kNoButton;
    }
}

ScratchTrackBuilder::ScratchTrackBuilder(const char *pszName,
                                         bool bValidate,
                                         ErrorHandler pfnError,
                                         int nChannel,
                                         int nIndex,
                                         int nTicksPerBar,
                                         int nIntroTicks,
                                         int nReserved,
                                         [[maybe_unused]] int nUnused,
                                         ScratchTrackData *pData)
    : TrackBuilder(pszName, bValidate, pfnError), mData(pData), mReserved14(nReserved),
      mIndex(nIndex), mTicksPerBar(nTicksPerBar), mIntroTicks(nIntroTicks),
      mValidator(pszName, pfnError, nChannel, true) {
}

ScratchTrackBuilder::~ScratchTrackBuilder() {
}

void ScratchTrackBuilder::OnEndTrack() {
    if (mButtons.empty()) {
        Error(kTrackErrorTick, kNoGemsError);
    }
    if ((mButtons.size() % ScratchTrackData::kSetSize) != 0) {
        Error(kTrackErrorTick, kSetSizeError);
    }

    const int nQuantum = TheGameConfig->mScratcherQuantizationTicks;
    int nSetTick = kFirstSetTick;
    const int nChannel = mPieces.GetChannel();
    Ptr<Muse> apPieces[ScratchTrackData::kSetSize];
    for (unsigned int i = 0; i < mButtons.size(); ++i) {
        const int nButton = static_cast<int>(i) % ScratchTrackData::kSetSize;
        const int nStart = mButtons[i].first;
        const int nLength = mButtons[i].second - nStart;
        apPieces[nButton] = Ptr<Muse>(mPieces.Extract(nStart, mButtons[i].second));
        if (nButton == 0) {
            nSetTick = nStart;
        } else if (nButton == kLastIndex) {
            Scratcher *pScratcher = new Scratcher(apPieces[0].Get(),
                                                  apPieces[1].Get(),
                                                  apPieces[2].Get(),
                                                  nQuantum,
                                                  nLength,
                                                  nChannel);
            mData->SetScratcher(nSetTick - mIntroTicks, mIndex, pScratcher);
        }
    }

    if (mValidate && (mIndex == kLastIndex)) {
        Scratcher *apSet[ScratchTrackData::kSetSize];
        int nTick = kFirstSetTick;
        // Yes, the binary reports a gap with the tick of the set after the incomplete one.
        while ((nTick = mData->GetScratchers(nTick, &apSet[0], &apSet[1], &apSet[2])) !=
               kNoNextSet) {
            for (int j = 0; j < ScratchTrackData::kSetSize; ++j) {
                if (apSet[j] == nullptr) {
                    Error(nTick, FormatString(kMissingScratcherFormat, j + 1));
                }
            }
        }
    }
}

void ScratchTrackBuilder::OnMidi(int nTick,
                                 unsigned char nStatus,
                                 unsigned char nData1,
                                 unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }

    const unsigned char nType = nStatus & kStatusTypeMask;
    if ((nTick < mIntroTicks) && ((nType == kStatusNoteOn) || (nType == kStatusNoteOff))) {
        Error(nTick, kIntroNoteError);
    }

    const bool bNoteOn = nType == kStatusNoteOn;
    if ((!bNoteOn && (nType != kStatusNoteOff)) || (ButtonForNote(nData1) == kNoButton)) {
        mPieces.AddMidi(nTick, nStatus, nData1, nData2);
        return;
    }

    if (bNoteOn) {
        mButtons.push_back(std::pair<int, int>(nTick, kOpenNoteEnd));
    } else {
        mButtons.back().second = nTick;
    }
    if (ButtonForNote(nData1) !=
        static_cast<int>((mButtons.size() - 1) % ScratchTrackData::kSetSize)) {
        Error(nTick, kButtonOrderError);
    }
}
