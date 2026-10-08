#include "synth/BankManager.h"

#include <map>
#include <vector>

#include <afxdlgs.h>

#include "os/Debug.h"
#include "os/File.h"
#include "synth/BankFileIO.h"
#include "synth/BankLookup.h"
#include "synth/BankWin.h"
#include "synth/InstrumentWin.h"
#include "synth/SampleDescWin.h"
#include "synth/SampleWin.h"
#include "utl/BinStream.h"
#include "utl/BufStream.h"
#include "utl/Str.h"

namespace {

// Limits of a bank on the console.
const unsigned int kMaxInstruments = 100;
const unsigned int kMaxSamples = 700;
const int kMaxSampleDescriptions = 700;

// Partitions of the console's audio memory, and the bytes of each.
const int kNumAudioPartitions = 28351;
const unsigned int kPartitionSize = 64;

const int kNameChunkVersion = 1;
const int kAnyProgram = -1;
const int kNumKeys = 128;

const unsigned short kMonoChannels = 1;
const int kSampleBits = 16;

// Flags of the dialog that finds a replacement WAV file.
const DWORD kReplacementDialogFlags =
    OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

// 0x1003ab28
int gNextWinID = 100;

// 0x10023720, 0x100237b0, 0x10023840, 0x100238d0
template <class T>
void DeleteAllInMap(std::map<int, T *> &iMap) {
    typename std::map<int, T *>::iterator it;
    for (it = iMap.begin(); it != iMap.end(); ++it) {
        T *object = it->second;
        delete object;
    }
    iMap.clear();
}

// 0x10023960, 0x10023f10
template <class T>
int FindInVector(std::vector<T> &iVector, const T &iValue) {
    bool found = false;
    int size = iVector.size();
    int i = -1;
    for (i = 0; i < size; ++i) {
        if (iVector[i] == iValue) {
            found = true;
            break;
        }
    }
    return found ? i : -1;
}

// 0x10024270, 0x10024b40, 0x10024e00, 0x10025420
template <class T>
T *AddToMap(std::map<int, T *> &iMap, int iKey, T *iValue) {
    typename std::map<int, T *>::iterator it = iMap.find(iKey);
    ASSERT(it == iMap.end());
    iMap[iKey] = iValue;
    return iMap[iKey];
}

// 0x10024190, 0x10024d20, 0x10025340
template <class T>
void DeleteFromMap(std::map<int, T *> &iMap, int iKey) {
    typename std::map<int, T *>::iterator it = iMap.find(iKey);
    ASSERT(it != iMap.end());
    ASSERT(it->second != NULL);
    T *object = it->second;
    iMap.erase(it);
    delete object;
}

// 0x100254a0
template <class T>
T *RemoveFromMap(std::map<int, T *> &iMap, int iKey) {
    typename std::map<int, T *>::iterator it = iMap.find(iKey);
    ASSERT(it != iMap.end());
    ASSERT(it->second != NULL);
    T *object = it->second;
    iMap.erase(it);
    return object;
}

// 0x10024a60
template <class K, class V>
bool IsInMap(std::map<K, V> &iMap, K iKey) {
    return iMap.find(iKey) != iMap.end();
}

// 0x10024110
BinStream &operator<<(BinStream &bs, const std::vector<unsigned char> &data) {
    bs << static_cast<int>(data.size());
    for (std::vector<unsigned char>::const_iterator it = data.begin(); it != data.end(); ++it) {
        bs << static_cast<char>(*it);
    }
    return bs;
}

// 0x100242f0
BinStream &operator>>(BinStream &bs, std::vector<unsigned char> &data) {
    int size;
    bs >> size;
    data.resize(size);
    for (std::vector<unsigned char>::iterator it = data.begin(); it != data.end(); ++it) {
        bs >> reinterpret_cast<char &>(*it);
    }
    return bs;
}

// 0x100203a0
BinStream &operator<<(BinStream &bs, const String &string) {
    return bs << string.c_str();
}

// Wait for the chunk FindChunk() requested to arrive.
inline void ReadWholeChunk(BankFileIO *bio, std::vector<unsigned char> &chunk) {
    while (true) {
        if (!bio->ReadChunk(chunk)) {
            break;
        }
    }
}

} // namespace

