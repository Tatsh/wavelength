#include "game/levelmidibuilder.h"

#include <algorithm>
#include <ctype.h>
#include <functional>
#include <stdlib.h>

#include "game/axetrackcontourbuilder.h"
#include "game/axetrackharmonybuilder.h"
#include "game/backmusicbuilder.h"
#include "game/bankloaderbuilder.h"
#include "game/catchtrackbuilder.h"
#include "game/conductorbuilder.h"
#include "game/gamedb.h"
#include "game/gem.h"
#include "game/pitchgem.h"
#include "game/pitchtrackbuilder.h"
#include "game/pitchtrackgems.h"
#include "game/remixinfo.h"
#include "game/scratchtrackbuilder.h"
#include "game/scripttrackbuilder.h"
#include "game/song.h"
#include "game/voxtrack.h"
#include "game/worldbeatbuilder.h"
#include "gs/musebuilder.h"
#include "mid/mbt.h"
#include "os/bufstream.h"
#include "os/debug.h"
#include "os/fileutil.h"
#include "os/loadfile.h"
#include "os/mem.h"
#include "os/system.h"

namespace {

constexpr char kIllegalTrackTypeWarning[] = "illegal track type";
constexpr char kFileMissingFormat[] = "File %s not found";
constexpr char kBufferTag[] = "LevelMidiBuilder buf";
constexpr char kNoWorldTrackError[] = "No WORLD track found";
constexpr char kNoBankTrackError[] = "No BANK track found";
constexpr char kLateTrackNameError[] = "Track name can only be specified at first tick";

constexpr char kIgnoreTrackPrefix[] = "IGNORE";
constexpr char kBankTrackPrefix[] = "BANK";
constexpr char kIntroTrackPrefix[] = "INTRO";
constexpr char kWorldTrackName[] = "WORLD";
constexpr char kSecondWorldTrackError[] = "Can't have more than one WORLD track";
constexpr char kBackgroundTrackPrefix[] = "BG";
constexpr char kScriptTrackPrefix[] = "SCRIPT";
constexpr char kUnknownTrackNameError[] = "Could not parse track type from track name ";
constexpr char kQuote[] = "\"";

constexpr char kImproperNameStart[] = "Improperly formatted track name \"";
constexpr char kImproperNameEnd[] = "\"; expected name like \"T1 CATCH:D:Drums\"";
constexpr char kPitchType[] = "PITCH";
constexpr char kPitchInGameError[] = "Pitch tracks not allowed in game mode";
constexpr char kVoxType[] = "VOX";
constexpr char kVoxInGameError[] = "Vox tracks not allowed in game mode";
constexpr char kScratchType[] = "SCRATCH";
constexpr char kScratchNameError[] = "Scratch tracks must be named SCRATCH1, SCRATCH2, or SCRATCH3";
constexpr char kScratchOrderError[] = "Scratch tracks must be in order";
constexpr char kAxeContourType[] = "AXE_CONTOUR";
constexpr char kAxeHarmonyType[] = "AXE_HARMONY";
constexpr char kAxeHarmonyOrderError[] =
    "Axe Harmony track must come immediately after Axe Contour track";
constexpr char kCatchType[] = "CATCH";
constexpr char kCatchModeError[] = "Catch tracks not allowed in duel or remix mode";
constexpr char kFreestyleOrderError[] = "Freestyle tracks must be after all catch tracks";
constexpr char kUnknownTypeStart[] = "Unknown game track type ";
constexpr char kInTrackName[] = " in track name ";
constexpr char kScratchConsecutiveError[] = "Scratch tracks must be consecutive";
constexpr char kTrackNumberOrderError[] = "Track numbers must be in order";
constexpr char kInstrumentCodeError[] = "Incorrect instrument code";
constexpr char kUnknownInstrumentStart[] = "Unrecognized instrument letter ";

constexpr char kBackMusicTag[] = "BackMusic";
constexpr char kScriptNameError[] = "Script track must look like this: \"SCRIPT_name\"\n";
constexpr char kSecondBankTrackError[] = "Can't have more than 1 BANK track";
constexpr char kGameKey[] = "game";
constexpr char kGameFxBankSlotKey[] = "game_fx_bank_slot";
constexpr char kUnknownTrackTypeWarning[] = "unknown track type";

constexpr char kMidiTrackText[] = "(MIDI track ";
constexpr char kTickText[] = ", tick ";
constexpr char kMessageText[] = "): ";
constexpr char kErrorPrefix[] = "ERROR ";
constexpr char kRemixBufferTag[] = "remix buf";

constexpr int kOpenRead = 1;
constexpr int kNoOpenFlags = 0;
constexpr int kDefaultAlign = 0;

// MidiReader::ReadFor() parses for this long each Poll().
constexpr float kParseMsPerPoll = 15.0f;

// The meta event that names a track.
constexpr unsigned char kTrackNameEvent = 3;

// The tick an error of the whole file or track reports.
constexpr int kNoTick = 0;

// The conductor track, and the first instrument track.
constexpr int kConductorTrack = 0;

// The position Format() gives an error at, in beats of a 4/4 bar.
constexpr int kBeatsPerBar = 4;
constexpr int kTicksPerBeat = 480;

// The parts of a game track name, `T1 CATCH:D:Drums`.
constexpr int kNumberPos = 1;
constexpr int kNumberEnd = 2;
constexpr int kTypePos = 3;
constexpr int kMinNameLength = 4;
constexpr int kMaxTrackNumber = 10;
constexpr char kNameSeparator = ' ';
constexpr char kCodeSeparator = ':';
constexpr int kInstrumentOffset = 1;
constexpr int kLabelSeparatorOffset = 2;
constexpr int kLabelOffset = 3;
constexpr int kScratchNumberPos = 7;
constexpr char kFirstScratch = '1';
constexpr unsigned int kNumScratchTracks = 3;
constexpr int kNoScratch = -1;
constexpr int kLastSharedScratch = 2;
constexpr char kGameTrackLetter = 'T';

// The instrument letters of a game track name and the instruments they select.
enum Instrument {
    kInstrumentDrums = 0,
    kInstrumentBass = 1,
    kInstrumentSynth = 2,
    kInstrumentGuitar = 3,
    kInstrumentVocals = 4,
    kInstrumentFx = 5,
};

constexpr char kDrumsLetter = 'D';
constexpr char kBassLetter = 'B';
constexpr char kSynthLetter = 'S';
constexpr char kGuitarLetter = 'G';
constexpr char kVocalsLetter = 'V';
constexpr char kFxLetter = 'F';

// The text a SCRIPT track name has before the name of its commands, and the separator before it.
constexpr int kScriptSeparatorPos = 6;
constexpr char kScriptSeparator = '_';
constexpr int kScriptNamePos = 7;

// CatchTrackBuilder settings of a catch track and a vocal track.
constexpr int kCatchFilterEffects = 0;
constexpr int kVoxFilterEffects = 1;
constexpr int kVoxSkill = 1;

// The first bank of the BANK track follows the game's effects bank.
constexpr int kBankSlotAfterFx = 1;

constexpr bool kLocalPlayer = true;

} // namespace

