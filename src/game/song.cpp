#include "game/song.h"

#include <algorithm>
#include <map>
#include <set>
#include <strings.h>

#include "game/duelpattern.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "os/bufstream.h"
#include "os/debug.h"
#include "os/fileutil.h"
#include "os/locale.h"
#include "synth/synth.h"

namespace {

constexpr char kNoName[] = "no name";
constexpr char kNoText[] = "";
constexpr char kErrorFormat[] = "ERROR in song text file: %s";
constexpr char kMissingString[] = "couldn't find string ";
constexpr char kMissingInteger[] = "couldn't find integer ";
constexpr char kMissingFloat[] = "couldn't find float ";
constexpr char kMissingArray[] = "couldn't find array ";

constexpr char kDuelSongBarsKey[] = "duel_song_bars";
constexpr char kRemixSongBarsKey[] = "remix_song_bars";
constexpr char kSongBarsKey[] = "song_bars";
constexpr char kIntroBarsKey[] = "intro_bars";
constexpr char kBpmKey[] = "bpm";
constexpr char kMidiFileKey[] = "midi_file";
constexpr char kRemixMidiFileKey[] = "remix_midi_file";
constexpr char kDuelMidiFileKey[] = "duel_midi_file";
constexpr char kIllegalModeWarning[] = "illegal mode";
constexpr char kPathSeparator[] = "/";
constexpr char kFreestyleEffectsBusKey[] = "freestyle_effects_bus";
constexpr char kFreestyleEffectsBusError[] = "Freestyle effects bus must be 0, 1, or 2";
constexpr char kRemixEffectsBusKey[] = "remix_effects_bus";
constexpr char kRemixEffectsBusError[] = "Remix effects bus must be 0, 1, or 2";
constexpr char kRemixEffectsNameKey[] = "remix_effects_name";
constexpr char kNoRemixEffectsNameError[] = "No remix_effects_name array";
constexpr char kNotLocalizedFormat[] = "\"%s\" is not localized";
constexpr char kEffectsKey[] = "effects";

constexpr char kDuelPatternFile[] = "/duel_pattern_";
constexpr char kRemixPatternFile[] = "/remix_pattern_";
constexpr char kPatternBufferTag[] = "PitchPatternBuilder buf";
constexpr char kPatternFileMissingFormat[] = "Remix template file %s not found";

constexpr char kDuelSectionNamesKey[] = "duel_section_names";
constexpr char kRemixSectionNamesKey[] = "remix_section_names";
constexpr char kDuelSectionLinksKey[] = "duel_section_links";
constexpr char kRemixSectionLinksKey[] = "remix_section_links";
constexpr char kDuelSectionLabelsKey[] = "duel_section_labels";
constexpr char kRemixSectionLabelsKey[] = "remix_section_labels";
constexpr char kMissingSectionNamesError[] = "Must have a name for every remix section";
constexpr char kAuthoringLabelFormat[] = "section %i";
constexpr char kDuplicateSectionNameFormat[] = "Remix section name %s used more than once";
constexpr char kMissingSectionFormat[] = "Remix section %s not found";
constexpr char kLinkedLengthFormat[] = "Linked remix sections %s and %s must have the same length";
constexpr char kNumberedLabelFormat[] = "%s %i";
constexpr char kLabelCountError[] =
    "There must be the same number of remix section labels as remix sections";
constexpr char kUnlinkedSectionFormat[] =
    "Remix section %s is not in the remix_section_links array";

constexpr char kSectionStartBarsKey[] = "section_start_bars";
constexpr char kDuelSectionStartBarsKey[] = "duel_section_start_bars";
constexpr char kRemixSectionStartBarsKey[] = "remix_section_start_bars";
constexpr char kNoRuleSetWarning[] = "no ruleset";
constexpr char kOddSectionFormat[] = "Section %i has %i bars; it should be an even number of bars ";

constexpr char kMixerOverridesKey[] = "mixer_overrides";
constexpr char kChannelKey[] = "channel";
constexpr char kChannelRangeError[] = "mixer channel must be in [1..16]";
constexpr char kChannelTwiceError[] = "mixer channel specified twice";
constexpr char kLowVolumesKey[] = "mixer_low_volumes";
constexpr char kFreestyleLowVolumesKey[] = "mixer_low_volumes_freestyle";
constexpr char kDuelMixVolumesKey[] = "duel_mix_volumes";
constexpr char kVolumesKey[] = "volumes";

constexpr char kEnableOrderKey[] = "enable_order";
constexpr char kEnableOrderRangeFormat[] = "Track %i in enable_order array is out of range";
constexpr char kTrackEnabledTwiceError[] = "track is enabled twice";
constexpr char kTrackRangeError[] = "track index out of range";

constexpr char kBackgroundTracksKey[] = "background_tracks";
constexpr char kInvalidMasterError[] = "invalid master track index";
constexpr char kMasterNotCatchError[] = "can only slave a background track to a catch track";
constexpr char kMasterTwiceError[] = "Can't slave a background track to more than one catch track";
constexpr char kUnknownBackgroundFormat[] = "Unknown background track %s";

constexpr char kDuelTrackOrderKey[] = "duel_track_order";
constexpr char kDuelTrackOrderRangeError[] = "invalid duel_track_order track number";
constexpr char kDuelTrackOrderTypeError[] = "duel_track_order track must be a pitch track";
constexpr char kDuelNoBackgroundKey[] = "duel_tracks_no_background";
constexpr char kNoBackgroundRangeError[] = "invalid 'no background' track number";
constexpr char kNoBackgroundTypeError[] = "'no background' track must be a pitch track";
constexpr char kDuelBreakBarsKey[] = "duel_break_bars";
constexpr char kDuelPatternsKey[] = "duel_patterns";

constexpr char kIllegalInstrumentWarning[] = "illegal instrument";

// The length of a beat in ticks, and the milliseconds in a minute.
constexpr float kTicksPerBeat = 480.0f;
constexpr float kMsPerMinute = 60000.0f;

// The first bar a value in a song text file counts from.
constexpr int kFirstBar = 1;

// The first value of an array after its name.
constexpr int kFirstValue = 1;

// The section starts entry that makes one endless section, and the bar it ends at.
constexpr int kEndlessSection = -1;
constexpr int kEndlessSectionEnd = 1000000000;

// The effects buses are 0 to kNumEffectsBuses - 1.
constexpr unsigned int kNumEffectsBuses = 3;

// A low volume is 127 less 1.25 times the attenuation the file gives, and never negative.
constexpr int kMaxVolume = 127;
constexpr double kMaxVolumeLevel = 127.0;
constexpr double kAttenuationScale = 1.25;

// ApplySavedRemix() splits the song at its quarters.
constexpr float kQuarter = 0.25f;
constexpr int kNumQuarters = 4;

// The pattern of a duel's first pattern file, which the duel names apart.
constexpr int kDuelPattern = 1;

// The rule set of a game database that sets none up.
constexpr int kNoRuleSet = 0;

constexpr int kOpenRead = 1;
constexpr int kNoOpenFlags = 0;
constexpr int kDefaultAlign = 0;

// The MIDI controller ApplySynthSettings() sets on each channel, and its values.
constexpr unsigned int kControlChange = 0xb0;
constexpr unsigned int kController = 16;
constexpr unsigned int kDataShift = 8;
constexpr unsigned int kValueShift = 16;
constexpr int kInitialLevel = 10;
constexpr int kFreestyleLevel = 50;
constexpr int kLastChannelLevel = 70;
constexpr int kLastChannel = 15;
constexpr int kInstrumentLevels[] = {40, 30, 30, 30, 60, 20};
constexpr unsigned int kNumInstruments = 6;

} // namespace