SampleWin::SampleWin(const String &filename) : mID(gNextWinID++) {
    mSample.Init();
    mAudioIO = new AudioIO(filename);
    if (mAudioIO == NULL) {
        return;
    }
    if (!(*mAudioIO)) {
        return;
    }
    mName = FileGetBase(filename.c_str());
}

SampleWin::~SampleWin() {
    delete mAudioIO;
}

bool SampleWin::Fail() {
    if (mAudioIO == NULL) {
        return true;
    }
    if (!(*mAudioIO)) {
        return true;
    }
    if (mAudioIO->GetNumChannels() != kMonoChannels) {
        return true;
    }
    if (mAudioIO->GetBitsPerSample() != kSampleBits) {
        return true;
    }
    return false;
}

SampleDescWin::SampleDescWin(int sampleID) : mID(gNextWinID++), mSampleID(sampleID) {
    mSampleDescription.Init();
    mName = FormatString("Sample Description #%i", mID);
}

SampleDescWin::~SampleDescWin() {
}

void SampleDescWin::SetKeymap(unsigned char iLow, unsigned char iBase, unsigned char iHigh) {
    ASSERT(iLow < kNumKeys);
    ASSERT(iBase < kNumKeys);
    ASSERT(iHigh < kNumKeys);
    mSampleDescription.mLowKeymap = iLow;
    mSampleDescription.mBaseKey = iBase;
    mSampleDescription.mHighKeymap = iHigh;
}

void SampleDescWin::GetKeymap(unsigned char *oLow, unsigned char *oBase, unsigned char *oHigh) {
    *oLow = mSampleDescription.mLowKeymap;
    *oBase = mSampleDescription.mBaseKey;
    *oHigh = mSampleDescription.mHighKeymap;
}

InstrumentWin::InstrumentWin(unsigned short program) : mID(gNextWinID++) {
    mInstrument.Init();
    mInstrument.mProgram = program;
    mName = FormatString("Instrument #%i", mID);
}

InstrumentWin::~InstrumentWin() {
    DeleteAllInMap(mSampleDescs);
}

BankWin::BankWin(const String &filename, unsigned short bankID, bool promptForMissingFiles)
    : mAudioDataDirty(true), mID(gNextWinID++), mFilename(filename),
      mPromptForMissingFiles(promptForMissingFiles) {
    mBank.Init();
    mBank.mId = bankID;
    mName = "Bank";
    if (mFilename.size() > 0) {
        Load();
    }
}

BankWin::~BankWin() {
    DeleteAllInMap(mInstruments);
    DeleteAllInMap(mSamples);
}

BankManager::BankManager() {
    mBanks.clear();
}

BankManager::~BankManager() {
    DeleteAllInMap(mBanks);
}

bool BankWin::HasDuplicateProgramIds() {
    std::vector<unsigned short> programs;
    std::map<int, InstrumentWin *>::iterator it;
    bool duplicate = false;
    for (it = mInstruments.begin(); it != mInstruments.end(); ++it) {
        InstrumentWin *iWin = it->second;
        unsigned short program = iWin->GetInstrument().mProgram;
        if (FindInVector(programs, program) >= 0) {
            duplicate = true;
            break;
        }
        programs.push_back(program);
    }
    return duplicate;
}

