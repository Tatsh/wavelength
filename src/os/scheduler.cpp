#include "os/scheduler.h"

#include "os/bufstream.h"
#include "os/debug.h"
#include "os/factory.h"
#include "os/filestream.h"
#include "os/memcardsync.h"
#include "os/memstream.h"

namespace {

// The first byte of a command in a recording.
constexpr char kNoCommand = '0';
constexpr char kCommand = '1';

constexpr int kNoDevice = -1;
constexpr float kNormalSpeed = 1.0f;
constexpr float kStartTime = 0.0f;

constexpr char kCommandStreamError[] = "Stream error while reading in a Command object.";

} // namespace

bool Scheduler::CommandInfo::operator<(const CommandInfo &other) const {
    if (mTime < other.mTime) {
        return true;
    }
    return mTime == other.mTime && mRecordable && !other.mRecordable;
}

void Scheduler::CommandInfo::Save(BinStream &stream) const {
    stream << mCommand.Get();
    stream.WriteEndian(&mTime, sizeof(mTime));
    stream.WriteEndian(&mTick, sizeof(mTick));
    const char cRecordable = mRecordable;
    stream.Write(&cRecordable, sizeof(cRecordable));
}

void Scheduler::CommandInfo::Load(BinStream &stream) {
    Command *pCommand;
    stream >> pCommand;
    stream.ReadEndian(&mTime, sizeof(mTime));
    stream.ReadEndian(&mTick, sizeof(mTick));
    char cRecordable;
    stream.Read(&cRecordable, sizeof(cRecordable));
    mRecordable = cRecordable != 0;
    mCommand = Ptr<Command>(pCommand);
    mId = TheDefaultCommandId;
}

bool Scheduler::ByCommand::operator()(const CommandInfo &info) const {
    return info.mCommand.Get() == mCommand;
}

Scheduler::Recorder::~Recorder() {
    Flush();
    delete mStream;
}

void Scheduler::Recorder::Flush() {
    if (mDevice != kNoDevice) {
        Save();
    }
}

void Scheduler::Recorder::Record(const CommandInfo &info) {
    info.Save(*mStream);
    mStream->Flush();
}

void Scheduler::Recorder::Save() {
    mStream->Seek(0, BinStream::kSeekEnd);
    const int nSize = mStream->Tell();
    mStream->Seek(0, BinStream::kSeekBegin);
    char *pBuffer = new char[nSize];
    mStream->Read(pBuffer, nSize);
    MemcardGetInfoAndWait(mDevice, nullptr, nullptr, nullptr);
    const int nFlags = mParts != 0 ? kMemcardOpenWrite : (kMemcardOpenWrite | kMemcardOpenCreate);
    const int nFile = MemcardOpenAndWait(mDevice, mFile.c_str(), nFlags);
    if (nFile >= 0) {
        MemcardWriteAndWait(nFile, pBuffer, nSize);
        MemcardCloseAndWait(nFile);
    }
    delete mStream;
    mStream = new MemStream(true);
    delete[] pBuffer;
    ++mParts;
}

Scheduler::Playbacker::Playbacker(const char *pszFile, Scheduler *pScheduler)
    : mStream(new FileStream(pszFile, false, true, 0)), mScheduler(pScheduler) {
}

Scheduler::Playbacker::Playbacker(char *pBuffer, int nSize, Scheduler *pScheduler)
    : mStream(new BufStream(pBuffer, nSize, true)), mScheduler(pScheduler) {
}

Scheduler::Playbacker::~Playbacker() {
    delete mStream;
}

void Scheduler::Playbacker::QueueAll() {
    CommandInfo info{Ptr<Command>(), 0.0f, 0, TheDefaultCommandId, false};
    while (!mStream->Eof()) {
        info.Load(*mStream);
        mScheduler->Insert(info.mCommand.Get(), info.mTime, info.mTick, info.mId, info.mRecordable);
    }
}

Scheduler::Scheduler()
    // Retail allocates the set, stores mTime, then stores mCommands.
    : mCommands([this] {
          std::multiset<CommandInfo> *pCommands = new std::multiset<CommandInfo>;
          mTime = kStartTime;
          return pCommands;
      }()),
      mRecorder(nullptr), mPlaybacker(nullptr), mTickDuration(nullptr), mTick(0), mFrameTime(0.0f),
      mFrameTick(0), mPrevFrameTime(0.0f), mPrevFrameTick(0), mClock(mTime) {
}

Scheduler::~Scheduler() {
    mCommands->clear();
    delete mRecorder;
    delete mPlaybacker;
    delete mCommands;
}

void Scheduler::Reset(const float *pTickDuration, int nTick) {
    Pause();
    Clear();
    SetSpeed(kNormalSpeed);
    mTickDuration = pTickDuration;
    mClock.SetTime(static_cast<float>(nTick) * *pTickDuration);
    const float fTime = mClock.GetTime();
    mTime = fTime;
    mTick = nTick;
    mFrameTime = fTime;
    mPrevFrameTime = fTime;
    mFrameTick = nTick;
    mPrevFrameTick = nTick;
    delete mRecorder;
    mRecorder = nullptr;
    delete mPlaybacker;
    mPlaybacker = nullptr;
}

void Scheduler::ResetForRecording(const float *pTickDuration,
                                  [[maybe_unused]] const char *pszFile,
                                  [[maybe_unused]] int nDevice,
                                  int nTick) {
    Reset(pTickDuration, nTick); // The shipped build creates no Recorder.
}