Song::Song(DataArray *pSongConfig, int nSkillLevel, int nRuleSet) {
    mRuleSet = nRuleSet;
    mDifficulty = nSkillLevel;
    mConfig = pSongConfig;
    mState = kLoadStateParse;
    mErrorHandler = nullptr;
    mValidate = 0;
    mReserved14 = 0;
    mBuilder = nullptr;
    mNumBars = 0;
    mIntroBars = 0;
    mSections = nullptr;
    mMsPerTick = nullptr;
    mSlotGrid = nullptr;
    mFreestyleEffectsBus = 0;
    mRemixEffectsBus = 0;
    mRemixEffectsName = kNoName;
    mReserved30 = kNoText;
}

Song::~Song() {
    delete mSections;
    delete mMsPerTick;
    delete mSlotGrid;
    delete mBuilder;
    for (auto &gems : mPitchGems) {
        for (PitchTrackGems *pGems : gems) {
            delete pGems;
        }
    }
}

void Song::Load() {
    mErrorHandler = nullptr;
    mValidate = 0;
    StartLoad();
}

void Song::StartLoad() {
    ReadConfig();
    mBuilder = new LevelMidiBuilder(mMidiFile.c_str(),
                                    mDifficulty,
                                    mRuleSet,
                                    mNumBars,
                                    mIntroBars,
                                    mFreestyleEffectsBus,
                                    mRemixEffectsBus,
                                    mSections,
                                    mMsPerTick,
                                    mSlotGrid);
    if (mValidate) {
        mBuilder->StartLoad(mErrorHandler);
    } else {
        mBuilder->StartLoad();
    }
}