int BankWin::Save(const String &filename, bool forceAudioData) {
    if (HasDuplicateProgramIds()) {
        TheDebug.Notify("This bank contains multiple instruments with the same program change id. "
                        "Save aborted.");
        return -1;
    }
    if (mInstruments.size() > kMaxInstruments) {
        String message(
            FormatString("This bank contains %i instruments. The maximum is %i. Save aborted.",
                         mInstruments.size(),
                         kMaxInstruments));
        TheDebug.Notify(message.c_str());
        return -1;
    }
    if (mSamples.size() > kMaxSamples) {
        String message(FormatString("This bank uses %i samples. The maximum is %i. Save aborted.",
                                    mSamples.size(),
                                    kMaxSamples));
        TheDebug.Notify(message.c_str());
        return -1;
    }

    std::map<int, InstrumentWin *>::iterator instIt;
    std::map<int, SampleDescWin *>::iterator sdIt;
    std::map<int, SampleWin *>::iterator sampleIt;
    std::map<int, unsigned short> sampleIndices;
    std::vector<SampleWin *> samples;
    for (sampleIt = mSamples.begin(); sampleIt != mSamples.end(); ++sampleIt) {
        SampleWin *swin = sampleIt->second;
        ASSERT(swin);
        int sId = sampleIt->first;
        ASSERT(sId == swin->GetID());
        samples.push_back(swin);
        int index = samples.size() - 1;
        sampleIndices[sId] = index;
    }

    std::vector<SampleWin *> usedSamples;
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        for (sdIt = iWin->GetSampleDescs().begin(); sdIt != iWin->GetSampleDescs().end(); ++sdIt) {
            SampleDescWin *sdWin = sdIt->second;
            ASSERT(sdWin);
            int sampleID = sdWin->GetSampleID();
            ASSERT(sampleID >= 0);
            SampleWin *sWin = mSamples[sampleID];
            ASSERT(sWin);
            int index = FindInVector(usedSamples, sWin);
            if (index < 0) {
                usedSamples.push_back(sWin);
            }
        }
    }

    int numSampleDescs = 0;
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        numSampleDescs += iWin->GetSampleDescs().size();
        if (numSampleDescs > kMaxSampleDescriptions) {
            String message(FormatString(
                "This bank contains %i sample descriptions. The maximum is %i. Save aborted.",
                numSampleDescs,
                kMaxSampleDescriptions));
            TheDebug.Notify(message.c_str());
            return -1;
        }
    }

    String bankFilename;
    if (filename.size() > 0) {
        bankFilename = filename;
        mAudioDataDirty = true;
    } else {
        ASSERT(mFilename.size() > 0);
        bankFilename = mFilename;
    }
    if (bankFilename.Find(kBankExtension.c_str()) == String::npos) {
        bankFilename += kBankExtension.c_str();
    }
    mFilename = bankFilename;
    bool writeAudioData = mAudioDataDirty || forceAudioData;

    std::vector<unsigned char> buffer;
    BankFileIO *bio = new BankFileIO(mFilename, BankFileIO::kWrite);
    ASSERT(!!(*bio));

    if (writeAudioData) {
        bio->BeginAudioDataOutput();
        unsigned int audioDataSize = 0;
        std::vector<unsigned char> vagData;
        for (unsigned int i = 0; i < usedSamples.size(); ++i) {
            vagData.clear();
            SampleWin *sWin = usedSamples[i];
            ASSERT(sWin);
            sWin->GetAudioIO().GetVagData(
                vagData, sWin->GetSample().mLoopStart, sWin->GetSample().mLoopStop);
            sWin->GetSample().mSampleStart = audioDataSize;
            sWin->GetSample().mSampleRate = sWin->GetAudioIO().GetSampleRate();
            audioDataSize += vagData.size();
            bio->WriteAudioData(vagData);
        }
        int numPartitions;
        if (audioDataSize % kPartitionSize == 0) {
            numPartitions = audioDataSize / kPartitionSize;
        } else {
            numPartitions = audioDataSize / kPartitionSize + 1;
        }
        String message(FormatString("This bank will occupy %i partition blocks in audio memory. "
                                    "There are %i total partitions available.",
                                    numPartitions,
                                    kNumAudioPartitions));
        TheDebug.Notify(message.c_str());
        bio->EndAudioDataOutput();
    }

    unsigned int i;
    buffer.resize(Sample::StreamSize());
    BinStream &bs = *bio->BeginChunk(BankFileIO::kSampleChunk);
    for (i = 0; i < samples.size(); ++i) {
        SampleWin *sWin = samples[i];
        ASSERT(sWin);
        sWin->GetSample().ToStream(&buffer[0]);
        bs << buffer;
    }
    bio->EndChunk(&bs);

    // Every chunk comes back through the same stream, so these assignments copy it onto itself.
    bs = *bio->BeginChunk(BankFileIO::kSampleNameChunk);
    bs << kNameChunkVersion;
    for (i = 0; i < samples.size(); ++i) {
        SampleWin *sWin = samples[i];
        ASSERT(sWin);
        bs << sWin->GetName();
    }
    bio->EndChunk(&bs);

    bs = *bio->BeginChunk(BankFileIO::kSampleFilenameChunk);
    bs << kNameChunkVersion;
    for (i = 0; i < samples.size(); ++i) {
        SampleWin *sWin = samples[i];
        ASSERT(sWin);
        ASSERT(!!(sWin->GetAudioIO()));
        bs << sWin->GetAudioIO().GetFilename();
    }
    bio->EndChunk(&bs);

    buffer.resize(Bank::StreamSize());
    GetBank().mNumInstruments = mInstruments.size();
    GetBank().ToStream(&buffer[0]);
    bs = *bio->BeginChunk(BankFileIO::kBankChunk);
    bs << buffer;
    bio->EndChunk(&bs);

    bs = *bio->BeginChunk(BankFileIO::kBankNameChunk);
    bs << kNameChunkVersion;
    bs << GetName();
    bio->EndChunk(&bs);

    buffer.resize(Instrument::StreamSize());
    bs = *bio->BeginChunk(BankFileIO::kInstrumentChunk);
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        iWin->GetInstrument().mNumSampleDescs = iWin->GetSampleDescs().size();
        iWin->GetInstrument().ToStream(&buffer[0]);
        bs << buffer;
    }
    bio->EndChunk(&bs);

    bs = *bio->BeginChunk(BankFileIO::kInstrumentNameChunk);
    bs << kNameChunkVersion;
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        bs << iWin->GetName();
    }
    bio->EndChunk(&bs);

    buffer.resize(SampleDescription::StreamSize());
    bs = *bio->BeginChunk(BankFileIO::kSampleDescriptionChunk);
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        for (sdIt = iWin->GetSampleDescs().begin(); sdIt != iWin->GetSampleDescs().end(); ++sdIt) {
            SampleDescWin *sdWin = sdIt->second;
            ASSERT(sdWin);
            int sampleID = sdWin->GetSampleID();
            sdWin->GetSampleDescription().mSampleIndex = sampleIndices[sampleID];
            sdWin->GetSampleDescription().ToStream(&buffer[0]);
            bs << buffer;
        }
    }
    bio->EndChunk(&bs);

    bs = *bio->BeginChunk(BankFileIO::kSampleDescriptionNameChunk);
    bs << kNameChunkVersion;
    for (instIt = mInstruments.begin(); instIt != mInstruments.end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        for (sdIt = iWin->GetSampleDescs().begin(); sdIt != iWin->GetSampleDescs().end(); ++sdIt) {
            SampleDescWin *sdWin = sdIt->second;
            ASSERT(sdWin);
            bs << sdWin->GetName();
        }
    }
    bio->EndChunk(&bs);

    delete bio;
    mAudioDataDirty = false;
    return 0;
}

