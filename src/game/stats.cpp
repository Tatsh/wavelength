#include "game/stats.h"

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "os/datetime.h"
#include "os/filestream.h"
#include "os/string.h"

namespace {

constexpr char kQuote[] = "\"";
constexpr char kNoQuote[] = "";
constexpr char kFieldOpen[] = "(";
constexpr char kSpace[] = " ";
constexpr char kFieldCloseLine[] = ")\n";
constexpr char kTrue[] = "TRUE";
constexpr char kFalse[] = "FALSE";
constexpr char kTimestampFormat[] = "_%02i%02i%02i_%02i%02i%02i";

// The years the console clock counts from 1900 that precede 2000.
constexpr int kYearsBefore2000 = 100;

constexpr char kPlayerLabel[] = "(player ";
constexpr char kTrackLabel[] = "(track ";
constexpr char kGemTickLabel[] = "(gem_tick ";
constexpr char kButtonTickLabel[] = "(button_tick ";
constexpr char kSlopMsLabel[] = "(slop_ms ";
constexpr char kOldTrackLabel[] = "(old_track ";
constexpr char kNewTrackLabel[] = "(new_track ";
constexpr char kTickLabel[] = "(tick ";
constexpr char kNumBarsLabel[] = "(num_bars ";
constexpr char kStartLabel[] = "(start ";
constexpr char kTypeLabel[] = "(type ";
constexpr char kScoreLabel[] = "(score ";
constexpr char kPctDoneLabel[] = "(pct_done ";
constexpr char kPctEnergizedLabel[] = "(pct_energized ";
constexpr char kFullMixBarsLabel[] = "(full_mix_bars ";
constexpr char kBestStreakLabel[] = "(best_streak ";
constexpr char kScoresLabel[] = "(scores ";
constexpr char kValueLabel[] = "(value ";
constexpr char kFieldSeparator[] = ") ";
constexpr char kFieldClose[] = ")";

constexpr char kStartTimeField[] = "start_time";
constexpr char kArenaField[] = "arena";
constexpr char kSongField[] = "song";
constexpr char kSkillLevelField[] = "skill_level";
constexpr char kPracticeModeField[] = "practice_mode";
constexpr char kNumPlayersField[] = "num_players";
constexpr char kCommunityField[] = "community";
constexpr char kRuleSetField[] = "ruleset";
constexpr char kSongBarsField[] = "song_bars";
constexpr char kEventsOpen[] = "(events\n";
constexpr char kEventIndent[] = "  ";

constexpr int kNoSlot = -1;
constexpr int kNoPlayer = -1;
constexpr int kNoValue = -1;

constexpr int kStart = 1;
constexpr int kEnd = 0;

constexpr bool kWriteFile = true;
constexpr bool kLittleEndian = true;
constexpr int kFileFlags = 0;
constexpr bool kSaveFlag = true;

const char *TrueFalse(int nValue) {
    return nValue ? kTrue : kFalse;
}

} // namespace

String Stats::GemHitEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kGemTickLabel << mGemTick << kFieldSeparator << kButtonTickLabel << mButtonTick
         << kFieldSeparator << kSlopMsLabel << mSlopMs << kFieldClose;
    return text;
}

String Stats::GemMissEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kButtonTickLabel << mButtonTick << kFieldSeparator;
    // Yes, the binary leaves the record unclosed when the press had no gem near it.
    if (mSlopMs != 0.0f) {
        text << kGemTickLabel << mGemTick << kFieldSeparator << kSlopMsLabel << mSlopMs
             << kFieldClose;
    }
    return text;
}

String Stats::GemPassEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kGemTickLabel << mTick << kFieldClose;
    return text;
}

String Stats::ChangeTrackEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kOldTrackLabel << mOldTrack
         << kFieldSeparator << kNewTrackLabel << mNewTrack << kFieldSeparator << kTickLabel << mTick
         << kFieldClose;
    return text;
}

String Stats::CaptureTrackEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kTickLabel << mTick << kFieldSeparator << kNumBarsLabel << mNumBars << kFieldClose;
    return text;
}

String Stats::CompleteStageEvent::ToString() const {
    String text;
    text << kTickLabel << mTick << kFieldClose;
    return text;
}

String Stats::AxeEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kTickLabel << mTick << kFieldSeparator << kStartLabel << TrueFalse(mStart)
         << kFieldClose;
    return text;
}

String Stats::ScratchEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTrackLabel << mTrack << kFieldSeparator
         << kTickLabel << mTick << kFieldSeparator << kStartLabel << TrueFalse(mStart)
         << kFieldClose;
    return text;
}

String Stats::CatchPowerupEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTickLabel << mTick << kFieldSeparator
         << kTypeLabel << mType << kFieldClose;
    return text;
}