LevelMidiBuilder::Tracks::Tracks(int nType, CatchTrackData *pGems, int nInstrument) {
    mInstrument = nInstrument;
    mCatch = nullptr;
    mScratch = nullptr;
    mAxe = nullptr;
    mRiffs = nullptr;
    mVox = nullptr;
    mFlags = 0;
    if (nType == Song::kTrackTypeCatch) {
        mCatch = pGems;
    } else if (nType == Song::kTrackTypeVox) {
        mVox = pGems;
    } else {
        DebugWarn(kIllegalTrackTypeWarning);
    }
}

LevelMidiBuilder::Tracks::Tracks(ScratchTrackData *pScratch, int nInstrument) {
    mScratch = pScratch;
    mInstrument = nInstrument;
    mCatch = nullptr;
    mAxe = nullptr;
    mRiffs = nullptr;
    mVox = nullptr;
    mFlags = 0;
}

LevelMidiBuilder::Tracks::Tracks(AxeTrackData *pAxe, int nInstrument) {
    mAxe = pAxe;
    mInstrument = nInstrument;
    mCatch = nullptr;
    mScratch = nullptr;
    mRiffs = nullptr;
    mVox = nullptr;
    mFlags = 0;
}

LevelMidiBuilder::Tracks::Tracks(PitchTrackRiffData *pRiffs, int nInstrument) {
    mRiffs = pRiffs;
    mFlags = 1;
    mInstrument = nInstrument;
    mCatch = nullptr;
    mScratch = nullptr;
    mAxe = nullptr;
    mVox = nullptr;
}

