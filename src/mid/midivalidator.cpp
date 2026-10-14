#include "mid/midivalidator.h"

#include "mid/mbt.h"
#include "os/debug.h"

namespace {

constexpr char kErrorPrefix[] = "ERROR ";
constexpr char kTrackLabel[] = "(MIDI track ";
constexpr char kTickLabel[] = ", tick ";
constexpr char kMessageSeparator[] = "): ";

constexpr int kBeatsPerMeasure = 4;
constexpr int kTicksPerBeat = 480;

constexpr unsigned char kChannelMask = 0x0f;
constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusNoteOn = 0x90;
constexpr unsigned char kStatusControlChange = 0xb0;
constexpr unsigned char kStatusProgramChange = 0xc0;
constexpr unsigned char kControllerBankSelect = 0;

// NTSC-U/C: 0x0027e0a0, PAL: 0x002879b8
String FormatMessage(const String &prefix, int nTick, int nTrack, const String &message) {
    String text;
    text.Print(prefix.c_str());
    PrnStream &stream = text << kTrackLabel << nTrack << kTickLabel;
    const MBT mbt(nTick, kBeatsPerMeasure, kTicksPerBeat);
    stream.Print(mbt.ToString().c_str());
    (stream << kMessageSeparator).Print(message.c_str());
    // Yes, the binary copies the text into the result rather than building it in place.
    return static_cast<const String &>(text);
}

} // namespace

MidiValidator::MidiValidator(int nTrack,
                             TrackBuilder::ErrorHandler pfnError,
                             int nChannel,
                             bool bRequirePrograms)
    : mTrack(nTrack), mChannel(nChannel), mErrorHandler(pfnError),
      mRequirePrograms(bRequirePrograms), mTick(kNone), mProgramChanged(0), mFirstNoteTick(kNone),
      mFirstProgramTick(kNone), mFirstBankTick(kNone), mOrderChecked(0), mChannelReported(0) {
}

MidiValidator::~MidiValidator() {
}

void MidiValidator::Check(int nTick,
                          unsigned char nStatus,
                          unsigned char nData1,
                          [[maybe_unused]] unsigned char nData2) {
    if (mChannel != kNone) {
        const int nChannel = nStatus & kChannelMask;
        if (nChannel != mChannel && !mChannelReported) {
            Error(nTick,
                  FormatString("Incorrect channel (%i); should be on channel %i",
                               nChannel + 1,
                               mChannel + 1));
            mChannelReported = 1;
        }
    }

    if (nTick != mTick) {
        mNotes.clear();
        mControllers.clear();
        mTick = nTick;
        mProgramChanged = 0;
    }

    const unsigned char nType = nStatus & kStatusTypeMask;
    if (nType == kStatusNoteOn) {
        if (!mNotes.insert(nData1).second) {
            Error(nTick, FormatString("Duplicate note: %i", nData1));
        }
    } else if (nType == kStatusProgramChange) {
        if (mProgramChanged) {
            Error(nTick, "Duplicate program change");
        }
        mProgramChanged = 1;
    } else if (nType == kStatusControlChange) {
        if (!mControllers.insert(nData1).second) {
            Error(nTick, FormatString("Duplicate controller message: %i", nData1));
        }
    }

    if (!mRequirePrograms || mOrderChecked) {
        return;
    }
    if (mFirstNoteTick == kNone && nType == kStatusNoteOn) {
        mFirstNoteTick = nTick;
    }
    if (mFirstProgramTick == kNone && nType == kStatusProgramChange) {
        mFirstProgramTick = nTick;
    }
    if (mFirstBankTick == kNone && nType == kStatusControlChange &&
        nData1 == kControllerBankSelect) {
        mFirstBankTick = nTick;
    }
    if (mFirstNoteTick != kNone && mFirstNoteTick < nTick) {
        mOrderChecked = 1;
        if (mFirstNoteTick < mFirstProgramTick || mFirstProgramTick == kNone) {
            Error(nTick, "There is no program change before first note");
        }
        if (mFirstNoteTick < mFirstBankTick || mFirstBankTick == kNone) {
            Error(nTick, "There is no bank change before first note");
        }
    }
}

void MidiValidator::Error(int nTick, const char *pszMessage) {
    // The prefix and the message strings are destroyed before the handler runs.
    String text = [&] {
        const String prefix(kErrorPrefix);
        const String message(pszMessage);
        return FormatMessage(prefix, nTick, mTrack, message);
    }();
    if (mErrorHandler != nullptr) {
        mErrorHandler(text);
    } else {
        DebugError(text.c_str()); // The message itself is the format.
    }
}