String Stats::DeployPowerupEvent::ToString() const {
    String text;
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTickLabel << mTick << kFieldSeparator
         << kTypeLabel << mType << kFieldClose;
    return text;
}

String Stats::LoseSoloGameEvent::ToString() const {
    String text;
    text << kTickLabel << mTick << kFieldSeparator << kScoreLabel << mScore << kFieldSeparator
         << kPctDoneLabel << mProgress << kFieldClose;
    return text;
}

String Stats::WinSoloGameEvent::ToString() const {
    String text;
    text << kScoreLabel << mScore << kFieldSeparator << kPctEnergizedLabel << mEnergized
         << kFieldSeparator << kFullMixBarsLabel << mFullMixBars << kFieldSeparator
         << kBestStreakLabel << mBestStreak << kFieldClose;
    return text;
}

String Stats::EndMultiGameEvent::ToString() const {
    String text;
    text << kScoresLabel << mScores[0] << kSpace << mScores[1] << kSpace << mScores[2] << kSpace
         << mScores[3] << kFieldClose;
    return text;
}

String Stats::PlayerAbortedEvent::ToString() const {
    String text;
    // Yes, the binary leaves the record unclosed.
    text << kPlayerLabel << mPlayer << kFieldSeparator << kTickLabel << mTick << kFieldSeparator;
    return text;
}

String Stats::JuiceEvent::ToString() const {
    String text;
    text << kValueLabel << mValue << kFieldSeparator << kTickLabel << mTick << kFieldClose;
    return text;
}

void Stats::StatsMemcardUser::OnFileSaved([[maybe_unused]] int nStatus) {
    TheStats->mLog.Clear();
}

String Stats::FormatField(const String &name, const char *pszValue, bool bQuoted) {
    const char *pszQuote = bQuoted ? kQuote : kNoQuote;
    String line;
    PrnStream &stream = line << kFieldOpen;
    stream.Print(name.c_str());
    stream << kSpace << pszQuote << pszValue << pszQuote << kFieldCloseLine;
    return line;
}

String Stats::FormatField(const String &name, int nValue) {
    String value;
    value << nValue;
    return FormatField(name, value.c_str(), false);
}

String Stats::FormatField(const String &name, bool bValue) {
    return FormatField(name, bValue ? kTrue : kFalse, false);
}

void Stats::AppendTimestamp(String &text) {
    DateTime date{};
    (void)date.ReadClock(); // Yes, the binary discards this call's result.
    text << FormatString(kTimestampFormat,
                         date.mYear - kYearsBefore2000,
                         date.mMonth + 1,
                         date.mDay,
                         date.mHour,
                         date.mMinute,
                         date.mSecond);
}

Stats::Stats()
    : mSongBars(kNoValue), mLogName(nullptr), mMemcardSlot(kNoSlot),
      mMemcardUser(new StatsMemcardUser), mActive(0) {
}

Stats::~Stats() {
    delete mMemcardUser;
}

Stats *Stats::shared() {
    static Stats instance;
    return &instance;
}

void Stats::End() {
    if (!mActive) {
        return;
    }
    mActive = 0;
    WriteSessionLog();
    if (mLogName) {
        String name(mLogName);
        if (mMemcardSlot >= 0) {
            AppendTimestamp(name);
            TheMCManager.SaveFile(
                mMemcardUser, mMemcardSlot, name.c_str(), mLog.c_str(), mLog.mLength, kSaveFlag);
        } else {
            AppendTimestamp(name);
            FileStream file(name.c_str(), kWriteFile, kLittleEndian, kFileFlags);
            file.Write(mLog.c_str(), mLog.mLength);
            mLog.Clear();
        }
    }
    for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
        delete *it;
    }
    std::vector<Event *>().swap(mEvents);
}

void Stats::GemHit(int nTrack, int nGemTick, int nButtonTick, float fSlopMs) {
    if (!mActive) {
        return;
    }
    int nPlayer = mTrackPlayers[nTrack];
    if (!mLogName) {
        return;
    }
    mEvents.push_back(new GemHitEvent(nPlayer, nTrack, nGemTick, nButtonTick, fSlopMs));
}

void Stats::GemMiss(int nTrack, int nGemTick, int nButtonTick, float fSlopMs) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(
        new GemMissEvent(mTrackPlayers[nTrack], nTrack, nGemTick, nButtonTick, fSlopMs));
}

void Stats::GemMiss(int nTrack, int nButtonTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new GemMissEvent(mTrackPlayers[nTrack], nTrack, 0, nButtonTick, 0.0f));
}