void Scheduler::ResetForPlayback(const float *pTickDuration, const char *pszFile, int nTick) {
    Reset(pTickDuration, nTick);
    mPlaybacker = new Playbacker(pszFile, this);
    mPlaybacker->QueueAll();
}

void Scheduler::ResetForPlayback(const float *pTickDuration, char *pBuffer, int nSize, int nTick) {
    Reset(pTickDuration, nTick);
    mPlaybacker = new Playbacker(pBuffer, nSize, this);
    mPlaybacker->QueueAll();
}

void Scheduler::Resume() {
    if (!IsRunning()) {
        mClock.Start();
    }
}

void Scheduler::Pause() {
    if (IsRunning()) {
        mClock.Stop();
        if (mRecorder != nullptr) {
            mRecorder->Flush();
        }
    }
}

bool Scheduler::IsRunning() {
    return mClock.IsRunning() != 0;
}

void Scheduler::SetSpeed(float fSpeed) {
    mClock.SetRate(fSpeed);
}

float Scheduler::GetSpeed() {
    return mClock.GetRate();
}

void Scheduler::Insert(
    Command *pCommand, float fTime, int nTick, const CommandId &id, bool bRecordable) {
    const CommandInfo info{Ptr<Command>(pCommand), fTime, nTick, id, bRecordable};
    mCommands->insert(info);
}

void Scheduler::PostAtTime(Command *pCommand, float fTime, const CommandId &id, bool bRecordable) {
    Insert(pCommand, fTime, static_cast<int>(fTime / *mTickDuration), id, bRecordable);
}

void Scheduler::PostAtTime(Command *pCommand, float fTime, bool bRecordable) {
    PostAtTime(pCommand, fTime, TheDefaultCommandId, bRecordable);
}

void Scheduler::PostAfter(Command *pCommand, float fDelay, const CommandId &id, bool bRecordable) {
    PostAtTime(pCommand, mTime + fDelay, id, bRecordable);
}

void Scheduler::PostAfter(Command *pCommand, float fDelay, bool bRecordable) {
    PostAfter(pCommand, fDelay, TheDefaultCommandId, bRecordable);
}

void Scheduler::PostAt(Command *pCommand, int nTick, const CommandId &id, bool bRecordable) {
    Insert(pCommand, static_cast<float>(nTick) * *mTickDuration, nTick, id, bRecordable);
}

void Scheduler::PostAt(Command *pCommand, int nTick, bool bRecordable) {
    PostAt(pCommand, nTick, TheDefaultCommandId, bRecordable);
}

void Scheduler::PostIn(Command *pCommand,
                       int nDelayTicks,
                       [[maybe_unused]] const CommandId &id,
                       bool bRecordable) {
    // Yes, the binary posts with TheDefaultCommandId and ignores id.
    PostAt(pCommand, mTick + nDelayTicks, TheDefaultCommandId, bRecordable);
}

void Scheduler::PostIn(Command *pCommand, int nDelayTicks, bool bRecordable) {
    PostIn(pCommand, nDelayTicks, TheDefaultCommandId, bRecordable);
}

float Scheduler::GetClockTime() {
    return mClock.GetTime();
}

int Scheduler::GetClockTick() {
    return static_cast<int>(mClock.GetTime() / *mTickDuration);
}

void Scheduler::Clear() {
    mCommands->clear();
}

void Scheduler::Cancel(Command *pCommand) {
    RemoveIf(ByCommand(pCommand));
}

void Scheduler::Poll() {
    if (mClock.IsRunning() == 0) {
        return;
    }
    mPrevFrameTick = mFrameTick;
    mPrevFrameTime = mFrameTime;
    mFrameTime = mClock.GetTime();
    mFrameTick = static_cast<int>(mFrameTime / *mTickDuration);
    for (;;) {
        auto it = mCommands->begin();
        if (it == mCommands->end() || mFrameTime < it->mTime) {
            break;
        }
        if (mRecorder != nullptr && it->mRecordable) {
            mRecorder->Record(*it);
        }
        Ptr<Command> command(it->mCommand);
        mTime = it->mTime;
        mTick = it->mTick;
        mCommands->erase(it);
        command->Execute();
    }
    mTime = mFrameTime;
    mTick = mFrameTick;
}

void Scheduler::RemoveIf(const CancelPred &pred) {
    auto it = mCommands->begin();
    while (it != mCommands->end()) {
        if (pred(*it)) {
            (void)it->mCommand.Get(); // Yes, the binary discards this call's result.
            mCommands->erase(it++);
        } else {
            ++it;
        }
    }
}

BinStream &operator<<(BinStream &stream, Command *pCommand) {
    if (pCommand == nullptr) {
        stream.Write(&kNoCommand, sizeof(kNoCommand));
        return stream;
    }
    (void)pCommand->IsSerializable(); // Yes, the binary discards this call's result.
    stream.Write(&kCommand, sizeof(kCommand));
    const int nId = pCommand->GetSerialId();
    stream.WriteEndian(&nId, sizeof(nId));
    pCommand->Save(stream);
    return stream;
}

BinStream &operator>>(BinStream &stream, Command *&pCommand) {
    char cKind;
    stream.Read(&cKind, sizeof(cKind));
    (void)stream.Eof(); // Yes, the binary discards this call's result.
    if (cKind == kNoCommand) {
        pCommand = nullptr;
    } else if (cKind == kCommand) {
        int nId;
        stream.ReadEndian(&nId, sizeof(nId));
        pCommand = Factory<Command>::GetUnits()[nId]->Create(stream);
        (void)pCommand->GetSerialId(); // Yes, the binary discards this call's result.
    } else {
        DebugWarn(kCommandStreamError);
    }
    return stream;
}