void Song::PollLoad() {
    switch (mState) {
    case kLoadStateParse:
        mBuilder->Poll();
        if (mBuilder->IsDone()) {
            mState = kLoadStateSetup;
        }
        break;
    case kLoadStateSetup:
        ReadTrackConfig();
        if (mValidate) {
            ValidateEnableOrder();
        }
        if (mRuleSet == GameDb::kRuleSetRemix || mRuleSet == GameDb::kRuleSetDuel) {
            StartPatternLoad();
            mState = kLoadStatePatterns;
        } else {
            mState = kLoadStateDone;
        }
        break;
    case kLoadStatePatterns: {
        bool bAllDone = true;
        for (PatternFile *pFile : mPatternFiles) {
            bool bDone;
            if (pFile->mDone) {
                bDone = true;
            } else {
                int nBytes;
                pFile->mDone = pFile->mFile->ReadDone(&nBytes);
                if (pFile->mDone) {
                    BufStream stream(pFile->mBuffer, pFile->mSize, true);
                    std::vector<std::vector<PitchTrackGems *>> &pitchGems =
                        pFile->mSong->mPitchGems;
                    for (unsigned int i = 0; i < pitchGems.size(); ++i) {
                        PitchTrackGems *pGems = pitchGems[i][pFile->mIndex];
                        if (pGems != nullptr) {
                            pGems->Load(stream);
                        }
                    }
                    bDone = true;
                } else {
                    bDone = false;
                }
            }
            bAllDone = bAllDone && bDone;
        }
        if (bAllDone) {
            for (PatternFile *pFile : mPatternFiles) {
                delete pFile;
            }
            mState = kLoadStateDone;
        }
        break;
    }
    default:
        break;
    }
}

bool Song::IsLoaded() {
    return mState == kLoadStateDone;
}

void Song::Error(const String &message) {
    String text;
    text.Printf(kErrorFormat, message.c_str());
    if (mErrorHandler != nullptr) {
        String copy(text.c_str());
        mErrorHandler(copy);
    } else {
        DebugError(text.c_str());
    }
}

const char *Song::ReadSymbol(const char *pszName, DataArray *pData) {
    const char *pszValue = nullptr;
    if (!pData->FindSymbol(pszName, &pszValue, false)) {
        Error(String(kMissingString) + pszName);
    }
    return pszValue;
}

int Song::ReadInt(const char *pszName, DataArray *pData) {
    int nValue = 0;
    if (!pData->FindInt(pszName, &nValue, false)) {
        Error(String(kMissingInteger) + pszName);
    }
    return nValue;
}

float Song::ReadFloat(const char *pszName, DataArray *pData) {
    float fValue = 0.0f;
    if (!pData->FindFloat(pszName, &fValue, false)) {
        Error(String(kMissingFloat) + pszName);
    }
    return fValue;
}

DataArray *Song::ReadArray(const char *pszName, DataArray *pData, bool bRequired) {
    DataArray *pArray = pData->FindArray(pszName, false);
    if (bRequired && pArray == nullptr) {
        Error(String(kMissingArray) + pszName);
    }
    return pArray;
}

void Song::StartPatternLoad() {
    const int nTracks = GetNumTracks();
    const int nTicksPerBar = mBuilder->mTicksPerBar;
    for (int i = 0; i < nTracks; ++i) {
        std::vector<PitchTrackGems *> gems(kNumPatternFiles, nullptr);
        if (GetTrackType(i) == kTrackTypePitch) {
            for (int j = 0; j < kNumPatternFiles; ++j) {
                gems[j] = new PitchTrackGems(GetSections(), GetSlotGrid(), mNumBars, nTicksPerBar);
            }
        }
        mPitchGems.push_back(gems);
    }

    for (int i = 0; i < kNumPatternFiles; ++i) {
        String path(mMidiDir);
        if (mRuleSet == GameDb::kRuleSetDuel && mConfig->FindArray(kDuelSongBarsKey, false) &&
            i == 0) {
            path << kDuelPatternFile << kDuelPattern;
        } else {
            path << kRemixPatternFile << i + 1;
        }
        auto *pFile = new PatternFile(path.c_str());
        pFile->mIndex = i;
        mPatternFiles[i] = pFile;
        pFile->mSong = this;
        pFile->mFile = nullptr;
        pFile->mBuffer = nullptr;
        pFile->mSize = 0;
        pFile->mDone = 0;
        pFile->mFile = File::New(pFile->mPath.c_str(), kOpenRead, kNoOpenFlags);
        if (pFile->mFile == nullptr) {
            DebugNotify(kPatternFileMissingFormat, pFile->mPath.c_str());
            pFile->mDone = 1;
        } else {
            pFile->mSize = pFile->mFile->Size();
            pFile->mBuffer =
                static_cast<char *>(PoolMemAlloc(pFile->mSize, kPatternBufferTag, kDefaultAlign));
            pFile->mFile->ReadAsync(pFile->mBuffer, pFile->mSize);
            pFile->mDone = 0;
        }
    }
}

