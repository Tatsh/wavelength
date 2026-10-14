#include "game/world.h"

#include <algorithm>
#include <climits>

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "math/rand.h"
#include "met/transitionmusic.h"
#include "netflow/netlaunchpad.h"
#include "netflow/nettransport.h"
#include "os/command.h"
#include "os/datetime.h"
#include "os/mem.h"
#include "os/memcardsync.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/serializablecommand.h"
#include "os/string.h"
#include "os/system.h"
#include "synth/songspeed.h"

namespace {

constexpr char kGameSection[] = "game";
constexpr char kRecordToMemcardKey[] = "record_to_memcard";
constexpr char kRecordingSuffixFormat[] = "_%02i%02i%02i_%02i%02i%02i";
constexpr char kNumberSeparator[] = "_";
constexpr char kDemoBufferTag[] = "attract file buf";
// The alignment argument of PoolMemAlloc() that selects the default.
constexpr int kDefaultAlignment = 0;
constexpr char kNoMessage[] = "";

// The last random seed a world draws for its logic.
constexpr int kMaxSeed = INT_MAX;

// The skill level a tutorial song is built at.
constexpr int kTutorialSkillLevel = 0;

// The File::New() mode that opens a file for reading.
constexpr int kOpenRead = 1;
constexpr int kNoOpenFlags = 0;

// The value of `record_to_memcard` that records to the host.
constexpr int kHostDevice = -1;

// The listing MemcardFileExists() requests.
constexpr int kMaxListEntries = 16;
constexpr int kNewListing = 0;

// DateTime::mYear counts from 1900 and DateTime::mMonth from 0.
constexpr int kCenturyYears = 100;
constexpr int kFirstMonth = 1;

// The ticks before the lead at which the track display starts.
constexpr int kTrackLeadTicks = 50;

constexpr float kScheduledArgument = -1.0f;
constexpr int kAllPlayers = -1;
constexpr float kQuitMessageDurationMs = 1600.0f;
constexpr float kQuitMessageScale = 1.0f;
constexpr float kNoOffset = 0.0f;
constexpr float kNoProgress = 0.0f;
constexpr float kStartTime = 0.0f;

// The time SystemMs() reported when the last world started loading. The value is not read.
// NTSC-U/C: 0x003af83c
float gWorldLoadStartMs;

// The identifier a recording saves the input command of each event under.
template <typename Event>
constexpr int kInputCmdId = 0;
template <>
constexpr int kInputCmdId<RotateEvent> = 0;
template <>
constexpr int kInputCmdId<PlayNoteEvent> = 1;
template <>
constexpr int kInputCmdId<ChangeSectionEvent> = 7;
template <int kId>
constexpr int kInputCmdId<BtnEvent<kId>> = kId;
template <int kId>
constexpr int kInputCmdId<StickEvent<kId>> = kId;

// A controller event the scheduler delivers to the logic, so that a recording of the song clock
// includes it. The unit's static initialiser at NTSC-U/C: 0x001458e0, PAL: 0x00147270, and its
// global constructor at NTSC-U/C: 0x001469f8, PAL: 0x00148388, register each instance in
// Factory<Command>, and the routine at NTSC-U/C: 0x00145830, PAL: 0x001471c0, destroys the
// registry at exit.
template <typename Event>
class InputCmd : public SerializableCommand<kInputCmdId<Event>, InputCmd<Event>> {
public:
    explicit InputCmd(const Event &event) : mEvent(event) {
    }

    explicit InputCmd(BinStream &stream) : mEvent(stream) {
    }

    // One copy for each event, in the order RotateEvent, PlayNoteEvent, BtnEvent<8>,
    // StickEvent<2>, StickEvent<6>, BtnEvent<3>, BtnEvent<4>, BtnEvent<5>, ChangeSectionEvent,
    // BtnEvent<9>, and BtnEvent<10>. The other members list their copies in the same order.
    // NTSC-U/C: 0x00345770, PAL: 0x003b2ca8
    // NTSC-U/C: 0x003457e8, PAL: 0x003b2d20
    // NTSC-U/C: 0x00345860, PAL: 0x003b2d98
    // NTSC-U/C: 0x003458d8, PAL: 0x003b2e10
    // NTSC-U/C: 0x00345950, PAL: 0x003b2e88
    // NTSC-U/C: 0x003459c8, PAL: 0x003b2f00
    // NTSC-U/C: 0x00345a40, PAL: 0x003b2f78
    // NTSC-U/C: 0x00345ab8, PAL: 0x003b2ff0
    // NTSC-U/C: 0x00345b30, PAL: 0x003b3068
    // NTSC-U/C: 0x00345ba8, PAL: 0x003b30e0
    // NTSC-U/C: 0x00345c20, PAL: 0x003b3158
    ~InputCmd() override = default;

