#include "netflow/ratingstat.h"

#include <algorithm>

#include "os/debug.h"

namespace {

// The rating of a player with no games.
constexpr int kUnratedRating = 10000;

// The bonus of a rating past the first games, and the bounds the rating is clamped to.
constexpr int kVeteranBonus = 1000000;
constexpr int kMaxRating = 90000;

// The weights of an opponent's games in a provisional rating.
constexpr int kMinOpponentWeight = 1;
constexpr int kMaxOpponentWeight = RatingStat::kProvisionalGames;

// The bound of the share of games won after a single game, and the half a share is centred on.
constexpr float kSingleGameShare = 0.333333f;
constexpr float kHalf = 0.5f;

// The Elo step of an opponent grows with the opponent's games up to kMaxStep.
constexpr float kMaxStep = 32.0f;
constexpr float kStepPerGame = 0.0625f;

// ExpectedScore() reads the table in steps of a hundredth of a game.
constexpr float kScorePerEntry = 0.01f;
constexpr float kEntriesPerScore = 100.0f;

// The rounding of an Elo step to whole points.
constexpr double kRounding = 0.5;

} // namespace

int RatingStat::sDifferences[kDifferenceCount] = {
    0,   4,   11,  18,  26,  33,  40,  47,  54,  62,  69,  77,  84,  92,  99,  107, 114,
    122, 130, 138, 146, 154, 163, 171, 180, 189, 198, 207, 216, 226, 236, 246, 257, 268,
    279, 291, 303, 316, 329, 345, 358, 375, 392, 412, 433, 457, 485, 518, 560, 620, 700,
};

int RatingStat::Rating() const {
    if (mGames == 0) {
        return kUnratedRating;
    }
    const int nRating = mRating - kVeteranBonus;
    return nRating < 0 ? mRating : nRating;
}

bool RatingStat::IsProvisional() const {
    return mGames < kProvisionalGames;
}

void LobbyPlayer::SetRating(const RatingStat *pStat) {
    mRating = pStat->mRating;
    mRecord.mGames = pStat->mGames;
    mRecord.mOpponents = pStat->mOpponents;
    mRecord.mWins = pStat->mWins;
    mRecord.mPoints = pStat->mPoints;
}

void RatingStat::Set(const NetLaunchpadPlayer *pPlayer) {
    mId = pPlayer->mId;
    mRating = pPlayer->mPlayer.mRating;
    mGames = pPlayer->mPlayer.mRecord.mGames;
    mWins = pPlayer->mPlayer.mRecord.mWins;
    mPoints = pPlayer->mPlayer.mRecord.mPoints;
    mOpponents = pPlayer->mPlayer.mRecord.mOpponents;
}

float RatingStat::Outcome(int nScore, int nOpponentScore) {
    if (nOpponentScore < nScore) {
        return 1.0f;
    }
    if (nScore < nOpponentScore) {
        return 0.0f;
    }
    return kHalf;
}

void RatingStat::Rate(int nId, const std::vector<RatingStat> *pStats) {
    for (auto it = pStats->begin(); it != pStats->end(); ++it) {
        if (it->mId == nId) {
            *this = *it;
            break;
        }
    }

    if (IsProvisional()) {
        for (const RatingStat &other : *pStats) {
            if (other.mId == nId) {
                continue;
            }
            const int nWeight =
                std::min(std::max(other.mGames, kMinOpponentWeight), kMaxOpponentWeight);
            mWins += Outcome(mScore, other.mScore) * static_cast<float>(nWeight);
            mPoints += other.Rating() * nWeight;
            ++mGames;
            mOpponents += nWeight;
        }
        if (mOpponents == 0) {
            return;
        }
        const float fShare = mWins / static_cast<float>(mOpponents);
        const float fAverage = static_cast<float>(mPoints) / static_cast<float>(mOpponents);
        float fLow = kHalf / static_cast<float>(mGames);
        if (mGames == 1) {
            fLow = kSingleGameShare;
        }
        const float fHigh = 1.0f - fLow;
        float fClamped = fShare < fLow ? fLow : fShare;
        fClamped = fHigh < fClamped ? fHigh : fClamped;
        DebugPrint("  ave wins:%.2f, ave rat:%.2f\n",
                   static_cast<double>(fClamped),
                   static_cast<double>(fAverage));
        mRating = static_cast<int>((fAverage + kHalf) + RatingDifference(fClamped));
    } else {
        int nRating = Rating();
        for (const RatingStat &other : *pStats) {
            if (other.mId == nId) {
                continue;
            }
            const float fOutcome = Outcome(mScore, other.mScore);
            const int nDifference = Rating() - other.Rating();
            const float fGrowth = static_cast<float>(other.mGames) * kMaxStep * kStepPerGame;
            const float fStep = fGrowth < kMaxStep ? fGrowth : kMaxStep;
            const float fChange = fStep * (fOutcome - ExpectedScore(nDifference));
            if (0.0f < fChange) {
                nRating += static_cast<int>(static_cast<double>(fChange) + kRounding);
            } else {
                nRating += static_cast<int>(static_cast<double>(fChange) - kRounding);
            }
        }
        mRating = nRating;
    }

    if (mRating < 0) {
        mRating = 0;
    } else if (kMaxRating < mRating) {
        mRating = kMaxRating;
    }
    if (!(mGames < kProvisionalGames)) {
        mRating += kVeteranBonus;
    }
}

float RatingStat::ExpectedScore(int nDifference) {
    float fSign = 1.0f;
    if (nDifference < 0) {
        nDifference = -nDifference;
        fSign = -1.0f;
    }
    const int *pEntry =
        std::upper_bound(&sDifferences[0], &sDifferences[kDifferenceCount], nDifference);
    const int nEntry = static_cast<int>(pEntry - &sDifferences[0]) - 1;
    const float fScore = (fSign * static_cast<float>(nEntry)) * kScorePerEntry + kHalf;
    return fScore < 1.0f ? fScore : 1.0f;
}

float RatingStat::RatingDifference(float fScore) {
    float fSign = 1.0f;
    if (fScore < kHalf) {
        fScore = 1.0f - fScore;
        fSign = -1.0f;
    }
    const float fPosition = (fScore - kHalf) * kEntriesPerScore;
    const int nEntry = static_cast<int>(fPosition);
    const float fFraction = fPosition - static_cast<float>(nEntry);
    if (nEntry == kDifferenceCount - 1) {
        return fSign * static_cast<float>(sDifferences[kDifferenceCount - 1]);
    }
    return fSign * (static_cast<float>(sDifferences[nEntry]) * (1.0f - fFraction) +
                    static_cast<float>(sDifferences[nEntry + 1]) * fFraction);
}