int BankManager::RetireBank(int bankID) {
    DeleteFromMap(mBanks, bankID);
    return 0;
}

int BankManager::NewBank(const String &iFilename, unsigned short bankID) {
    String filename(iFilename);
    if (filename.size() > 0) {
        std::map<int, BankWin *>::iterator it;
        for (it = mBanks.begin(); it != mBanks.end(); ++it) {
            BankWin *bnk = it->second;
            ASSERT(bnk);
            if (bnk->GetFilename() == filename) {
                TheDebug.Notify("You are already editing the bank in %s. Don't do that.",
                                filename.c_str());
                return -1;
            }
        }
    }
    BankWin *bw = new BankWin(filename, bankID, true);
    ASSERT(bw);
    AddToMap(mBanks, bw->GetID(), bw);
    return bw->GetID();
}

int BankWin::Load() {
    ASSERT(mFilename.size() > 0);
    DeleteAllInMap(mSamples);
    DeleteAllInMap(mInstruments);
    mAudioDataDirty = true;
    mBank.Init();

    std::vector<unsigned char> chunk;
    std::vector<unsigned char> record;
    String name;
    BufStream *bufStream = NULL;
    int version;
    BankFileIO *bio = new BankFileIO(mFilename, BankFileIO::kRead);
    ASSERT(bio);
    ASSERT(!!(*bio));

    bio->FindChunk(BankFileIO::kBankChunk);
    ReadWholeChunk(bio, chunk);
    bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
    *bufStream >> record;
    delete bufStream;
    bufStream = NULL;
    mBank.FromStream(&record[0]);

    bio->FindChunk(BankFileIO::kBankNameChunk);
    ReadWholeChunk(bio, chunk);
    bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
    *bufStream >> version;
    ReadTextString(*bufStream, name);
    delete bufStream;
    bufStream = NULL;
    SetName(name);

    std::vector<String> sampleFilenames;
    bio->FindChunk(BankFileIO::kSampleFilenameChunk);
    ReadWholeChunk(bio, chunk);
    bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
    *bufStream >> version;
    while (!bufStream->Eof()) {
        ReadTextString(*bufStream, name);
        sampleFilenames.push_back(name);
    }
    ASSERT(bufStream->Eof());
    delete bufStream;
    bufStream = NULL;

    std::map<unsigned short, int> sampleIDs;
    std::map<int, unsigned short> sampleIndices;
    bio->FindChunk(BankFileIO::kSampleChunk);
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        int sampleCounter = 0;
        Sample sample;
        while (!bufStream->Eof()) {
            *bufStream >> record;
            sample.FromStream(&record[0]);
            bool keepSample = true;
            bool tryAgain = false;
            int sampId = -1;
            do {
                keepSample = true;
                tryAgain = false;
                bool alreadyExists;
                sampId = NewSample(sampleFilenames[sampleCounter], &alreadyExists);
                ASSERT(!alreadyExists);
                if (sampId < 0) {
                    if (mPromptForMissingFiles) {
                        String question(
                            FormatString("The file \n%s\n could not be read. Do you want "
                                         "to try and find a replacement?\n",
                                         sampleFilenames[sampleCounter].c_str()));
                        int answer = MessageBox(NULL, question.c_str(), "File error", MB_YESNO);
                        if (answer == IDYES) {
                            SetCurrentDirectory(
                                FileGetPath(sampleFilenames[sampleCounter].c_str()));
                            CFileDialog dialog(TRUE,
                                               NULL,
                                               NULL,
                                               kReplacementDialogFlags,
                                               ".wav Files (*.wav)|*.wav||",
                                               NULL);
                            dialog.DoModal();
                            sampleFilenames[sampleCounter] =
                                static_cast<LPCTSTR>(dialog.GetPathName());
                            tryAgain = true;
                        } else {
                            String message(FormatString("The file \n%s\n could not be found. It is "
                                                        "being dropped from the bank.\n",
                                                        sampleFilenames[sampleCounter].c_str()));
                            TheDebug.Printf(message.c_str());
                            keepSample = false;
                        }
                    } else {
                        String message(FormatString("The file \n%s\n could not be found. It is "
                                                    "being dropped from the bank.\n",
                                                    sampleFilenames[sampleCounter].c_str()));
                        TheDebug.Printf(message.c_str());
                        keepSample = false;
                    }
                }
            } while (tryAgain);
            if (keepSample) {
                ASSERT(sampId >= 0);
                SampleWin *newSampleWin = GetFromMap(GetSamples(), sampId);
                ASSERT(newSampleWin);
                sampleIDs[sampleCounter] = sampId;
                sampleIndices[sampId] = sampleCounter;
                newSampleWin->GetSample().mLoopStart = sample.mLoopStart;
                newSampleWin->GetSample().mLoopStop = sample.mLoopStop;
                newSampleWin->GetSample().mSampleStart = sample.mSampleStart;
            }
            ++sampleCounter;
        }
        ASSERT(bufStream->Eof());
        ASSERT(sampleCounter == sampleFilenames.size());
        delete bufStream;
        bufStream = NULL;
    }

    bio->FindChunk(BankFileIO::kSampleNameChunk);
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        *bufStream >> version;
        unsigned short sampleIndex = 0;
        while (!bufStream->Eof()) {
            ReadTextString(*bufStream, name);
            if (IsInMap(sampleIDs, sampleIndex)) {
                int sampleID = sampleIDs[sampleIndex];
                SampleWin *sWin = GetFromMap(GetSamples(), sampleID);
                ASSERT(sWin);
                sWin->SetName(name);
            } else {
                TheDebug.Printf("just skipped getting the sample #%i.\n", sampleIndex);
            }
            ++sampleIndex;
        }
        ASSERT(bufStream->Eof());
        delete bufStream;
        bufStream = NULL;
    }

    std::vector<InstrumentWin *> instruments;
    Instrument instrument;
    bio->FindChunk(BankFileIO::kInstrumentChunk);
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        while (!bufStream->Eof()) {
            *bufStream >> record;
            instrument.FromStream(&record[0]);
            int instrumentID = NewInstrument(instrument.mProgram);
            InstrumentWin *iWin = GetFromMap(GetInstruments(), instrumentID);
            ASSERT(iWin);
            instruments.push_back(iWin);
            iWin->GetInstrument() = instrument;
        }
        ASSERT(bufStream->Eof());
        delete bufStream;
        bufStream = NULL;
    }

    bio->FindChunk(BankFileIO::kInstrumentNameChunk);
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        *bufStream >> version;
        int instrumentIndex = 0;
        while (!bufStream->Eof()) {
            ReadTextString(*bufStream, name);
            InstrumentWin *iWin = instruments[instrumentIndex];
            ASSERT(iWin);
            iWin->SetName(name);
            ++instrumentIndex;
        }
        ASSERT(bufStream->Eof());
        delete bufStream;
        bufStream = NULL;
    }

    std::vector<SampleDescWin *> indexedSampleDescs;
    bio->FindChunk(BankFileIO::kSampleDescriptionChunk);
    SampleDescription sampleDesc;
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        int sdIndex = 0;
        int instrumentIndex = 0;
        InstrumentWin *curInst = instruments[instrumentIndex];
        while (!bufStream->Eof()) {
            ASSERT(curInst);
            *bufStream >> record;
            sampleDesc.FromStream(&record[0]);
            unsigned short sampleIndex = sampleDesc.mSampleIndex;
            if (IsInMap(sampleIDs, sampleIndex)) {
                int sampleID = sampleIDs[sampleIndex];
                SampleWin *sWin = GetFromMap(GetSamples(), sampleID);
                ASSERT(sWin);
                while (true) {
                    if (sdIndex == curInst->GetInstrument().mNumSampleDescs) {
                        ++instrumentIndex;
                        curInst = instruments[instrumentIndex];
                        sdIndex = 0;
                    } else if (sdIndex < curInst->GetInstrument().mNumSampleDescs) {
                        break;
                    } else {
                        ASSERT(false);
                    }
                }
                ASSERT(curInst);
                int sdId = curInst->AttachSample(sWin);
                ASSERT(sdId >= 0);
                SampleDescWin *sdWin = GetFromMap(curInst->GetSampleDescs(), sdId);
                ASSERT(sdWin);
                indexedSampleDescs.push_back(sdWin);
                sdWin->GetSampleDescription() = sampleDesc;
            } else {
                SampleDescWin *skipped = NULL;
                indexedSampleDescs.push_back(skipped);
                TheDebug.Printf("just skipped sample description #%i looking for sample #%i.\n",
                                sdIndex,
                                sampleIndex);
            }
            ++sdIndex;
        }
        ASSERT(bufStream->Eof());
        delete bufStream;
        bufStream = NULL;
    }

    bio->FindChunk(BankFileIO::kSampleDescriptionNameChunk);
    ReadWholeChunk(bio, chunk);
    if (chunk.size() > 0) {
        bufStream = new BufStream(reinterpret_cast<char *>(&chunk[0]), chunk.size(), true);
        *bufStream >> version;
        int sdCounter = 0;
        while (!bufStream->Eof()) {
            ReadTextString(*bufStream, name);
            if (indexedSampleDescs[sdCounter] != NULL) {
                SampleDescWin *sdWin = indexedSampleDescs[sdCounter];
                ASSERT(sdWin);
                sdWin->SetName(name);
            }
            ++sdCounter;
        }
        ASSERT(bufStream->Eof());
        ASSERT(sdCounter == indexedSampleDescs.size());
        delete bufStream;
        bufStream = NULL;
    }
    return mID; // Yes, bio is never deleted.
}