void LevelMidiBuilder::Tracks::Free() {
    delete mCatch;
    delete mScratch;
    delete mAxe;
    delete mRiffs;
    delete mVox;
}

LevelMidiBuilder::LevelMidiBuilder(const char *pszMidiFile,
                                   int nDifficulty,
                                   int nRuleSet,
                                   int nNumBars,
                                   int nIntroBars,
                                   int nFreestyleEffectsBus,
                                   int nRemixEffectsBus,
                                   SectionBoundaries *pSections,
                                   float *pMsPerTick,
                                   SlotGrid *pSlotGrid)
    : mPlayMap(nullptr), mBankTrack(nullptr), mLyric(nullptr), mFXMgr(nullptr), mSpeed(1.0f),
      mBuffer(nullptr), mSize(0), mFile(nullptr), mReader(nullptr),
      mDirectory(FileGetPath(pszMidiFile)) {
    mMidiFile = pszMidiFile;
    mDifficulty = nDifficulty;
    mNumBars = nNumBars;
    mIntroBars = nIntroBars;
    mFreestyleEffectsBus = nFreestyleEffectsBus;
    mRemixEffectsBus = nRemixEffectsBus;
    mReadDone = 0;
    mDone = 0;
    mTicksPerBar = 0;
    mReceiver = new BuilderReceiver(this);
    mValidate = 0;
    mErrorHandler = nullptr;
    mTrackIndex = 0;
    mGameTrackCount = 0;
    mRuleSet = nRuleSet;
    mScratchIndex = kNoScratch;
    mSections = pSections;
    mMsPerTick = pMsPerTick;
    mInstrument = kInstrumentDrums;
    mTrackKind = kTrackKindNone;
    mSlotGrid = pSlotGrid;
    mHasWorldTrack = 0;
    mTrackBuilder = nullptr;
}

LevelMidiBuilder::~LevelMidiBuilder() {
    PoolMemFree(mBuffer);
    delete mFile;
    delete mReceiver;
    delete mReader;
    std::for_each(mTracks.begin(), mTracks.end(), std::mem_fn(&Tracks::Free));
    for (BackMusic *pMusic : mBackMusic) {
        delete pMusic;
    }
    for (ScriptTrackData *pScript : mScriptTracks) {
        delete pScript;
    }
    delete mPlayMap;
    delete mBankTrack;
    delete mFXMgr;
    delete mLyric;
}

void LevelMidiBuilder::StartLoad() {
    mErrorHandler = nullptr;
    mValidate = 0;
    OpenFile();
}

void LevelMidiBuilder::StartLoad(TrackBuilder::ErrorHandler pfnError) {
    mErrorHandler = pfnError;
    mValidate = 1;
    OpenFile();
}

void LevelMidiBuilder::OpenFile() {
    mFile = File::New(mMidiFile, kOpenRead, kNoOpenFlags);
    if (mFile == nullptr) {
        Error(kNoTick, String(FormatString(kFileMissingFormat, mMidiFile)));
    }
    // Yes, the binary reads the size of a file that was not found.
    mSize = mFile->Size();
    mBuffer = static_cast<char *>(PoolMemAlloc(mSize, kBufferTag, kDefaultAlign));
    mFile->ReadAsync(mBuffer, mSize);
    mDone = 0;
    mReadDone = 0;
}

