#include "game/difficultyrating.h"

#include <algorithm>

#include "os/system.h"
#include "script/dataarray.h"

namespace {

constexpr double kMsPerMinute = 60000.0;
constexpr float kTicksPerBeat = 480.0f;

// The bar length the gap from the last gem of a bar to its end assumes, whatever mTicksPerBar is.
constexpr int kStandardBarTicks = 1920;

// The value of mLastTick before the first gem of a track.
constexpr int kNoTick = -kStandardBarTicks;
constexpr int kNoBar = -1;
constexpr int kNoLane = -1;

// The number of the hardest gaps of a bar that add to its difficulty.
constexpr int kHardestGaps = 3;

// The weight of the gap from the last gem of a bar to its end.
constexpr float kBarEndGapWeight = 2.0f;

// A bar difficulty and a track difficulty are recorded at this scale.
constexpr double kRecordedScale = 0.5;

constexpr char kGameSection[] = "game";
constexpr char kSkillThresholdsEntry[] = "skill_thresholds";
constexpr char kEmptyThresholdEntry[] = "empty_threshold";
constexpr int kNumSkillThresholds = 3;

// The difficulty of a gap of mGapMs, for gems in different lanes and in the same lane. A gap
// between two entries interpolates their difficulties linearly.
struct GapDifficultyEntry {
    int mGapMs;
    float mDifficulty[2];
};

// NTSC-U/C: 0x003af778
const GapDifficultyEntry kGapDifficulties[] = {
    {0, {40.0f, 100.0f}},
    {80, {40.0f, 120.0f}},
    {90, {27.0f, 80.0f}},
    {100, {20.0f, 50.0f}},
    {110, {16.0f, 40.0f}},
    {125, {13.0f, 30.0f}},
    {150, {9.0f, 20.0f}},
    {175, {7.0f, 12.0f}},
    {200, {6.0f, 9.0f}},
    {300, {5.0f, 7.0f}},
    {400, {4.0f, 5.5f}},
    {500, {3.0f, 4.0f}},
    {600, {2.5f, 3.0f}},
    {800, {2.0f, 2.2f}},
    {1000, {1.5f, 1.5f}},
    {1500, {1.0f, 1.0f}},
};
constexpr int kNumGapDifficulties = sizeof(kGapDifficulties) / sizeof(kGapDifficulties[0]);

} // namespace

DifficultyRating::DifficultyRating(float fTempo, int nNumBars, int nTicksPerBar)
    : mTempo(fTempo),
      mMsPerTick(static_cast<float>(kMsPerMinute / static_cast<double>(fTempo * kTicksPerBeat))),
      mNumBars(nNumBars), mTicksPerBar(nTicksPerBar), mDifficulty(0.0f), mNumTracks(0), mGemBars(0),
      mRateAverager(nTicksPerBar) {
    for (int i = 0; i < kNumSlots; ++i) {
        mBarDifficulties[i].resize(mNumBars);
    }
}

float DifficultyRating::GapDifficulty([[maybe_unused]] int nTick,
                                      bool bSameLane,
                                      float fGapMs) const {
    // The binary also tests the first entry, and a negative gap interpolates from the bytes
    // before the table.
    for (int i = 1; i < kNumGapDifficulties; ++i) {
        const GapDifficultyEntry &next = kGapDifficulties[i];
        if (fGapMs < static_cast<float>(next.mGapMs)) {
            const GapDifficultyEntry &previous = kGapDifficulties[i - 1];
            float fStart = previous.mDifficulty[bSameLane];
            return fStart + (fGapMs - static_cast<float>(previous.mGapMs)) /
                                static_cast<float>(next.mGapMs - previous.mGapMs) *
                                (next.mDifficulty[bSameLane] - fStart);
        }
    }
    return kGapDifficulties[kNumGapDifficulties - 1].mDifficulty[bSameLane];
}

void DifficultyRating::BeginTrack(int nSlot) {
    mSlot = nSlot;
    mLastLane = kNoLane;
    mLastTick = kNoTick;
    mBarGaps.clear();
    mTrackDifficulty = 0.0f;
    mBar = kNoBar;
    mTrackGemBars = 0;
}