    // NTSC-U/C: 0x00346590, PAL: 0x003b39d8
    // NTSC-U/C: 0x00346508, PAL: 0x003b3950
    // NTSC-U/C: 0x00346448, PAL: 0x003b38b0
    // NTSC-U/C: 0x00346310, PAL: 0x003b3790
    // NTSC-U/C: 0x003461d8, PAL: 0x003b3670
    // NTSC-U/C: 0x00346118, PAL: 0x003b35d0
    // NTSC-U/C: 0x00346058, PAL: 0x003b3530
    // NTSC-U/C: 0x00345f98, PAL: 0x003b3490
    // NTSC-U/C: 0x00345f10, PAL: 0x003b3408
    // NTSC-U/C: 0x00345e50, PAL: 0x003b3368
    // NTSC-U/C: 0x00345d90, PAL: 0x003b32c8
    void Execute() override {
        TheWorldLogic->HandleInput(mEvent);
    }

    // NTSC-U/C: 0x003465d8, PAL: 0x003b3a20
    // NTSC-U/C: 0x00346550, PAL: 0x003b3998
    // NTSC-U/C: 0x00346490
    // NTSC-U/C: 0x00346358, PAL: 0x003b37d8
    // NTSC-U/C: 0x00346220, PAL: 0x003b36b8
    // NTSC-U/C: 0x00346160
    // NTSC-U/C: 0x003460a0
    // NTSC-U/C: 0x00345fe0
    // NTSC-U/C: 0x00345f58, PAL: 0x003b3450
    // NTSC-U/C: 0x00345e98
    // NTSC-U/C: 0x00345dd8
    void Save(BinStream &stream) const override {
        mEvent.Save(stream);
    }

    // NTSC-U/C: 0x003465f8, PAL: 0x003b3a40
    // NTSC-U/C: 0x00346570, PAL: 0x003b39b8
    // NTSC-U/C: 0x003464d0
    // NTSC-U/C: 0x003463e0, PAL: 0x003b3850
    // NTSC-U/C: 0x003462a8, PAL: 0x003b3730
    // NTSC-U/C: 0x003461a0
    // NTSC-U/C: 0x003460e0
    // NTSC-U/C: 0x00346020
    // NTSC-U/C: 0x00345f78, PAL: 0x003b3470
    // NTSC-U/C: 0x00345ed8
    // NTSC-U/C: 0x00345e18
    void Load(BinStream &stream) override {
        mEvent.Load(stream);
    }

    // Post the event to run at once while the logic runs and the clock runs.
    // NTSC-U/C: 0x003447c8, PAL: 0x003b1d00
    // NTSC-U/C: 0x00344868, PAL: 0x003b1da0
    // NTSC-U/C: 0x00344918, PAL: 0x003b1e50
    // NTSC-U/C: 0x003449b0, PAL: 0x003b1ee8
    // NTSC-U/C: 0x00344a58, PAL: 0x003b1f90
    // NTSC-U/C: 0x00344b00, PAL: 0x003b2038
    // NTSC-U/C: 0x00344b98, PAL: 0x003b20d0
    // NTSC-U/C: 0x00344c30, PAL: 0x003b2168
    // NTSC-U/C: 0x00344cc8, PAL: 0x003b2200
    // NTSC-U/C: 0x00344d68, PAL: 0x003b22a0
    // NTSC-U/C: 0x00344e00, PAL: 0x003b2338
    static void Post(const Event &event) {
        if (!TheWorldLogic->IsFinished() && TheSongScheduler.IsRunning()) {
            TheSongScheduler.PostIn(new InputCmd(event), 0, true);
        }
    }

    Event mEvent;
};

// Append `_` and a number to a host file name until no file of that name exists.
// NTSC-U/C: 0x00144038, PAL: 0x001459c8
void MakeUniqueHostName(String &file) {
    const String base(file);
    int nNumber = 0;
    File *pExisting;
    while ((pExisting = File::New(file.c_str(), kOpenRead, kNoOpenFlags)) != nullptr) {
        delete pExisting;
        ++nNumber;
        file = base;
        file << kNumberSeparator << nNumber;
    }
}

// Report whether a file of a name exists on the memory card of a device.
// NTSC-U/C: 0x00144100, PAL: 0x00145a90
bool MemcardFileExists(int nDevice, const char *pszName) {
    MemcardDirEntry *pEntries;
    // Yes, the binary tests a result that is always -1.
    return MemcardGetDirAndWait(nDevice, pszName, kMaxListEntries, kNewListing, &pEntries) > 0;
}

// Append `_` and a number to a memory card file name until no file of that name exists.
// NTSC-U/C: 0x00144128, PAL: 0x00145ab8
void MakeUniqueMemcardName(int nDevice, String &file) {
    MemcardGetInfoAndWait(nDevice, nullptr, nullptr, nullptr);
    int nNumber = 0;
    const String base(file);
    while (MemcardFileExists(nDevice, file.c_str())) {
        file = base;
        ++nNumber;
        file << kNumberSeparator << nNumber;
    }
}

} // namespace

