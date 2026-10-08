#include "game/axetrackcontourbuilder.h"

#include <map>

#include "gs/stdmidimuse.h"

namespace {

constexpr char kDurationError[] = "axe contour duration must be an integral number of beats";
constexpr char kIntroNoteError[] = "no notes allowed in intro";
constexpr char kButtonOrderError[] = "Button notes out of order";

// The channel the controller state follows before the first set.
constexpr unsigned char kInitialChannel = 0;

// The channel message types and the channel bits of a status byte.
constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusChannelMask = 0x0f;
constexpr unsigned char kStatusNoteOff = 0x80;
constexpr unsigned char kStatusNoteOn = 0x90;

// The notes of the three gem buttons.
enum ButtonNote {
    kButtonNoteFirst = 96,
    kButtonNoteSecond = 100,
    kButtonNoteThird = 103,
};

// The value ButtonForNote() returns for a note that is not a gem button.
constexpr int kNoButton = -1;

// The end of a note that has not ended yet.
constexpr int kOpenNoteEnd = -1;

// A note-off moves back to a multiple of this many ticks.
constexpr int kNoteOffQuantumTicks = 30;

// The length of the notes of a set must be a multiple of this many ticks.
constexpr int kTicksPerBeat = 480;

// The tick within the notes the controller state plays at.
constexpr int kStateTick = 0;

} // namespace

int AxeTrackContourBuilder::ButtonForNote(unsigned char nNote) {
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

AxeTrackContourBuilder::AxeTrackContourBuilder(int nTrack,
                                               bool bValidate,
                                               ErrorHandler pfnError,
                                               int nChannel,
                                               int nTicksPerBar,
                                               int nIntroTicks,
                                               AxeTrackData *pData)
    : TrackBuilder(nTrack, bValidate, pfnError), mData(pData), mTicksPerBar(nTicksPerBar),
      mIntroTicks(nIntroTicks), mChannelState(kInitialChannel),
      mValidator(nTrack, pfnError, nChannel, true) {
}

AxeTrackContourBuilder::~AxeTrackContourBuilder() {
}

void AxeTrackContourBuilder::OnEndTrack() {
    auto itMessage = mMessages.begin();
    mChannelState.SetChannel(itMessage->mStatus & kStatusChannelMask);
    AxeContour *apContours[AxeTrackData::kSetSize] = {};
    int nSetTick = 0;
    auto itButton = mButtons.begin();
    if (itButton == mButtons.end()) {
        return;
    }

    int nIndex = 0;
    for (;;) {
        while (itMessage->mTick < itButton->first ||
               (itMessage->mTick == itButton->first &&
                (itMessage->mStatus & kStatusTypeMask) == kStatusNoteOff)) {
            mChannelState.OnMessage(itMessage->mStatus, itMessage->mData1, itMessage->mData2);
            ++itMessage;
            if (itMessage == mMessages.end()) {
                return; // Yes, the binary abandons the notes of an unfinished set.
            }
        }
        if (nIndex == 0) {
            nSetTick = itMessage->mTick;
        }
        apContours[nIndex] = BuildContour(itMessage, itButton->second);
        ++nIndex;
        ++itButton;
        if (nIndex < AxeTrackData::kSetSize) {
            continue;
        }
        mData->AddContours(nSetTick - mIntroTicks, apContours[0], apContours[1], apContours[2]);
        if (itButton == mButtons.end()) {
            return;
        }
        nIndex = 0;
    }
}

void AxeTrackContourBuilder::OnMidi(int nTick,
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

    const bool bNoteOff = nType == kStatusNoteOff;
    if (bNoteOff) {
        nTick -= nTick % kNoteOffQuantumTicks;
    }
    const bool bNoteOn = nType == kStatusNoteOn;
    if ((!bNoteOn && !bNoteOff) || (ButtonForNote(nData1) == kNoButton)) {
        MidiMsg message{nTick, nStatus, nData1, nData2};
        mMessages.push_back(message);
        return;
    }

    if (bNoteOn) {
        mButtons.push_back(std::pair<int, int>(nTick, kOpenNoteEnd));
        if (ButtonForNote(nData1) !=
            static_cast<int>((mButtons.size() - 1) % AxeTrackData::kSetSize)) {
            Error(nTick, kButtonOrderError);
        }
        return;
    }

    auto itButton = mButtons.end() - 1;
    while (itButton != mButtons.begin() && (itButton - 1)->second == kOpenNoteEnd) {
        --itButton;
    }
    itButton->second = nTick;
}

AxeContour *AxeTrackContourBuilder::BuildContour(std::vector<MidiMsg>::iterator itMessage,
                                                 int nEndTick) {
    const int nStartTick = itMessage->mTick;
    const int nLength = nEndTick - nStartTick;
    if (nLength % kTicksPerBeat != 0) {
        Error(nEndTick, kDurationError);
    }

    auto *pContour = new AxeContour(nLength, itMessage->mStatus & kStatusChannelMask);
    Muse *pState = mChannelState.GetMuse();
    if (pState != nullptr) {
        pContour->AddMuse(kStateTick, pState);
    }

    std::map<unsigned char, MidiMsg> notes;
    for (; itMessage != mMessages.end() && itMessage->mTick <= nEndTick; ++itMessage) {
        mChannelState.OnMessage(itMessage->mStatus, itMessage->mData1, itMessage->mData2);
        const unsigned char nType = itMessage->mStatus & kStatusTypeMask;
        if (nType == kStatusNoteOn) {
            notes[itMessage->mData1] = *itMessage;
        } else if (nType == kStatusNoteOff) {
            const MidiMsg &noteOn = notes[itMessage->mData1];
            pContour->AddNote(noteOn.mTick - nStartTick,
                              noteOn.mData1,
                              noteOn.mData2,
                              itMessage->mTick - noteOn.mTick);
        } else {
            pContour->AddMuse(
                itMessage->mTick - nStartTick,
                new StdMidiMuse(itMessage->mStatus, itMessage->mData1, itMessage->mData2));
        }
    }
    return pContour;
}