void Song::ReadConfig() {
    if (mRuleSet == GameDb::kRuleSetDuel) {
        if (!mConfig->FindInt(kDuelSongBarsKey, &mNumBars, false)) {
            mNumBars = ReadInt(kRemixSongBarsKey, mConfig);
        }
    } else if (mRuleSet == GameDb::kRuleSetRemix) {
        mNumBars = ReadInt(kRemixSongBarsKey, mConfig);
    } else {
        mNumBars = ReadInt(kSongBarsKey, mConfig);
    }
    mIntroBars = ReadInt(kIntroBarsKey, mConfig);
    const float fBpm = ReadFloat(kBpmKey, mConfig);
    mMsPerTick = new float;
    *mMsPerTick = kMsPerMinute / fBpm / kTicksPerBeat;

    const char *pszMidiKey = nullptr;
    switch (mRuleSet) {
    case GameDb::kRuleSetRemix:
        pszMidiKey = kRemixMidiFileKey;
        break;
    case GameDb::kRuleSetGame:
        pszMidiKey = kMidiFileKey;
        break;
    case GameDb::kRuleSetDuel:
        pszMidiKey = kDuelMidiFileKey;
        if (mConfig->FindArray(pszMidiKey, false) == nullptr) {
            pszMidiKey = kRemixMidiFileKey;
        }
        break;
    default:
        DebugWarn(kIllegalModeWarning);
        break;
    }
    mMidiDir = FileGetPath(mConfig->FindArray(pszMidiKey, false)->mFile);
    mMidiFile = mMidiDir;
    mMidiFile += kPathSeparator;
    mMidiFile += ReadSymbol(pszMidiKey, mConfig);

    ReadEffects();
    ReadMixerOverrides();
    if (mRuleSet == GameDb::kRuleSetDuel) {
        ReadDuelMixVolumes();
    }
    mFreestyleEffectsBus = ReadInt(kFreestyleEffectsBusKey, mConfig);
    if (static_cast<unsigned int>(mFreestyleEffectsBus) >= kNumEffectsBuses) {
        Error(String(kFreestyleEffectsBusError));
    }
    if (mRuleSet == GameDb::kRuleSetRemix) {
        mRemixEffectsBus = ReadInt(kRemixEffectsBusKey, mConfig);
        if (static_cast<unsigned int>(mRemixEffectsBus) >= kNumEffectsBuses) {
            Error(String(kRemixEffectsBusError));
        }
        const char *pszEffectsName = nullptr;
        mConfig->FindSymbol(kRemixEffectsNameKey, &pszEffectsName, false);
        if (mValidate && pszEffectsName == nullptr) {
            Error(String(kNoRemixEffectsNameError));
        }
        if (pszEffectsName != nullptr) {
            mRemixEffectsName = TheLocale.Localize(pszEffectsName, false);
            if (mRemixEffectsName == nullptr) {
                Error(String(FormatString(kNotLocalizedFormat, pszEffectsName)));
            }
        }
    }
    ReadSectionStarts();
    if (mRuleSet == GameDb::kRuleSetRemix || mRuleSet == GameDb::kRuleSetDuel) {
        ReadRemixSections();
    }
}

void Song::ReadTrackConfig() {
    if (mRuleSet == GameDb::kRuleSetDuel) {
        ReadDuelTracks();
        ReadDuelPatterns();
    }
    if (mRuleSet == GameDb::kRuleSetGame) {
        ReadEnableOrder();
        ReadBackgroundTracks();
    }
}

