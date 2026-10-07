#include "game/catchtrackbuilder.h"

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/gem.h"
#include "math/rand.h"
#include "os/ptr.h"
#include "os/string.h"

namespace {

constexpr char kIntroNoteError[] = "no notes allowed in intro";
constexpr char kNotGemFormat[] =
    "Note %i is above %i, so it should correspond to a gem, but it does not.";
constexpr char kDownbeatError[] = "Gem should be on following downbeat";
constexpr char kSameTickError[] = "More than one gem at the same tick at the same skill level";
constexpr char kNoGemsError[] = "No gems in the track; are your gems on the wrong notes?";
constexpr char kNoSamplesError[] = "No samples in the track";
constexpr char kSampleFirstError[] = "Sample is before first gem";
constexpr char kNoSampleAtGemError[] = "No samples start on same tick as this gem";

// The meta event types of the lyrics.
constexpr unsigned char kMetaText = 1;
constexpr unsigned char kMetaLyric = 5;

// The tick an error that concerns the whole track is reported at.
constexpr int kTrackErrorTick = 0;

constexpr int kNoTick = -1;
constexpr int kNoLane = -1;
constexpr int kNoSkill = -1;

// The channel message types.
constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusNoteOff = 0x80;
constexpr unsigned char kStatusNoteOn = 0x90;
constexpr unsigned char kStatusControlChange = 0xb0;

// The effect depth controllers mFilterEffects leaves out.
constexpr unsigned char kFirstFilteredController = 93;
constexpr unsigned char kFilteredControllers = 2;

// The gem notes: two notes per lane, three lanes per skill level, from the first gem note on.
constexpr unsigned char kFirstGemNote = 96;
constexpr int kNotesPerLane = 2;
constexpr int kNotesPerSkill = 6;
constexpr int kNumSkills = 4;

// A gem this close before the end of a bar belongs on the downbeat of the next bar.
constexpr int kTicksPerBar = 1920;
constexpr int kLateGemTicks = 1861;

// The lane of the gem that ends the last gem's samples.
constexpr int kEndGemLane = 0;

// The lanes RandomInt() scrambles a gem into.
constexpr int kFirstLane = 0;
constexpr int kNumLanes = 3;

} // namespace

bool CatchTrackBuilder::DecodeGemNote(unsigned char nNote, int *pnLane, int *pnSkill) {
    const int nOffset = nNote - kFirstGemNote;
    if ((nOffset % kNotesPerLane) != 0) {
        return false;
    }
    *pnSkill = nOffset / kNotesPerSkill;
    if (*pnSkill >= kNumSkills) {
        return false;
    }
    *pnLane = (nOffset % kNotesPerSkill) / kNotesPerLane;
    return true;
}

CatchTrackBuilder::CatchTrackBuilder(const char *pszName,
                                     bool bValidate,
                                     ErrorHandler pfnError,
                                     int nChannel,
                                     int nIntroTicks,
                                     CatchTrackData *pData,
                                     Lyric *pLyric,
                                     int nFilterEffects,
                                     int nSkill)
    : TrackBuilder(pszName, bValidate, pfnError), mIntroTicks(nIntroTicks), mData(pData),
      mFilterEffects(nFilterEffects), mLyric(pLyric), mValidator(pszName, pfnError, nChannel, true),
      mSkill(nSkill), mLastNoteOnTick(kNoTick), mFirstSampleTick(kNoTick) {
}

void CatchTrackBuilder::OnMidi(int nTick,
                               unsigned char nStatus,
                               unsigned char nData1,
                               unsigned char nData2) {
    const unsigned char nType = nStatus & kStatusTypeMask;
    const bool bNoteOn = nType == kStatusNoteOn;
    if (mValidate) {
        if ((nTick < mIntroTicks) && (bNoteOn || (nType == kStatusNoteOff))) {
            Error(nTick, kIntroNoteError);
        }
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }
    if (bNoteOn) {
        mLastNoteOnTick = nTick;
    }
    if (AddGem(nTick, nStatus, nData1, nData2)) {
        return;
    }
    if (mFilterEffects != 0 && nType == kStatusControlChange &&
        static_cast<unsigned char>(nData1 - kFirstFilteredController) < kFilteredControllers) {
        return;
    }
    if (mFirstSampleTick == kNoTick && bNoteOn) {
        mFirstSampleTick = nTick;
    }
    mSamples.AddMidi(nTick, nStatus, nData1, nData2);
}

void CatchTrackBuilder::OnText(int nTick, const char *pszText, unsigned char nType) {
    if (nType == kMetaLyric || nType == kMetaText) {
        mLyric->AddLyric(nTick - mIntroTicks, pszText);
    }
}

bool CatchTrackBuilder::AddGem(int nTick,
                               unsigned char nStatus,
                               unsigned char nData1,
                               [[maybe_unused]] unsigned char nData2) {
    const unsigned char nType = nStatus & kStatusTypeMask;
    const bool bNoteOff = nType == kStatusNoteOff;
    const bool bNoteOn = nType == kStatusNoteOn;
    if (!bNoteOn && !bNoteOff) {
        return false;
    }
    if (nData1 < kFirstGemNote) {
        return false;
    }

    int nLane = kNoLane;
    int nSkill = kNoSkill;
    if (!DecodeGemNote(nData1, &nLane, &nSkill)) {
        Error(nTick, FormatString(kNotGemFormat, nData1, kFirstGemNote));
        return true;
    }
    if (nSkill != mSkill) {
        return true;
    }
    if (bNoteOn && mValidate && (nTick % kTicksPerBar) >= kLateGemTicks) {
        Error(nTick, kDownbeatError);
    }
    if (!mGems.empty() && mGems.back().mTick == nTick) {
        Error(nTick, kSameTickError);
        return true;
    }
    if (!bNoteOn) {
        return true;
    }

    if (g_bScrambleGems != 0 && TheGameDb->mCommunity != GameDb::kCommunityOnline &&
        TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        nLane = RandomInt(kFirstLane, kNumLanes);
    }
    GemData gem{nTick, nLane};
    mGems.push_back(gem);
    return true;
}

void CatchTrackBuilder::OnEndTrack() {
    if (mValidate) {
        if (mGems.empty()) {
            Error(kTrackErrorTick, kNoGemsError);
        }
        if (mFirstSampleTick == kNoTick) {
            Error(kTrackErrorTick, kNoSamplesError);
        }
        if (!mGems.empty() && mFirstSampleTick != kNoTick &&
            mGems.front().mTick != mFirstSampleTick) {
            Error(mFirstSampleTick, kSampleFirstError);
        }
    }

    GemData endGem{mLastNoteOnTick + 1, kEndGemLane};
    mGems.push_back(endGem);

    int nPrevTick = kNoTick;
    int nPrevLane = kNoLane;
    for (const auto &gem : mGems) {
        if (nPrevTick != kNoTick) {
            if (mValidate && mSamples.CountMessages(nPrevTick, nPrevTick + 1) <= 0) {
                Error(nPrevTick, kNoSampleAtGemError);
            }
            MultiMuse *pSamples = mSamples.Extract(nPrevTick, gem.mTick);
            const Gem data{nPrevLane, nPrevTick - mIntroTicks, Ptr<Muse>(pSamples)};
            mData->AddGem(data);
        }
        nPrevTick = gem.mTick;
        nPrevLane = gem.mLane;
    }
}