void LevelMidiBuilder::Poll() {
    if (!mReadDone) {
        int nBytes;
        mReadDone = mFile->ReadDone(&nBytes);
        return;
    }
    if (mDone) {
        return;
    }
    if (mReader == nullptr) {
        mReader = new MidiReader(mBuffer, mSize, mReceiver);
        return;
    }
    mDone = !mReader->ReadFor(kParseMsPerPoll);
    if (!mDone) {
        return;
    }
    delete mReader;
    mReader = nullptr;
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
    delete mFile;
    mFile = nullptr;
}

int LevelMidiBuilder::IsDone() const {
    return mDone;
}

void LevelMidiBuilder::OnNewTrack([[maybe_unused]] unsigned char nTrack) {
    mTrackName.Clear();
    mTrackLabel.Clear();
    if (mTrackIndex == kConductorTrack) {
        mTrackKind = kTrackKindConductor;
        CreateTrackBuilder();
    } else {
        mTrackKind = kTrackKindNone;
    }
}

void LevelMidiBuilder::OnEndTrack() {
    if (mTrackBuilder != nullptr) {
        mTrackBuilder->OnEndTrack();
        delete mTrackBuilder;
        mTrackBuilder = nullptr;
    }
    if (mTrackIndex == kConductorTrack) {
        mLyric = new Lyric(mTicksPerBar, mNumBars);
    }
    ++mTrackIndex;
}

void LevelMidiBuilder::OnAllDone() {
    if (!mHasWorldTrack) {
        Error(kNoTick, String(kNoWorldTrackError));
    }
    if (mBankTrack == nullptr) {
        Error(kNoTick, String(kNoBankTrackError));
    }
}

void LevelMidiBuilder::OnMidi(int nTick,
                              unsigned char nStatus,
                              unsigned char nData1,
                              unsigned char nData2) {
    if (mTrackBuilder != nullptr) {
        mTrackBuilder->OnMidi(nTick, nStatus, nData1, nData2);
    }
}

void LevelMidiBuilder::OnTempo(int nTick, int nMicrosecondsPerBeat) {
    if (mTrackBuilder != nullptr) {
        mTrackBuilder->OnTempo(nTick, nMicrosecondsPerBeat);
    }
}

void LevelMidiBuilder::OnTimeSignature(int nTick, int nNumerator, int nDenominator) {
    if (mTrackBuilder != nullptr) {
        mTrackBuilder->OnTimeSignature(nTick, nNumerator, nDenominator);
    }
}

void LevelMidiBuilder::OnText(int nTick, const char *pszText, unsigned char nType) {
    if (nType == kTrackNameEvent && mTrackKind == kTrackKindNone) {
        if (nTick != 0) {
            Error(nTick, String(kLateTrackNameError));
        }
        mTrackName = pszText;
        ClassifyTrack(pszText);
        CreateTrackBuilder();
        return;
    }
    if (mTrackBuilder != nullptr) {
        mTrackBuilder->OnText(nTick, pszText, nType);
    }
}

void LevelMidiBuilder::ClassifyTrack(const char *pszName) {
    String name(pszName);
    std::transform(name.mBuffer, name.mBuffer + name.mLength, name.mBuffer, toupper);
    if (name.Find(kIgnoreTrackPrefix) == 0) {
        mTrackKind = kTrackKindIgnore;
    } else if (name.Find(kBankTrackPrefix) == 0) {
        mTrackKind = kTrackKindBank;
    } else if (name.Find(kIntroTrackPrefix) == 0) {
        mTrackKind = kTrackKindIntro;
    } else if (name == kWorldTrackName) {
        mTrackKind = kTrackKindWorld;
        if (mHasWorldTrack) {
            Error(kNoTick, String(kSecondWorldTrackError));
        }
        mHasWorldTrack = 1;
    } else if (name.Find(kBackgroundTrackPrefix) == 0) {
        mTrackKind = kTrackKindBackground;
    } else if (name.Find(kScriptTrackPrefix) == 0) {
        mTrackKind = kTrackKindScript;
    } else if (name[0] == kGameTrackLetter) {
        ParseGameTrackName(name);
    } else {
        const String message = String(kUnknownTrackNameError) + kQuote + pszName + kQuote;
        Error(kNoTick, message);
        mTrackKind = kTrackKindNone;
    }
}