void Song::ReadRemixSections() {
    if (mSections == nullptr) {
        return;
    }
    mSlotGrid = new SlotGrid(mSections->NumSections());

    DataArray *pNames;
    DataArray *pLinks;
    DataArray *pLabels;
    if (mRuleSet == GameDb::kRuleSetDuel) {
        pNames = mConfig->FindArray(kDuelSectionNamesKey, false);
        if (pNames == nullptr) {
            pNames = ReadArray(kRemixSectionNamesKey, mConfig, false);
        }
        pLinks = mConfig->FindArray(kDuelSectionLinksKey, false);
        if (pLinks == nullptr) {
            pLinks = ReadArray(kRemixSectionLinksKey, mConfig, false);
        }
        pLabels = mConfig->FindArray(kDuelSectionLabelsKey, false);
        if (pLabels == nullptr) {
            pLabels = ReadArray(kRemixSectionLabelsKey, mConfig, false);
        }
    } else {
        pNames = ReadArray(kRemixSectionNamesKey, mConfig, false);
        pLinks = ReadArray(kRemixSectionLinksKey, mConfig, false);
        pLabels = ReadArray(kRemixSectionLabelsKey, mConfig, false);
    }
    if (pNames == nullptr || pLinks == nullptr) {
        return;
    }
    if (pNames->Size() - kFirstValue != mSections->NumSections()) {
        Error(String(kMissingSectionNamesError));
    }

    if (g_bDuelAuthoring) {
        for (int i = kFirstValue; i < pNames->Size(); ++i) {
            mSectionLabels.push_back(String(FormatString(kAuthoringLabelFormat, i)));
        }
        return;
    }

    // Symbols are unique, so the map compares their addresses.
    std::map<const char *, int> sections;
    for (int i = kFirstValue; i < pNames->Size(); ++i) {
        const char *pszName = pNames->Sym(i);
        if (mValidate && sections.find(pszName) != sections.end()) {
            Error(String(FormatString(kDuplicateSectionNameFormat, pszName)));
        }
        sections[pszName] = i - kFirstValue;
    }

    for (int i = kFirstValue; i < pLinks->Size(); ++i) {
        DataArray *pLink = pLinks->Array(i);
        const char *pszFirst = pLink->Sym(0);
        for (int j = kFirstValue; j < pLink->Size(); ++j) {
            const char *pszOther = pLink->Sym(j);
            if (mValidate) {
                if (sections.find(pszFirst) == sections.end()) {
                    Error(String(FormatString(kMissingSectionFormat, pszFirst)));
                }
                if (sections.find(pszOther) == sections.end()) {
                    Error(String(FormatString(kMissingSectionFormat, pszOther)));
                }
            }
            const int nFirst = sections[pszFirst];
            const int nOther = sections[pszOther];
            if (mValidate) {
                const int nFirstBars =
                    mSections->SectionEnd(nFirst) - mSections->SectionStart(nFirst);
                const int nOtherBars =
                    mSections->SectionEnd(nOther) - mSections->SectionStart(nOther);
                if (nFirstBars != nOtherBars) {
                    Error(String(FormatString(kLinkedLengthFormat, pszFirst, pszOther)));
                }
            }
            mSlotGrid->SetPattern(nFirst, nOther);
        }
    }

    if (pLabels != nullptr) {
        std::vector<const char *> labels;
        for (int i = kFirstValue; i < pLabels->Size(); ++i) {
            const char *pszToken = pLabels->Sym(i);
            const char *pszLabel = TheLocale.Localize(pszToken, true);
            if (pszLabel == nullptr) {
                Error(String(FormatString(kNotLocalizedFormat, pszToken)));
            }
            labels.push_back(pszLabel);
        }
        for (unsigned int i = 0; i < labels.size(); ++i) {
            if (std::count(labels.begin(), labels.end(), labels[i]) >= 2) {
                const int nOrdinal =
                    static_cast<int>(std::count(labels.begin(), labels.begin() + i, labels[i]));
                mSectionLabels.push_back(
                    String(FormatString(kNumberedLabelFormat, labels[i], nOrdinal + 1)));
            } else {
                mSectionLabels.push_back(String(labels[i]));
            }
        }
    }

    if (static_cast<int>(mSectionLabels.size()) != mSlotGrid->GetNumFree()) {
        Error(String(kLabelCountError));
        while (static_cast<int>(mSectionLabels.size()) < mSlotGrid->GetNumFree()) {
            mSectionLabels.push_back(String(kNoName));
        }
    }

    if (mValidate) {
        std::set<const char *> unused; // Yes, the binary builds this set and never fills it.
        for (int i = kFirstValue; i < pNames->Size(); ++i) {
            const char *pszName = pNames->Sym(i);
            bool bFound = false;
            for (int j = kFirstValue; j < pLinks->Size(); ++j) {
                DataArray *pLink = pLinks->Array(j);
                for (int k = 0; k < pLink->Size(); ++k) {
                    if (pLink->Sym(k) == pszName) {
                        bFound = true;
                        break;
                    }
                }
                if (bFound) {
                    break;
                }
            }
            if (!bFound) {
                Error(String(FormatString(kUnlinkedSectionFormat, pszName)));
            }
        }
    }
}

void Song::ReadSectionStarts() {
    DataArray *pStarts = nullptr;
    switch (mRuleSet) {
    case GameDb::kRuleSetGame:
        pStarts = ReadArray(kSectionStartBarsKey, mConfig, false);
        break;
    case GameDb::kRuleSetDuel:
        pStarts = ReadArray(kDuelSectionStartBarsKey, mConfig, false);
        if (pStarts == nullptr) {
            pStarts = ReadArray(kRemixSectionStartBarsKey, mConfig, false);
        }
        break;
    case GameDb::kRuleSetRemix:
        pStarts = ReadArray(kRemixSectionStartBarsKey, mConfig, false);
        break;
    case kNoRuleSet:
        DebugWarn(kNoRuleSetWarning);
        break;
    default:
        break;
    }
    BuildSections(pStarts);
}

void Song::BuildSections(DataArray *pStarts) {
    std::vector<int> ends;
    if (pStarts == nullptr) {
        ends.push_back(mNumBars);
    } else if (pStarts->Int(kFirstValue) == kEndlessSection) {
        ends.push_back(kEndlessSectionEnd);
    } else {
        for (int i = kFirstValue; i < pStarts->Size(); ++i) {
            const int nBar = pStarts->Int(i) - kFirstBar;
            if (nBar >= mNumBars) {
                break;
            }
            if (nBar != 0) {
                ends.push_back(nBar);
            }
        }
        ends.push_back(mNumBars);
    }
    mSections = new SectionBoundaries{ends};

    if (mRuleSet == GameDb::kRuleSetRemix && mValidate) {
        for (int i = 0; i < mSections->NumSections();) {
            int nStart;
            int nEnd;
            mSections->GetSectionRange(i, &nStart, &nEnd);
            ++i;
            if ((nEnd - nStart) & 1) {
                Error(String(FormatString(kOddSectionFormat, i, nEnd - nStart)));
            }
        }
    }
}