WorldLogic *TheWorldLogic;

World::World() {
    mState = kStateIdle;
    mSong = nullptr;
    mSongConfig = nullptr;
    mSeed = RandomInt(0, kMaxSeed);
    mLoadStep = kLoadStepSong;
    mDemoFile = nullptr;
    mDemoSize = 0;
    mDemoBuffer = nullptr;
    mLeadTicks = 0;
    TheWorldLogic = nullptr;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        TheNetTransport->SetSink(this);
    }
}

World::~World() {
    TheSongScheduler.Pause();
    TheSongScheduler.Clear();
    delete TheWorldLogic;
    delete mSong;
    delete mDemoFile;
    PoolMemFree(mDemoBuffer);
    TheWorldLogic = nullptr;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        TheNetTransport->SetSink(nullptr);
    }
}

void World::SetLogic(WorldLogic *pLogic) {
    delete TheWorldLogic;
    TheWorldLogic = pLogic;
}

void World::DeleteLogic() {
    SetLogic(nullptr);
}

void World::Start() {
    const int nStartTick = -mLeadTicks - kTrackLeadTicks;
    TheGameDb->mSongTick = static_cast<float>(-mLeadTicks);
    TheGameDb->mSongTime = static_cast<float>(-mLeadTicks) * *mSong->GetMsPerTick();
    BuildTracks(nStartTick);
    StartClock(nStartTick);
    CreateLogic();
    TheSongScheduler.PostAt(NewMemFunCommand(this, &World::OnSongStart), -mLeadTicks, false);
    OnStart();
    mState = kStatePlaying;
}

void World::Stop() {
    TheWorldLogic->Stop();
    delete mDemoFile;
    mDemoFile = nullptr;
    if (mDemoBuffer != nullptr) {
        PoolMemFree(mDemoBuffer);
        mDemoBuffer = nullptr;
    }
}

void World::OnSongStart() {
    TheWorldLogic->Start();
    if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        return;
    }
    TheNetTransport->SetSink(TheWorldLogic);
    if (!mGameEndedMsgs.empty()) {
        TheWorldLogic->Dispatch(mGameEndedMsgs.front());
    } else {
        for (unsigned int i = 0; i < mPlayerAbortedMsgs.size(); ++i) {
            TheWorldLogic->Dispatch(mPlayerAbortedMsgs[i]);
        }
    }
    for (GameEndedMsg *pMsg : mGameEndedMsgs) {
        delete pMsg;
    }
    mGameEndedMsgs.clear();
    for (PlayerAbortedMsg *pMsg : mPlayerAbortedMsgs) {
        delete pMsg;
    }
    mPlayerAbortedMsgs.clear();
}

int World::IsRestartRequested() {
    if (mState != kStatePlaying) {
        return 0;
    }
    return TheWorldLogic->IsRestartRequested();
}

bool World::IsFreestyling(int nPlayer) {
    if (mState != kStatePlaying) {
        return false;
    }
    return TheWorldLogic->IsFreestyling(nPlayer);
}

void World::Poll() {
    switch (mState) {
    case kStateLoading:
        PollLoad();
        break;
    case kStatePlaying:
        PollPlaying();
        break;
    case kStateEnding:
        PollQuitEnding();
        break;
    case kStateRestarting:
        Restart();
        break;
    default:
        break;
    }
    TheSongScheduler.Poll();
}