void LevelMidiBuilder::ParseGameTrackName(const String &name) {
    const int nNumber = atoi(name.Substring(kNumberPos).c_str());
    if (!isdigit(name[kNumberPos]) || nNumber < 0 || nNumber >= kMaxTrackNumber ||
        name[kNumberEnd] != kNameSeparator || name.mLength < kMinNameLength) {
        String message;
        PrnStream &stream = message << kImproperNameStart;
        stream.Print(name.c_str());
        stream << kImproperNameEnd;
        Error(kNoTick, message);
        mTrackKind = kTrackKindNone;
        return;
    }

    String type;
    const int nColon = name.Find(kCodeSeparator, kNumberEnd);
    if (nColon == String::npos) {
        type = name.Substring(kTypePos);
    } else {
        type = name.Substring(kTypePos, nColon - kTypePos);
    }

    if (type == kPitchType) {
        if (mRuleSet == GameDb::kRuleSetGame) {
            Error(kNoTick, String(kPitchInGameError));
        }
        mTrackKind = kTrackKindPitch;
        ++mGameTrackCount;
    } else if (type == kVoxType) {
        if (mRuleSet == GameDb::kRuleSetGame) {
            Error(kNoTick, String(kVoxInGameError));
        }
        mTrackKind = kTrackKindVox;
        ++mGameTrackCount;
    } else if (type.Find(kScratchType) == 0) {
        const int nScratch = type[kScratchNumberPos] - kFirstScratch;
        if (static_cast<unsigned int>(nScratch) >= kNumScratchTracks) {
            Error(kNoTick, String(kScratchNameError));
        }
        if (nScratch != 0 && nScratch != mScratchIndex + 1) {
            Error(kNoTick, String(kScratchOrderError));
        }
        mScratchIndex = nScratch;
        mTrackKind = kTrackKindScratch;
        if (nScratch == 0) {
            ++mGameTrackCount;
        }
    } else if (type == kAxeContourType) {
        mTrackKind = kTrackKindAxeContour;
        ++mGameTrackCount;
    } else if (type == kAxeHarmonyType) {
        mTrackKind = kTrackKindAxeHarmony;
        if (mTracks.back().mAxe == nullptr) {
            Error(kNoTick, String(kAxeHarmonyOrderError));
        }
    } else if (type == kCatchType) {
        mTrackKind = kTrackKindCatch;
        if (mRuleSet != GameDb::kRuleSetGame) {
            Error(kNoTick, String(kCatchModeError));
        }
        if (mGameTrackCount != 0 && (GetTrackType(mGameTrackCount - 1) == Song::kTrackTypeScratch ||
                                     GetTrackType(mGameTrackCount - 1) == Song::kTrackTypeAxe)) {
            Error(kNoTick, String(kFreestyleOrderError));
        }
        ++mGameTrackCount;
    } else {
        const String message = String(kUnknownTypeStart) + kQuote + type + kQuote + kInTrackName +
                               kQuote + name + kQuote;
        Error(kNoTick, message);
        mTrackKind = kTrackKindNone;
    }

    if (mTrackKind != kTrackKindScratch &&
        static_cast<unsigned int>(mScratchIndex) < static_cast<unsigned int>(kLastSharedScratch)) {
        Error(kNoTick, String(kScratchConsecutiveError));
    }
    if (mGameTrackCount != nNumber) {
        Error(kNoTick, String(kTrackNumberOrderError));
    }

    if (mTrackKind != kTrackKindPitch && mTrackKind != kTrackKindCatch &&
        mTrackKind != kTrackKindVox && mTrackKind != kTrackKindScratch &&
        mTrackKind != kTrackKindAxeContour) {
        return;
    }
    if (nColon == String::npos) {
        Error(kNoTick, String(kInstrumentCodeError));
    }
    if (static_cast<unsigned int>(nColon + kLabelSeparatorOffset) >=
        static_cast<unsigned int>(name.mLength)) {
        Error(kNoTick, String(kInstrumentCodeError));
    }
    const char cInstrument = name[nColon + kInstrumentOffset];
    if (name[nColon + kLabelSeparatorOffset] != kCodeSeparator) {
        Error(kNoTick, String(kInstrumentCodeError));
    }
    mTrackLabel = name.Substring(nColon + kLabelOffset);
    mInstrument = kInstrumentDrums;
    switch (cInstrument) {
    case kDrumsLetter:
        mInstrument = kInstrumentDrums;
        break;
    case kBassLetter:
        mInstrument = kInstrumentBass;
        break;
    case kSynthLetter:
        mInstrument = kInstrumentSynth;
        break;
    case kGuitarLetter:
        mInstrument = kInstrumentGuitar;
        break;
    case kVocalsLetter:
        mInstrument = kInstrumentVocals;
        break;
    case kFxLetter:
        mInstrument = kInstrumentFx;
        break;
    default: {
        mTrackKind = kTrackKindNone;
        // Yes, the binary reports the first letter of the type, not the instrument letter.
        const String message = String(kUnknownInstrumentStart) + kQuote + type[0] + kQuote +
                               kInTrackName + kQuote + name + kQuote;
        Error(kNoTick, message);
        break;
    }
    }
}