BankWin *BankManager::GetBankWin(int bankID) {
    return GetFromMap(mBanks, bankID);
}

int BankWin::NewInstrument(unsigned short program) {
    std::map<int, InstrumentWin *>::iterator it;
    unsigned short nextProgram = 0;
    bool inUse = false;
    for (it = mInstruments.begin(); it != mInstruments.end(); ++it) {
        InstrumentWin *inst = it->second;
        ASSERT(inst);
        unsigned short instProgram = inst->GetInstrument().mProgram;
        if (nextProgram <= instProgram) {
            nextProgram = instProgram + 1;
        }
        if (instProgram == program) {
            inUse = true;
        }
    }
    if (inUse || program < 1) {
        program = nextProgram;
    }
    InstrumentWin *inst = new InstrumentWin(program);
    ASSERT(inst);
    AddToMap(mInstruments, inst->GetID(), inst);
    return inst->GetID();
}

int BankWin::DeleteInstrument(int instrumentID, std::vector<int> &sampleIDs) {
    sampleIDs.clear();
    InstrumentWin *iWin = GetFromMap(GetInstruments(), instrumentID);
    std::map<int, SampleDescWin *> &sampleDescs = iWin->GetSampleDescs();
    std::map<int, SampleDescWin *>::iterator it;
    for (it = sampleDescs.begin(); it != sampleDescs.end(); ++it) {
        SampleDescWin *sdWin = it->second;
        int sampleID = sdWin->GetSampleID();
        if (!FindInVector(sampleIDs, sampleID)) { // Adds only a sample found at index zero.
            sampleIDs.push_back(sampleID);
        }
    }
    DeleteFromMap(mInstruments, instrumentID);
    return 0;
}