void World::PollLoad() {
    (void)SystemMs(); // Yes, the binary advances the system clock and discards the time.
    switch (mLoadStep) {
    case kLoadStepDemo: {
        int nBytes;
        if (mDemoFile->ReadDone(&nBytes)) {
            delete mDemoFile;
            mDemoFile = nullptr;
            mLoadStep = kLoadStepSong;
            mSong->Load();
        }
        break;
    }
    case kLoadStepSong:
        mSong->PollLoad();
        if (mSong->IsLoaded()) {
            if (TheGameDb->mRuleSet == GameDb::kRuleSetGame && TheGameDb->mLoadRemix != 0) {
                mSong->ApplySavedRemix();
            }
            mLoadStep = kLoadStepTracks;
            mLeadTicks = mSong->mBuilder->mTicksPerBar * mSong->mIntroBars;
            BeginSongLoad();
            FadeOutSharedMusic();
        }
        break;
    case kLoadStepTracks:
        PollSongLoad();
        if (IsSongLoadDone()) {
            mLoadStep = kLoadStepMusic;
        }
        break;
    case kLoadStepMusic:
        if (!IsSharedMusicFading()) {
            mLoadStep = kLoadStepAssets;
            BeginAssetLoad();
        }
        break;
    case kLoadStepAssets:
        PollAssetLoad();
        if (IsAssetLoadDone()) {
            mSong->ApplySynthSettings();
            FinishLoad();
        }
        break;
    default:
        break;
    }
}

void World::FinishLoad() {
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        if (TheNetLaunchpad != nullptr) {
            TheNetLaunchpad->NotifyLoaded();
        }
        if (mGameEndedMsgs.empty()) {
            mLoadStep = kLoadStepSession;
            return;
        }
    }
    mState = kStateLoaded;
    mLoadStep = kLoadStepComplete;
}

void World::PollQuitEnding() {
    PollEnding();
    if (TheGfxManager.IsIntroFinished() && IsEndingDone()) {
        TheGfxManager.RestartIntro();
        mState = kStateRestarting;
    }
}

void World::PollPlaying() {
    TheWorldLogic->Poll();
    if (TheWorldLogic->IsFinished() && TheWorldLogic->HasQuit()) {
        BeginQuitEnding();
    }
}

void World::Restart() {
    mState = kStatePlaying;
    TheSongScheduler.Pause();
    TheSongScheduler.Clear();
    DeleteLogic();
    StartClock(-mLeadTicks);
    TheGameDb->mSongTick = static_cast<float>(-mLeadTicks);
    TheGameDb->mSongTime = static_cast<float>(-mLeadTicks) * *mSong->GetMsPerTick();
    TheGfxManager.SetActive(false);
    TheGfxManager.ClearAll();
    (void)TheGfxManager.Poll(kStartTime); // Yes, the binary discards the result.
    CreateLogic();
    TheWorldLogic->Start();
    mState = kStatePlaying;
}

void World::BeginQuitEnding() {
    TheGfxManager.ClearLanes();
    (void)TheGfxManager.StartIntro(kScheduledArgument); // Yes, the binary discards the result.
    TheGfxManager.ShowMessage(kNoMessage,
                              nullptr,
                              kAllPlayers,
                              kQuitMessageDurationMs,
                              kQuitMessageScale,
                              kNoOffset,
                              kNoOffset);
    BeginEnding();
    mState = kStateEnding;
}

bool World::IsLoaded() const {
    return mLoadStep == kLoadStepComplete;
}

void World::LoadAssets() {
    mState = kStateLoading;
    gWorldLoadStartMs = SystemMs();
    mSongConfig = TheGameDb->FindSong(TheGameDb->mSong.c_str());
    int nSkillLevel = TheGameDb->mSkillLevel;
    int nRuleSet = GetRuleSet();
    if (TheGameDb->mTutorial != 0) {
        nSkillLevel = kTutorialSkillLevel;
    }
    if (nRuleSet == GameDb::kRuleSetGame) {
        nRuleSet = TheGameDb->mLoadRemix != 0 ? GameDb::kRuleSetRemix : GameDb::kRuleSetGame;
    }
    mSong = new Song(mSongConfig, nSkillLevel, nRuleSet);
    if (TheGameDb->GetDemo() != nullptr) {
        mLoadStep = kLoadStepDemo;
        mDemoFile = File::New(TheGameDb->GetDemo(), kOpenRead, kNoOpenFlags);
        mDemoSize = mDemoFile->Size();
        mDemoBuffer =
            static_cast<char *>(PoolMemAlloc(mDemoSize, kDemoBufferTag, kDefaultAlignment));
        (void)mDemoFile->ReadAsync(mDemoBuffer, mDemoSize); // Yes, the binary discards the result.
    } else {
        mLoadStep = kLoadStepSong;
        mSong->Load();
    }
}

int World::GetTick() {
    if (mState != kStatePlaying) {
        return TheSongScheduler.GetClockTick();
    }
    return std::max(-mLeadTicks, TheWorldLogic->GetTick());
}

float World::GetTime() {
    if (mState != kStatePlaying) {
        return TheSongScheduler.GetClockTime();
    }
    return TheWorldLogic->GetTime();
}