void LevelMidiBuilder::CreateTrackBuilder() {
    const int nIntroTicks = mIntroBars * mTicksPerBar;
    const int nChannel = mGameTrackCount - 1;
    switch (mTrackKind) {
    case kTrackKindNone:
        if (!mValidate) {
            DebugWarn(kUnknownTrackTypeWarning);
        }
        break;
    case kTrackKindConductor:
        mTrackBuilder = new ConductorBuilder(mTrackIndex,
                                             mValidate,
                                             mErrorHandler,
                                             mNumBars,
                                             kTicksPerBeat,
                                             mMsPerTick,
                                             &mPlayMap,
                                             &mTicksPerBar);
        break;
    case kTrackKindCatch: {
        auto *pGems = new CatchTrackData(mNumBars * mTicksPerBar);
        mTracks.push_back(Tracks(Song::kTrackTypeCatch, pGems, mInstrument));
        mTrackBuilder = new CatchTrackBuilder(mTrackIndex,
                                              mValidate,
                                              mErrorHandler,
                                              nChannel,
                                              nIntroTicks,
                                              pGems,
                                              mLyric,
                                              kCatchFilterEffects,
                                              mDifficulty);
        break;
    }
    case kTrackKindPitch: {
        auto *pRiffs = new PitchTrackRiffData();
        mTracks.push_back(Tracks(pRiffs, mInstrument));
        mTrackBuilder = new PitchTrackBuilder(mTrackIndex,
                                              mValidate,
                                              mErrorHandler,
                                              mTrackLabel,
                                              nChannel,
                                              nIntroTicks,
                                              mNumBars * mTicksPerBar,
                                              pRiffs);
        break;
    }
    case kTrackKindVox: {
        auto *pGems = new CatchTrackData(mNumBars * mTicksPerBar);
        mTracks.push_back(Tracks(Song::kTrackTypeVox, pGems, mInstrument));
        mTrackBuilder = new CatchTrackBuilder(mTrackIndex,
                                              mValidate,
                                              mErrorHandler,
                                              nChannel,
                                              nIntroTicks,
                                              pGems,
                                              mLyric,
                                              kVoxFilterEffects,
                                              kVoxSkill);
        break;
    }
    case kTrackKindScratch: {
        ScratchTrackData *pScratch;
        if (mScratchIndex == 0) {
            pScratch = new ScratchTrackData(mNumBars * mTicksPerBar);
            mTracks.push_back(Tracks(pScratch, mInstrument));
        } else {
            pScratch = mTracks.back().mScratch;
        }
        mTrackBuilder = new ScratchTrackBuilder(mTrackIndex,
                                                mValidate,
                                                mErrorHandler,
                                                nChannel,
                                                mScratchIndex,
                                                mTicksPerBar,
                                                nIntroTicks,
                                                mFreestyleEffectsBus,
                                                pScratch);
        break;
    }
    case kTrackKindAxeContour: {
        auto *pAxe = new AxeTrackData(mNumBars * mTicksPerBar, mFreestyleEffectsBus);
        mTracks.push_back(Tracks(pAxe, mInstrument));
        mTrackBuilder = new AxeTrackContourBuilder(
            mTrackIndex, mValidate, mErrorHandler, nChannel, mTicksPerBar, nIntroTicks, pAxe);
        break;
    }
    case kTrackKindAxeHarmony:
        mTrackBuilder = new AxeTrackHarmonyBuilder(
            mTrackIndex, mValidate, mErrorHandler, nChannel, nIntroTicks, mTracks.back().mAxe);
        break;
    case kTrackKindBackground: {
        auto *pMusic = new BackMusic(mTrackName.c_str(), mIntroBars, mNumBars, mTicksPerBar);
        mBackMusic.push_back(pMusic);
        mTrackBuilder = new BackMusicBuilder(
            mTrackIndex, mValidate, mErrorHandler, mIntroBars, mNumBars, mTicksPerBar, pMusic);
        break;
    }
    case kTrackKindIntro:
        mIntroMuses.push_back(Ptr<Muse>());
        mTrackBuilder = new MuseBuilder(mTrackIndex, mValidate, mErrorHandler, &mIntroMuses.back());
        break;
    case kTrackKindWorld:
        mTrackBuilder =
            new WorldBeatBuilder(mTrackIndex, mValidate, mErrorHandler, nIntroTicks, &mWorldTrack);
        break;
    case kTrackKindScript:
        if (mTrackName[kScriptSeparatorPos] != kScriptSeparator) {
            Error(kNoTick, String(kScriptNameError));
        }
        mScriptTracks.push_back(new ScriptTrackData(mTrackName.c_str() + kScriptNamePos));
        mTrackBuilder = new ScriptTrackBuilder(
            mTrackIndex, mValidate, mErrorHandler, nIntroTicks, mScriptTracks.back());
        break;
    case kTrackKindBank: {
        if (mBankTrack != nullptr) {
            Error(kNoTick, String(kSecondBankTrackError));
        }
        int nFxSlot = 0;
        SystemConfig()->FindArray(kGameKey, false)->FindInt(kGameFxBankSlotKey, &nFxSlot, true);
        mBankTrack = new BankLoader(static_cast<unsigned short>(nFxSlot + kBankSlotAfterFx),
                                    mPlayMap,
                                    mIntroBars,
                                    mNumBars,
                                    mTicksPerBar);
        mTrackBuilder = new BankLoaderBuilder(mTrackIndex,
                                              mValidate,
                                              mErrorHandler,
                                              nIntroTicks,
                                              mTicksPerBar,
                                              &mDirectory,
                                              mBankTrack);
        break;
    }
    case kTrackKindIgnore:
    default:
        break;
    }
}