String BankWin::GetSampleInfo(unsigned short sampleID) {
    SampleWin *sWin = GetFromMap(GetSamples(), sampleID);
    String info(sWin->GetName());
    info += "\n";
    info += sWin->GetAudioIO().GetFilename();
    info += "\n";
    info += FormatString("Sample Rate:: %i Hz\n", sWin->GetAudioIO().GetSampleRate());
    info += FormatString("Sample Size:: %i bytes\n", sWin->GetAudioIO().GetVagSize());
    std::vector<String> names;
    std::vector<int> sampleDescIDs;
    GetSampleUsers(names, sampleDescIDs, sampleID);
    int numUsers = names.size();
    if (numUsers == 0) {
        info += "\n\nThis sample is not used by any instruments.\n";
    } else {
        info += "\n\nThis sample is used by the following::\n";
        for (int i = 0; i < numUsers; ++i) {
            info += names[i];
            info += "\n";
        }
    }
    return info;
}

int BankWin::NewSample(const String &filename, bool *alreadyExists) {
    *alreadyExists = false;
    int sampleID = -1;
    std::map<int, SampleWin *>::iterator it;
    for (it = mSamples.begin(); it != mSamples.end(); ++it) {
        SampleWin *samp = it->second;
        ASSERT(samp);
        if (samp->GetAudioIO().GetFilename() == filename) {
            *alreadyExists = true;
            sampleID = samp->GetID();
        }
    }
    if (!*alreadyExists) {
        SampleWin *sw = new SampleWin(filename);
        ASSERT(sw);
        if (sw->Fail()) {
            String message(
                FormatString("There was an error creating a sample from %s.", filename.c_str()));
            TheDebug.Notify(message.c_str());
            delete sw;
        } else {
            AddToMap(mSamples, sw->GetID(), sw);
            mAudioDataDirty = true;
            sampleID = sw->GetID();
        }
    }
    return sampleID;
}

