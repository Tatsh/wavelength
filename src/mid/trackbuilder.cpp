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
    text.Print(prefix.c_str());
    PrnStream &stream = text << kTrackLabel << nTrack << kTickLabel;
    const MBT mbt(nTick, kBeatsPerMeasure, kTicksPerBeat);
    stream.Print(mbt.ToString().c_str());
    (stream << kMessageSeparator).Print(message.c_str());
    // Yes, the binary copies the text into the result rather than building it in place.
    return static_cast<const String &>(text);
}

TrackBuilder::TrackBuilder(int nTrack, bool bValidate, ErrorHandler pfnError)
    : mTrack(nTrack), mValidate(bValidate), mErrorHandler(pfnError) {
}

TrackBuilder::~TrackBuilder() {
}

void TrackBuilder::Error(int nTick, const char *pszMessage) {
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