String
LevelMidiBuilder::FormatError(const String &prefix, int nTick, int nTrack, const String &message) {
    String text;
    text.Print(prefix.c_str());
    PrnStream &stream = text << kMidiTrackText << nTrack << kTickText;
    const MBT position(nTick, kBeatsPerBar, kTicksPerBeat);
    stream.Print(position.ToString().c_str());
    (stream << kMessageText).Print(message.c_str());
    return text;
}

void LevelMidiBuilder::Error(int nTick, const String &message) {
    const String text = FormatError(String(kErrorPrefix), nTick, mTrackIndex, message);
    if (mErrorHandler != nullptr) {
        String copy(text.c_str());
        mErrorHandler(copy);
    } else {
        DebugError(text.c_str());
    }
}

void LevelMidiBuilder::LoadRemix() {
    const char *pPacked = TheGameDb->GetRemixBuffer();
    const int nPackedSize = TheGameDb->GetRemixInfo()->mDataSize;
    const int nSize = GzipInflatedSize(pPacked, nPackedSize);
    auto *pData = static_cast<char *>(PoolMemAlloc(nSize, kRemixBufferTag, kDefaultAlign));
    GzipDecompressRamToRam(pPacked, nPackedSize, pData);
    BufStream stream(pData, nSize, true);
    unsigned char nVersion = 0; // Yes, the binary reads the version and never checks it.
    stream.Read(&nVersion, sizeof(nVersion));
    stream.ReadEndian(&mSpeed, sizeof(mSpeed));
    mFXMgr = new FXMgr(GetNumTracks() - 1, mRemixEffectsBus);
    mFXMgr->Load(stream);

    for (int i = 0; i < GetNumTracks(); ++i) {
        Tracks &track = mTracks[i];
        switch (GetTrackType(i)) {
        case Song::kTrackTypePitch: {
            PitchTrackGems gems(mSections, mSlotGrid, mNumBars, mTicksPerBar);
            track.mCatch = new CatchTrackData();
            gems.Load(stream);
            gems.ApplyPatterns();
            for (int nBar = 0; nBar < mNumBars; ++nBar) {
                for (const PitchGem &pitchGem : *gems.GetBar(nBar)) {
                    Muse *pRiff = track.mRiffs->GetRiff(pitchGem.mTick, pitchGem.mSlot);
                    const Gem gem{pitchGem.mSlot, pitchGem.mTick, Ptr<Muse>(pRiff->Clone())};
                    track.mCatch->AddGem(gem);
                }
            }
            delete track.mRiffs;
            track.mRiffs = nullptr;
            break;
        }
        case Song::kTrackTypeAxe:
        case Song::kTrackTypeScratch:
            break;
        case Song::kTrackTypeVox: {
            track.mCatch = track.mVox;
            track.mVox = nullptr;
            VoxTrack vox(track.mCatch,
                         mPlayMap,
                         mSections,
                         mSlotGrid,
                         i,
                         mIntroBars,
                         mNumBars,
                         mTicksPerBar);
            vox.Load(stream);
            break;
        }
        default:
            DebugWarn(kIllegalTrackTypeWarning);
            break;
        }
    }
    PoolMemFree(pData);
}