void Song::ReadMixerOverrides() {
    DataArray *pOverrides = mConfig->FindArray(kMixerOverridesKey, false);
    if (pOverrides == nullptr) {
        return;
    }
    for (int i = kFirstValue; i < pOverrides->Size(); ++i) {
        DataArray *pOverride = pOverrides->Array(i);
        const int nChannel = ReadInt(kChannelKey, pOverride) - 1;
        if (static_cast<unsigned int>(nChannel) >= kNumMixerChannels) {
            Error(String(kChannelRangeError));
        }
        std::vector<unsigned char> &volumes = mLowVolumes[nChannel];
        if (!volumes.empty()) {
            Error(String(kChannelTwiceError));
        }
        DataArray *pVolumes = ReadArray(kLowVolumesKey, pOverride, true);
        for (int j = kFirstValue; j < pVolumes->Size(); ++j) {
            int nVolume = static_cast<int>(kMaxVolumeLevel -
                                           kAttenuationScale *
                                               static_cast<double>(kMaxVolume - pVolumes->Int(j)));
            if (nVolume < 0) {
                nVolume = 0;
            }
            volumes.push_back(static_cast<unsigned char>(nVolume));
        }
    }

    DataArray *pFreestyle = mConfig->FindArray(kFreestyleLowVolumesKey, false);
    if (pFreestyle == nullptr) {
        return;
    }
    for (int i = kFirstValue; i < pFreestyle->Size(); ++i) {
        mFreestyleLowVolumes.push_back(static_cast<unsigned char>(pFreestyle->Int(i)));
    }
}

void Song::ReadDuelMixVolumes() {
    DataArray *pMixes = mConfig->FindArray(kDuelMixVolumesKey, false);
    if (pMixes == nullptr) {
        return;
    }
    for (int i = kFirstValue; i < pMixes->Size(); ++i) {
        DataArray *pMix = pMixes->Array(i);
        const int nChannel = ReadInt(kChannelKey, pMix) - 1;
        if (static_cast<unsigned int>(nChannel) >= kNumMixerChannels) {
            Error(String(kChannelRangeError));
        }
        std::vector<unsigned char> &volumes = mTrackVolumes[nChannel];
        if (!volumes.empty()) {
            Error(String(kChannelTwiceError));
        }
        DataArray *pVolumes = ReadArray(kVolumesKey, pMix, true);
        for (int j = kFirstValue; j < pVolumes->Size(); ++j) {
            volumes.push_back(static_cast<unsigned char>(pVolumes->Int(j)));
        }
    }
}

void Song::ReadEffects() {
    mEffects.Load(ReadArray(kEffectsKey, mConfig, true));
}

void Song::ReadEnableOrder() {
    DataArray *pOrder = ReadArray(kEnableOrderKey, mConfig, true);
    for (int i = kFirstValue; i < pOrder->Size(); ++i) {
        DataArray *pStep = pOrder->Array(i);
        std::vector<int> tracks;
        for (int j = 0; j < pStep->Size(); ++j) {
            const int nTrack = pStep->Int(j) - 1;
            if (nTrack >= GetNumTracks() - 1 || nTrack < 0) {
                Error(String(FormatString(kEnableOrderRangeFormat, nTrack + 1)));
            }
            tracks.push_back(nTrack);
        }
        mEnableOrder.push_back(tracks);
    }
}

void Song::ReadBackgroundTracks() {
    mBackMusicMasters.assign(GetNumBackMusic(), -1);
    DataArray *pTracks = mConfig->FindArray(kBackgroundTracksKey, false);
    if (pTracks == nullptr) {
        return;
    }
    for (int i = kFirstValue; i < pTracks->Size(); ++i) {
        const char *pszName = pTracks->Array(i)->Sym(0);
        const int nMaster = pTracks->Array(i)->Int(1) - 1;
        if (nMaster < 0 || nMaster >= GetNumTracks()) {
            Error(String(kInvalidMasterError));
        }
        if (GetTrackType(nMaster) != kTrackTypeCatch) {
            Error(String(kMasterNotCatchError));
        }
        bool bFound = false;
        for (int j = 0; j < GetNumBackMusic(); ++j) {
            if (strcasecmp(pszName, GetBackMusic(j)->GetName()) == 0) {
                if (mBackMusicMasters[j] != -1) {
                    Error(String(kMasterTwiceError));
                }
                bFound = true;
                mBackMusicMasters[j] = nMaster;
                break;
            }
        }
        if (!bFound) {
            Error(String(FormatString(kUnknownBackgroundFormat, pszName)));
        }
    }
}

