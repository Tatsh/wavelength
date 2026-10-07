#include "synth/songspeed.h"

#include <cmath>

#include "os/scheduler.h"
#include "synth/synth.h"

namespace {

constexpr float kSemitonesPerOctave = 12.0f;
constexpr float kOctaveRatio = 2.0f;
constexpr int kMaxShiftSemitones = 48;
constexpr float kMaxBend = 8191.0f;
constexpr float kBendCenter = 8192.0f;
constexpr int kBendMsbShift = 7;
constexpr unsigned int kDataByteMask = 0x7f;

constexpr unsigned int kStatusControlChange = 0xb0;

// The controller that carries the pitch shift.
constexpr unsigned int kPitchShiftController = 91;

// Every channel except the last receives the shift.
constexpr unsigned int kNumShiftedChannels = 15;

constexpr int kControllerShift = 8;
constexpr int kValueShift = 16;

} // namespace

void SetSongSpeed(float fSpeed) {
    TheSongScheduler.SetSpeed(fSpeed);
    float fSemitones = std::log(fSpeed) * kSemitonesPerOctave / std::log(kOctaveRatio);
    float fLimit = static_cast<float>(kMaxShiftSemitones);
    if (fLimit < fSemitones) {
        fSemitones = fLimit;
    } else if (fSemitones < -fLimit) {
        fSemitones = -fLimit;
    }
    int nBend = static_cast<int>(fSemitones / static_cast<float>(kMaxShiftSemitones) * kMaxBend +
                                 kBendCenter);
    unsigned int nValue = (static_cast<unsigned int>(nBend) >> kBendMsbShift) & kDataByteMask;
    for (unsigned int nChannel = 0; nChannel < kNumShiftedChannels; ++nChannel) {
        TheSynth->SendPackedMessage((kStatusControlChange | nChannel) |
                                    (kPitchShiftController << kControllerShift) |
                                    (nValue << kValueShift));
    }
}