int LevelMidiBuilder::GetNumTracks() const {
    return static_cast<int>(mTracks.size());
}

int LevelMidiBuilder::GetTrackType(int nTrack) const {
    const Tracks &track = mTracks[nTrack];
    if (track.mCatch != nullptr) {
        return Song::kTrackTypeCatch;
    }
    if (track.mScratch != nullptr) {
        return Song::kTrackTypeScratch;
    }
    if (track.mAxe != nullptr) {
        return Song::kTrackTypeAxe;
    }
    return track.mRiffs != nullptr ? Song::kTrackTypePitch : Song::kTrackTypeVox;
}

int LevelMidiBuilder::GetTrackInstrument(int nTrack) const {
    return mTracks[nTrack].mInstrument;
}

CatchTrackData *LevelMidiBuilder::GetCatchTrackData(int nTrack) const {
    return mTracks[nTrack].mCatch;
}

ScratchTrackData *LevelMidiBuilder::GetScratchData(int nTrack) const {
    return mTracks[nTrack].mScratch;
}

AxeTrackData *LevelMidiBuilder::GetAxeData(int nTrack) const {
    return mTracks[nTrack].mAxe;
}

PitchTrackRiffData *LevelMidiBuilder::GetTrackRiffData(int nTrack) const {
    return mTracks[nTrack].mRiffs;
}

CatchTrackData *LevelMidiBuilder::GetVoxData(int nTrack) const {
    return mTracks[nTrack].mVox;
}

int LevelMidiBuilder::GetTrackFlags(int nTrack) const {
    return mTracks[nTrack].mFlags;
}

int LevelMidiBuilder::GetNumBackMusic() const {
    return static_cast<int>(mBackMusic.size());
}

BackMusic *LevelMidiBuilder::GetBackMusic(int nIndex) const {
    return mBackMusic[nIndex];
}

int LevelMidiBuilder::GetNumIntroMuses() const {
    return static_cast<int>(mIntroMuses.size());
}

Muse *LevelMidiBuilder::GetIntroMuse(int nIndex) const {
    return mIntroMuses[nIndex].Get();
}

ScriptTrackData *LevelMidiBuilder::FindScriptTrack(const char *pszName) const {
    for (ScriptTrackData *pScript : mScriptTracks) {
        if (pScript->mName == pszName) {
            return pScript;
        }
    }
    return nullptr;
}