void Song::ReadDuelTracks() {
    DataArray *pOrder = mConfig->FindArray(kDuelTrackOrderKey, false);
    if (pOrder != nullptr) {
        for (int i = kFirstValue; i < pOrder->Size(); ++i) {
            const int nTrack = pOrder->Int(i) - 1;
            if (nTrack < 0 || nTrack >= GetNumTracks()) {
                Error(String(kDuelTrackOrderRangeError));
            }
            if (GetTrackType(nTrack) != kTrackTypePitch) {
                Error(String(kDuelTrackOrderTypeError));
            }
            mDuelTrackOrder.push_back(nTrack);
        }
    } else {
        for (int i = 0; i < GetNumTracks(); ++i) {
            if (GetTrackType(i) == kTrackTypePitch) {
                mDuelTrackOrder.push_back(i);
            }
        }
    }

    DataArray *pNoBackground = mConfig->FindArray(kDuelNoBackgroundKey, false);
    if (pNoBackground != nullptr) {
        for (int i = kFirstValue; i < pNoBackground->Size(); ++i) {
            const int nTrack = pNoBackground->Int(i) - 1;
            if (nTrack < 0 || nTrack >= GetNumTracks()) {
                Error(String(kNoBackgroundRangeError));
            }
            if (GetTrackType(nTrack) != kTrackTypePitch) {
                Error(String(kNoBackgroundTypeError));
            }
            GetTrackRiffData(nTrack)->SetEnabled(false);
        }
    }

    DataArray *pBreaks = mConfig->FindArray(kDuelBreakBarsKey, false);
    if (pBreaks != nullptr) {
        for (int i = kFirstValue; i < pBreaks->Size(); ++i) {
            mSkippedBars.push_back(pBreaks->Int(i) - kFirstBar);
        }
    }
}

void Song::ReadDuelPatterns() {
    DataArray *pPatterns = mConfig->FindArray(kDuelPatternsKey, false);
    if (pPatterns == nullptr) {
        const DuelPattern pattern(nullptr);
        mDuelPatterns.mEntries.push_back(DuelPatternTable::Entry{0, pattern});
        return;
    }
    DataArray *pLevel = pPatterns->Array(mDifficulty);
    const int nTicksPerBar = mBuilder->mTicksPerBar;
    for (int i = 0; i < pLevel->Size(); ++i) {
        DataArray *pEntry = pLevel->Array(i);
        const int nTick = (pEntry->Int(0) - kFirstBar) * nTicksPerBar;
        const DuelPattern pattern(pEntry->Array(1));
        mDuelPatterns.mEntries.push_back(DuelPatternTable::Entry{nTick, pattern});
    }
}

void Song::ValidateEnableOrder() {
    std::set<int> enabled;
    for (unsigned int i = 0; i < mEnableOrder.size(); ++i) {
        for (unsigned int j = 0; j < mEnableOrder[i].size(); ++j) {
            const int nTrack = mEnableOrder[i][j];
            if (enabled.find(nTrack) != enabled.end()) {
                Error(String(kTrackEnabledTwiceError));
            }
            enabled.insert(nTrack);
        }
    }
    const int nTracks = GetNumTracks();
    for (unsigned int i = 0; i < mEnableOrder.size(); ++i) {
        for (unsigned int j = 0; j < mEnableOrder[i].size(); ++j) {
            if (mEnableOrder[i][j] >= nTracks) {
                Error(String(kTrackRangeError));
            }
        }
    }
}

void Song::ApplySynthSettings() {
    mEffects.Apply();
    Synth *pSynth = TheSynth;
    int nLevel = kInitialLevel;
    for (int i = 0; i < kNumMixerChannels; ++i) {
        if (i < GetNumTracks()) {
            const int nType = GetTrackType(i);
            if (nType == kTrackTypeAxe || nType == kTrackTypeScratch) {
                nLevel = kFreestyleLevel;
            } else {
                const int nInstrument = GetTrackInstrument(i);
                if (static_cast<unsigned int>(nInstrument) < kNumInstruments) {
                    nLevel = kInstrumentLevels[nInstrument];
                } else {
                    DebugWarn(kIllegalInstrumentWarning);
                }
            }
        } else if (i == kLastChannel) {
            nLevel = kLastChannelLevel;
        }
        pSynth->SendPackedMessage((kControlChange | static_cast<unsigned int>(i)) |
                                  (kController << kDataShift) |
                                  (static_cast<unsigned char>(nLevel) << kValueShift));
    }
}