void Stats::GemPass(int nTrack, int nTick) {
    if (!mActive) {
        return;
    }
    int nPlayer = mTrackPlayers[nTrack];
    if (nPlayer == kNoPlayer || !mLogName) {
        return;
    }
    mEvents.push_back(new GemPassEvent(nPlayer, nTrack, nTick));
}

void Stats::SetTrackPlayer(int nTrack, int nPlayer) {
    if (mActive) {
        mTrackPlayers[nTrack] = nPlayer;
    }
}

void Stats::ChangeTrack(int nPlayer, int nTrack, int nTick) {
    if (!mActive) {
        return;
    }
    if (mLogName) {
        mEvents.push_back(new ChangeTrackEvent(nPlayer, mPlayerTracks[nPlayer], nTick, nTrack));
    }
    mPlayerTracks[nPlayer] = nTrack;
}

void Stats::CaptureTrack(int nTrack, int nTick, int nNumBars) {
    if (!mActive) {
        return;
    }
    int nPlayer = mTrackPlayers[nTrack];
    if (!mLogName) {
        return;
    }
    mEvents.push_back(new CaptureTrackEvent(nPlayer, nTrack, nTick, nNumBars));
}

void Stats::CompleteStage(int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new CompleteStageEvent(nTick));
}

void Stats::AxeBegin(int nTrack, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new AxeEvent(mTrackPlayers[nTrack], nTrack, nTick, kStart));
}

void Stats::AxeEnd(int nTrack, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new AxeEvent(mTrackPlayers[nTrack], nTrack, nTick, kEnd));
}

void Stats::ScratchBegin(int nTrack, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new ScratchEvent(mTrackPlayers[nTrack], nTrack, nTick, kStart));
}

void Stats::ScratchEnd(int nTrack, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new ScratchEvent(mTrackPlayers[nTrack], nTrack, nTick, kEnd));
}

void Stats::DeployPowerup(int nPlayer, int nTick, int nType) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new DeployPowerupEvent(nPlayer, nTick, nType));
}

void Stats::CatchPowerup(int nPlayer, int nTick, int nType) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new CatchPowerupEvent(nPlayer, nTick, nType));
}

void Stats::LoseSoloGame(int nTick, int nScore, float fProgress) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new LoseSoloGameEvent(nTick, nScore, fProgress));
}

void Stats::WinSoloGame(int nScore, int nFullMixBars, int nBestStreak, float fEnergized) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new WinSoloGameEvent(nScore, fEnergized, nFullMixBars, nBestStreak));
}

void Stats::EndMultiGame(int nScore0, int nScore1, int nScore2, int nScore3) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new EndMultiGameEvent(nScore0, nScore1, nScore2, nScore3));
}

void Stats::PlayerAborted(int nPlayer, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new PlayerAbortedEvent(nPlayer, nTick));
}

void Stats::Juice(float fValue, int nTick) {
    if (!mActive || !mLogName) {
        return;
    }
    mEvents.push_back(new JuiceEvent(fValue, nTick));
}

void Stats::WriteSessionLog() {
    mLog.Clear();
    {
        String name(kStartTimeField);
        String line = FormatField(name, mStartTime.c_str(), true);
        mLog.Print(line.c_str());
    }
    {
        String name(kArenaField);
        String line = FormatField(name, TheGameDb->mArena.c_str(), true);
        mLog.Print(line.c_str());
    }
    {
        String name(kSongField);
        String line = FormatField(name, TheGameDb->mSong.c_str(), true);
        mLog.Print(line.c_str());
    }
    {
        String name(kSkillLevelField);
        String line = FormatField(name, TheGameDb->mSkillLevel);
        mLog.Print(line.c_str());
    }
    {
        String name(kPracticeModeField);
        String line = FormatField(name, TheGameDb->mPracticeMode != 0);
        mLog.Print(line.c_str());
    }
    {
        String name(kNumPlayersField);
        String line = FormatField(name, TheGameDb->GetNumPlayers());
        mLog.Print(line.c_str());
    }
    {
        String name(kCommunityField);
        String line = FormatField(name, TheGameDb->mCommunity);
        mLog.Print(line.c_str());
    }
    {
        String name(kRuleSetField);
        String line = FormatField(name, TheGameDb->mRuleSet);
        mLog.Print(line.c_str());
    }
    {
        String name(kSongBarsField);
        String line = FormatField(name, mSongBars);
        mLog.Print(line.c_str());
    }
    mLog << kEventsOpen;
    for (unsigned int i = 0; i < mEvents.size(); ++i) {
        PrnStream &stream = mLog << kEventIndent;
        const Event *pEvent = mEvents[i];
        String name(pEvent->mName);
        String value = pEvent->ToString();
        String line = FormatField(name, value.c_str(), false);
        stream.Print(line.c_str());
    }
    mLog << kFieldCloseLine;
}

Stats *TheStats = Stats::shared();