int BankWin::DeleteSample(int sampleID, std::map<int, int> &removed) {
    removed.clear();
    SampleWin *sWin = GetFromMap(mSamples, sampleID);
    std::map<int, InstrumentWin *>::iterator it;
    for (it = mInstruments.begin(); it != mInstruments.end(); ++it) {
        ASSERT(it->second);
        InstrumentWin *iWin = it->second;
        iWin->RemoveSample(sWin, removed);
    }
    DeleteFromMap(mSamples, sampleID);
    mAudioDataDirty = true;
    return 0;
}

int InstrumentWin::AttachSample(SampleWin *iSampleWin) {
    ASSERT(iSampleWin);
    SampleDescWin *sampDesc = new SampleDescWin(iSampleWin->GetID());
    ASSERT(sampDesc);
    String name(iSampleWin->GetName());
    sampDesc->SetName(name);
    SetDefaultKeymap(sampDesc);
    AddToMap(mSampleDescs, sampDesc->GetID(), sampDesc);
    return sampDesc->GetID();
}

int InstrumentWin::SetDefaultKeymap(SampleDescWin *iSampleDescWin) {
    int nextKey = -1;
    std::map<int, SampleDescWin *>::iterator it;
    for (it = mSampleDescs.begin(); it != mSampleDescs.end(); ++it) {
        SampleDescWin *sdw = it->second;
        ASSERT(sdw);
        unsigned char low;
        unsigned char base;
        unsigned char high;
        sdw->GetKeymap(&low, &base, &high);
        if (base >= nextKey) {
            nextKey = base + 1;
        }
    }
    if (nextKey == -1) {
        nextKey = 0;
    } else if (nextKey >= kNumKeys) {
        nextKey = 0;
    }
    iSampleDescWin->SetKeymap(nextKey, nextKey, nextKey);
    return 0;
}