void Song::ApplySavedRemix() {
    for (unsigned int i = 0; i < mPitchGems.size(); ++i) {
        for (PitchTrackGems *pGems : mPitchGems[i]) {
            delete pGems;
        }
    }
    mPitchGems.clear();
    ReadBackgroundTracks();

    mEnableOrder.resize(1);
    std::vector<int> &tracks = mEnableOrder[0];
    tracks.resize(GetNumTracks() - 1, 0);
    for (unsigned int i = 0; i < tracks.size(); ++i) {
        tracks[i] = static_cast<int>(i);
    }

    mBuilder->LoadRemix();

    std::vector<int> ends;
    const float fQuarterBars = static_cast<float>(mNumBars) * kQuarter;
    for (int i = 1; i < kNumQuarters; ++i) {
        const int nBar = static_cast<int>(fQuarterBars * static_cast<float>(i));
        ends.push_back(mSections->SectionStart(mSections->SectionAt(nBar)));
    }
    ends.push_back(mNumBars);
    delete mSections;
    mSections = new SectionBoundaries{ends};
}

int Song::GetNumTracks() const {
    return mBuilder->GetNumTracks();
}

int Song::GetTrackType(int nTrack) const {
    return mBuilder->GetTrackType(nTrack);
}

int Song::GetTrackInstrument(int nTrack) const {
    return mBuilder->GetTrackInstrument(nTrack);
}

DuelPatternTable *Song::GetDuelPatterns() {
    return &mDuelPatterns;
}

CatchTrackData *Song::GetCatchTrackData(int nTrack) const {
    return mBuilder->GetCatchTrackData(nTrack);
}

ScratchTrackData *Song::GetScratchData(int nTrack) const {
    return mBuilder->GetScratchData(nTrack);
}

AxeTrackData *Song::GetAxeData(int nTrack) const {
    return mBuilder->GetAxeData(nTrack);
}

PitchTrackRiffData *Song::GetTrackRiffData(int nTrack) const {
    return mBuilder->GetTrackRiffData(nTrack);
}

CatchTrackData *Song::GetVoxData(int nTrack) const {
    return mBuilder->GetVoxData(nTrack);
}

int Song::GetTrackFlags(int nTrack) const {
    return mBuilder->GetTrackFlags(nTrack);
}

int Song::GetNumBackMusic() const {
    return mBuilder->GetNumBackMusic();
}

BackMusic *Song::GetBackMusic(int nIndex) const {
    return mBuilder->GetBackMusic(nIndex);
}

int Song::GetBackMusicMaster(int nIndex) const {
    return mBackMusicMasters[nIndex];
}

int Song::GetNumIntroMuses() const {
    return mBuilder->GetNumIntroMuses();
}

Muse *Song::GetIntroMuse(int nIndex) const {
    return mBuilder->GetIntroMuse(nIndex);
}

ScriptTrackData *Song::GetScriptTrack(const char *pszName) const {
    return mBuilder->FindScriptTrack(pszName);
}

SectionBoundaries *Song::GetSections() const {
    return mSections;
}

float *Song::GetMsPerTick() const {
    return mMsPerTick;
}

PlayMap *Song::GetPlayMap() const {
    return mBuilder->mPlayMap;
}

BankLoader *Song::GetBankTrack() const {
    return mBuilder->mBankTrack;
}

WorldTrack *Song::GetWorldTrack() const {
    return &mBuilder->mWorldTrack;
}

Lyric *Song::GetLyric() const {
    return mBuilder->mLyric;
}

FXMgr *Song::GetFXMgr() const {
    return mBuilder->mFXMgr;
}

float Song::GetSpeed() const {
    return mBuilder->mSpeed;
}

SlotGrid *Song::GetSlotGrid() const {
    return mSlotGrid;
}

std::vector<PitchTrackGems *> *Song::GetTrackPitchData(int nTrack) {
    (void)GetTrackType(nTrack); // Yes, the binary discards the type.
    return &mPitchGems[nTrack];
}

const std::vector<std::vector<int>> &Song::GetEnableOrder() const {
    return mEnableOrder;
}

bool Song::HasLowVolumes(int nChannel) const {
    return !mLowVolumes[nChannel].empty();
}

const std::vector<unsigned char> *Song::GetLowVolumes(int nChannel) const {
    (void)HasLowVolumes(nChannel); // Yes, the binary discards the result.
    return &mLowVolumes[nChannel];
}

bool Song::HasTrackVolumes(int nTrack) const {
    return !mTrackVolumes[nTrack].empty();
}

const std::vector<unsigned char> &Song::GetTrackVolumes(int nTrack) const {
    (void)HasTrackVolumes(nTrack); // Yes, the binary discards the result.
    return mTrackVolumes[nTrack];
}

bool Song::HasFreestyleLowVolumes() const {
    return !mFreestyleLowVolumes.empty();
}

const std::vector<unsigned char> *Song::GetFreestyleLowVolumes() const {
    (void)HasFreestyleLowVolumes(); // Yes, the binary discards the result.
    return &mFreestyleLowVolumes;
}

int Song::GetRemixEffectsBus() const {
    return mRemixEffectsBus;
}

const char *Song::GetRemixEffectsName() const {
    return mRemixEffectsName;
}

const char *Song::GetSectionLabel(int nIndex) const {
    return mSectionLabels[nIndex].c_str();
}