void DifficultyRating::AddGem(int nLane, int nTick) {
    int nBar = nTick / mTicksPerBar;
    if (nBar != mBar && !mBarGaps.empty()) {
        ++mTrackGemBars;
        float fBarEndGapMs = static_cast<float>(static_cast<int>(
            static_cast<float>(mTicksPerBar - mLastTick % kStandardBarTicks) * mMsPerTick));
        float fBarDifficulty = GapDifficulty(0, false, fBarEndGapMs) * kBarEndGapWeight;
        std::sort(mBarGaps.begin(), mBarGaps.end());
        int nNumGaps = static_cast<int>(mBarGaps.size());
        for (unsigned int i = std::max(0, nNumGaps - kHardestGaps);
             i < static_cast<unsigned int>(nNumGaps);
             ++i) {
            fBarDifficulty += mBarGaps[i];
        }
        mBarGaps.clear();
        mTrackDifficulty += fBarDifficulty;
        mBarDifficulties[mSlot][mBar] = static_cast<int>(fBarDifficulty * kRecordedScale);
    }

    float fGapMs =
        static_cast<float>(static_cast<int>(static_cast<float>(nTick - mLastTick) * mMsPerTick));
    float fGapDifficulty = GapDifficulty(nTick, nLane == mLastLane, fGapMs);
    mBarGaps.push_back(fGapDifficulty);
    mBar = nBar;
    mLastTick = nTick;
    mLastLane = nLane;
    mTrackDifficulty += fGapDifficulty;
}

int DifficultyRating::EndTrack() {
    ++mNumTracks;
    mGemBars += mTrackGemBars;
    float fMean = 0.0f;
    if (mTrackGemBars != 0) {
        fMean = mTrackDifficulty / static_cast<float>(mTrackGemBars);
    }
    mDifficulty += fMean;
    return static_cast<int>(fMean * kRecordedScale);
}

int DifficultyRating::RateCatchTrack(const CatchTrackData *pData, int nSlot) {
    int nNumGems = pData->GetNumGems();
    BeginTrack(nSlot);
    for (int i = 0; i < nNumGems; ++i) {
        const Gem *pGem = pData->GetGem(i);
        AddGem(pGem->mLane, pGem->mTick);
    }
    return EndTrack();
}

int DifficultyRating::RatePitchTrack(PitchTrackGems *pGems) {
    BeginTrack(0);
    for (int i = 0; i < mNumBars; ++i) {
        std::vector<PitchGem> *pBar = pGems->GetBar(i);
        for (auto it = pBar->begin(); it != pBar->end(); ++it) {
            AddGem(it->mSlot, it->mTick);
        }
    }
    return EndTrack();
}

int DifficultyRating::GetSkillTier() {
    DataArray *pThresholds =
        SystemConfig()->FindArray(kGameSection, false)->FindArray(kSkillThresholdsEntry, false);
    float afThresholds[kNumSkillThresholds];
    for (int i = 0; i < kNumSkillThresholds; ++i) {
        afThresholds[i] = pThresholds->Float(i + 1);
    }
    float fDifficulty = GetDifficulty();
    for (int i = 0; i < kNumSkillThresholds; ++i) {
        if (fDifficulty < afThresholds[i]) {
            return i;
        }
    }
    return kNumSkillThresholds;
}

bool DifficultyRating::IsBelowEmptyThreshold() {
    float fThreshold = 0.0f;
    SystemConfig()
        ->FindArray(kGameSection, false)
        ->FindFloat(kEmptyThresholdEntry, &fThreshold, false);
    return GetEmptyFraction() < fThreshold;
}

float DifficultyRating::GetDifficulty() const {
    return mDifficulty;
}

float DifficultyRating::GetEmptyFraction() const {
    int nBars = mNumTracks * mNumBars;
    return static_cast<float>(nBars - mGemBars) / static_cast<float>(nBars);
}
