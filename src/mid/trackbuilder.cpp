#include "mid/trackbuilder.h"

#include "mid/mbt.h"
#include "os/debug.h"

namespace {

constexpr char kErrorPrefix[] = "ERROR ";
constexpr char kTrackLabel[] = "(MIDI track ";
constexpr char kTickLabel[] = ", tick ";
constexpr char kMessageSeparator[] = "): ";

constexpr int kBeatsPerMeasure = 4;
constexpr int kTicksPerBeat = 480;

} // namespace

String
TrackBuilder::FormatMessage(const String &prefix, int nTick, int nTrack, const String &message) {
    String text;
    text << prefix.c_str();
    text << kTrackLabel << nTrack << kTickLabel;
    const MBT mbt(nTick, kBeatsPerMeasure, kTicksPerBeat);
    text << mbt.ToString().c_str();
    text << kMessageSeparator << message.c_str();
    return text;
}

TrackBuilder::TrackBuilder(int nTrack, bool bValidate, ErrorHandler pfnError)
    : mTrack(nTrack), mValidate(bValidate), mErrorHandler(pfnError) {
}

TrackBuilder::~TrackBuilder() {
}

void TrackBuilder::Error(int nTick, const char *pszMessage) {
    String text = FormatMessage(String(kErrorPrefix), nTick, mTrack, String(pszMessage));
    if (mErrorHandler != nullptr) {
        mErrorHandler(text);
    } else {
        DebugError(text.c_str()); // The message itself is the format.
    }
}
