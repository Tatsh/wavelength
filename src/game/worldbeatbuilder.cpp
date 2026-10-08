#include "game/worldbeatbuilder.h"

namespace {

constexpr char kNoteRangeError[] =
    "notes in WORLD track must be C, D, E, F, G, A, or B in octave starting at note 96";
constexpr char kNotesOnlyError[] = "only notes are allowed in WORLD track";

constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusNoteOn = 0x90;
constexpr unsigned char kStatusNoteOff = 0x80;

// The validator accepts every channel and does not require a program change.
constexpr int kAnyChannel = -1;

// The notes of the octave from note 96 that mark events, and the letter of each.
constexpr unsigned char kNoteC = 96;
constexpr unsigned char kNoteD = 98;
constexpr unsigned char kNoteE = 100;
constexpr unsigned char kNoteF = 101;
constexpr unsigned char kNoteG = 103;
constexpr unsigned char kNoteA = 105;
constexpr unsigned char kNoteB = 107;
constexpr char kNoLetter = '\0';

} // namespace

WorldBeatBuilder::WorldBeatBuilder(
    int nTrack, bool bValidate, ErrorHandler pfnError, int nIntroTicks, WorldTrack *pTrack)
    : TrackBuilder(nTrack, bValidate, pfnError), mIntroTicks(nIntroTicks), mTrack(pTrack),
      mValidator(nTrack, pfnError, kAnyChannel, false) {
}

WorldBeatBuilder::~WorldBeatBuilder() {
}

void WorldBeatBuilder::OnMidi(int nTick,
                              unsigned char nStatus,
                              unsigned char nData1,
                              unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }
    const unsigned char nType = nStatus & kStatusTypeMask;
    if (nType == kStatusNoteOn) {
        char cLetter = kNoLetter;
        switch (nData1) {
        case kNoteC:
            cLetter = 'C';
            break;
        case kNoteD:
            cLetter = 'D';
            break;
        case kNoteE:
            cLetter = 'E';
            break;
        case kNoteF:
            cLetter = 'F';
            break;
        case kNoteG:
            cLetter = 'G';
            break;
        case kNoteA:
            cLetter = 'A';
            break;
        case kNoteB:
            cLetter = 'B';
            break;
        default:
            Error(nTick, kNoteRangeError);
            break;
        }
        if (cLetter != kNoLetter) {
            mTrack->Insert(nTick - mIntroTicks, cLetter);
        }
    } else if (nType != kStatusNoteOff) {
        Error(nTick, kNotesOnlyError);
    }
}