void BankWin::GetSampleUsers(std::vector<String> &names,
                             std::vector<int> &sampleDescIDs,
                             int sampleID) {
    names.resize(0);
    sampleDescIDs.resize(0);
    std::map<int, InstrumentWin *>::iterator instIt;
    std::map<int, SampleDescWin *>::iterator sdIt;
    for (instIt = GetInstruments().begin(); instIt != GetInstruments().end(); ++instIt) {
        InstrumentWin *iWin = instIt->second;
        ASSERT(iWin);
        std::map<int, SampleDescWin *> &sampleDescs = iWin->GetSampleDescs();
        for (sdIt = sampleDescs.begin(); sdIt != sampleDescs.end(); ++sdIt) {
            SampleDescWin *sdWin = sdIt->second;
            ASSERT(sdWin);
            if (sdWin->GetSampleID() == sampleID) {
                names.push_back(sdWin->GetName());
                int sampleDescID = sdWin->GetID();
                sampleDescIDs.push_back(sampleDescID);
            }
        }
    }
}

int InstrumentWin::DeleteSampleDescription(int sampleDescID) {
    SampleDescWin *sdWin = RemoveFromMap(mSampleDescs, sampleDescID);
    delete sdWin;
    return 0;
}

int InstrumentWin::RemoveSample(SampleWin *sampleWin, std::map<int, int> &removed) {
    std::map<int, SampleDescWin *>::iterator it = mSampleDescs.begin();
    while (it != mSampleDescs.end()) {
        ASSERT(it->second);
        SampleDescWin *sdWin = it->second;
        if (sdWin->GetSampleID() == sampleWin->GetID()) {
            int sampleDescID = sdWin->GetID();
            removed[sampleDescID] = GetID();
            mSampleDescs.erase(it++);
            delete sdWin;
        } else {
            ++it;
        }
    }
    return 0;
}

bool BankLookup::HasKey(unsigned char key, int program) {
    std::map<int, InstrumentWin *> &instruments = mBankWin->GetInstruments();
    std::map<int, InstrumentWin *>::iterator it = instruments.begin();
    bool found = false;
    for (; it != instruments.end(); ++it) {
        InstrumentWin *iwin = it->second;
        ASSERT(iwin);
        if (program != kAnyProgram && program != iwin->GetInstrument().mProgram) {
            continue;
        }
        std::map<int, SampleDescWin *> &sampleDescs = iwin->GetSampleDescs();
        std::map<int, SampleDescWin *>::iterator sdIt;
        for (sdIt = sampleDescs.begin(); sdIt != sampleDescs.end(); ++sdIt) {
            SampleDescWin *sdWin = sdIt->second;
            ASSERT(sdWin);
            unsigned char low;
            unsigned char base;
            unsigned char high;
            sdWin->GetKeymap(&low, &base, &high);
            if (key >= low && key <= high) {
                found = true;
                break;
            }
        }
    }
    return found;
}

BankLookup::BankLookup(const String &filename) : mBankWin(NULL) {
    mBankWin = new BankWin(filename, 0, false);
}

BankLookup::~BankLookup() {
    delete mBankWin;
}