float World::GetDisplayTime() {
    if (mState != kStatePlaying) {
        return kNoProgress;
    }
    return TheWorldLogic->GetProgress();
}

void World::StartClock(int nTick) {
    const float *pTickDuration = mSong->GetMsPerTick();
    if (TheGameDb->GetDemo() != nullptr) {
        TheSongScheduler.ResetForPlayback(pTickDuration, mDemoBuffer, mDemoSize, nTick);
    } else {
        int nDevice = kHostDevice;
        const char *pszRecordFile = TheGameConfig->mRecordToFile;
        const char *pszPlayFile = TheGameConfig->mPlayFromFile;
        // Yes, the binary discards the result. A missing key leaves the host.
        (void)SystemConfig()
            ->FindArray(kGameSection, false)
            ->FindInt(kRecordToMemcardKey, &nDevice, false);
        if (pszRecordFile != nullptr) {
            String file(pszRecordFile);
            DateTime now{};
            (void)now.ReadClock(); // Yes, the binary discards the result.
            file << FormatString(kRecordingSuffixFormat,
                                 now.mYear - kCenturyYears,
                                 now.mMonth + kFirstMonth,
                                 now.mDay,
                                 now.mHour,
                                 now.mMinute,
                                 now.mSecond);
            if (nDevice == kHostDevice) {
                MakeUniqueHostName(file);
            } else {
                MakeUniqueMemcardName(nDevice, file);
            }
            TheSongScheduler.ResetForRecording(pTickDuration, file.c_str(), nDevice, nTick);
        } else if (pszPlayFile != nullptr) {
            TheSongScheduler.ResetForPlayback(pTickDuration, pszPlayFile, nTick);
        } else {
            TheSongScheduler.Reset(pTickDuration, nTick);
        }
    }
    SetSongSpeed(mSong->GetSpeed());
    TheSongScheduler.Resume();
}

int World::OnStartGame(StartGameMsg *pMsg) {
    mState = kStateLoaded;
    mLoadStep = kLoadStepComplete;
    mSeed = pMsg->mSeed;
    return 0;
}

int World::OnGameEnded(GameEndedMsg *pMsg) {
    // The binary copy-constructs the message inline.
    mGameEndedMsgs.push_back(static_cast<GameEndedMsg *>(pMsg->Clone()));
    if (mState == kStateLoading && mLoadStep == kLoadStepSession) {
        mState = kStateLoaded;
        mLoadStep = kLoadStepComplete;
    }
    return 0;
}

int World::OnPlayerAborted(PlayerAbortedMsg *pMsg) {
    // The binary copy-constructs the message inline.
    mPlayerAbortedMsgs.push_back(static_cast<PlayerAbortedMsg *>(pMsg->Clone()));
    return 0;
}

bool World::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nStartGameMsgType) {
        return OnStartGame(static_cast<StartGameMsg *>(pMsg)) != 0;
    }
    if (nType == g_nPlayerAbortedMsgType) {
        return OnPlayerAborted(static_cast<PlayerAbortedMsg *>(pMsg)) != 0;
    }
    if (nType == g_nGameEndedMsgType) {
        return OnGameEnded(static_cast<GameEndedMsg *>(pMsg)) != 0;
    }
    return false;
}

void World::Record(const RotateEvent &event) {
    InputCmd<RotateEvent>::Post(event);
}

void World::Record(const PlayNoteEvent &event) {
    InputCmd<PlayNoteEvent>::Post(event);
}

void World::Record(const BtnEvent<8> &event) {
    InputCmd<BtnEvent<8>>::Post(event);
}

void World::Record(const StickEvent<2> &event) {
    InputCmd<StickEvent<2>>::Post(event);
}

void World::Record(const StickEvent<6> &event) {
    InputCmd<StickEvent<6>>::Post(event);
}

void World::Record(const BtnEvent<3> &event) {
    InputCmd<BtnEvent<3>>::Post(event);
}

void World::Record(const BtnEvent<4> &event) {
    InputCmd<BtnEvent<4>>::Post(event);
}

void World::Record(const BtnEvent<5> &event) {
    InputCmd<BtnEvent<5>>::Post(event);
}

void World::Record(const ChangeSectionEvent &event) {
    InputCmd<ChangeSectionEvent>::Post(event);
}

void World::Record(const BtnEvent<9> &event) {
    InputCmd<BtnEvent<9>>::Post(event);
}

void World::Record(const BtnEvent<10> &event) {
    InputCmd<BtnEvent<10>>::Post(event);
}
