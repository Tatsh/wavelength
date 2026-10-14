#include "synth/bankfileio.h"

#include <string.h>

#include "os/bufstream.h"

// This file's copies of kBankExtension and kAudioDataExtension are constructed and destroyed by one
// routine at 0x00251740. The image's static construction and destruction entries at 0x002517b0 and
// 0x002517d0 call it.
// NTSC-U/C: 0x00251740, PAL: 0x0025a0e8
// NTSC-U/C: 0x002517b0, PAL: 0x0025a158
// NTSC-U/C: 0x002517d0, PAL: 0x0025a178

namespace {

// The data of a read of the audio data file is padded to a multiple of this size.
constexpr unsigned int kAudioDataAlignment = 64;

// NTSC-U/C: 0x004126c0
const char kBankChunkTag[] = {'B', 'A', 'N', 'K'};
// NTSC-U/C: 0x004126c8
const char kInstrumentChunkTag[] = {'I', 'N', 'S', 'T'};
// NTSC-U/C: 0x004126d0
const char kSampleDescriptionChunkTag[] = {'S', 'D', 'E', 'S'};
// NTSC-U/C: 0x004126d8
const char kSampleChunkTag[] = {'S', 'A', 'M', 'P'};
// NTSC-U/C: 0x004126e0
const char kSampleFilenameChunkTag[] = {'S', 'A', 'F', 'N'};
// NTSC-U/C: 0x004126e8
const char kBankNameChunkTag[] = {'B', 'K', 'N', 'M'};
// NTSC-U/C: 0x004126f0
const char kInstrumentNameChunkTag[] = {'I', 'N', 'N', 'M'};
// NTSC-U/C: 0x004126f8
const char kSampleDescriptionNameChunkTag[] = {'S', 'D', 'N', 'M'};
// NTSC-U/C: 0x00412700
const char kSampleNameChunkTag[] = {'S', 'A', 'N', 'M'};
// NTSC-U/C: 0x00412708
const char kNullChunkTag[] = {'X', 'X', 'X', 'X'};

} // namespace

BankFileIO::BankFileIO(const String &filename, int nIODirection)
    : mAudioFileStream(nullptr), mFilename(filename), mAudioDataOutputFile(nullptr),
      mBankOutputFile(nullptr), mBankAsyncInputFile(nullptr), mReadState(kReadDone),
      mIODirection(nIODirection), mChunkStart(0) {
    if (nIODirection == kRead) {
        mBankAsyncInputFile = File::New(mFilename.c_str(), File::kModeRead, 0);
        if (mBankAsyncInputFile == nullptr) {
            return;
        }
    } else if (nIODirection == kWrite) {
        mBankOutputFile = new FileStream(mFilename.c_str(), true, true, 0);
        if (mBankOutputFile == nullptr) {
            return;
        }
    }
    memcpy(mChunkTag, kNullChunkTag, sizeof(mChunkTag));
}

BankFileIO::~BankFileIO() {
    delete mBankAsyncInputFile;
    delete mBankOutputFile;
    delete mAudioFileStream;
}

bool BankFileIO::operator!() const {
    return (mBankOutputFile == nullptr) && (mBankAsyncInputFile == nullptr);
}

String BankFileIO::AudioDataFilename(const String &bankFileName) {
    String filename(bankFileName);
    const int nExtension = filename.Find(kBankExtension.c_str());
    filename.Replace(nExtension, kAudioDataExtension.mLength, kAudioDataExtension);
    return filename;
}

const char *BankFileIO::GetChunkTag(int nChunkType) {
    const char *pChunk = nullptr;
    switch (nChunkType) {
    case kBankChunk:
        pChunk = kBankChunkTag;
        break;
    case kInstrumentChunk:
        pChunk = kInstrumentChunkTag;
        break;
    case kSampleDescriptionChunk:
        pChunk = kSampleDescriptionChunkTag;
        break;
    case kSampleChunk:
        pChunk = kSampleChunkTag;
        break;
    case kSampleFilenameChunk:
        pChunk = kSampleFilenameChunkTag;
        break;
    case kBankNameChunk:
        pChunk = kBankNameChunkTag;
        break;
    case kInstrumentNameChunk:
        pChunk = kInstrumentNameChunkTag;
        break;
    case kSampleDescriptionNameChunk:
        pChunk = kSampleDescriptionNameChunkTag;
        break;
    case kSampleNameChunk:
        pChunk = kSampleNameChunkTag;
        break;
    default:
        break;
    }
    return pChunk;
}

void BankFileIO::OpenAudioData(int nBufferSize) {
    const String filename = AudioDataFilename(mFilename);
    mAudioFileStream = new LazyFileStream(filename, nBufferSize);
}

bool BankFileIO::GetAudioData(std::vector<unsigned char> &audioData,
                              int *pnPosition,
                              bool *pbGotData) {
    bool bMoreToCome = true;
    *pbGotData = false;
    *pnPosition = 0;
    *pbGotData = false; // Yes, the binary clears the flag twice.
    if (mAudioFileStream->ReadDone()) {
        int nBytesAvailable;
        const char *pData = mAudioFileStream->GetData(pnPosition, &nBytesAvailable);
        int nZeroPadding = static_cast<int>(
            kAudioDataAlignment - static_cast<unsigned int>(nBytesAvailable) % kAudioDataAlignment);
        if (nZeroPadding == static_cast<int>(kAudioDataAlignment)) {
            nZeroPadding = 0;
        }
        audioData.resize(nBytesAvailable + nZeroPadding);
        *pbGotData = true;
        memcpy(audioData.data(), pData, nBytesAvailable);
        if (nZeroPadding > 0) {
            memset(&audioData[nBytesAvailable], 0, nZeroPadding);
        }
        bMoreToCome = true;
    }
    if (mAudioFileStream->Eof()) {
        delete mAudioFileStream;
        mAudioFileStream = nullptr;
        bMoreToCome = false;
    }
    return bMoreToCome;
}

void BankFileIO::FindChunk(int nChunkType) {
    const char *pChunk = GetChunkTag(nChunkType);
    memcpy(mChunkTag, pChunk, sizeof(mChunkTag));
    (void)mBankAsyncInputFile->Seek(0, BinStream::kSeekBegin);
    (void)mBankAsyncInputFile->ReadAsync(mHeaderData, sizeof(mHeaderData));
    mReadState = kReadingHeader;
}

bool BankFileIO::ReadChunkHeader(std::vector<unsigned char> &data) {
    bool bStarted = false;
    int nBytes = 0;
    if (mBankAsyncInputFile->ReadDone(&nBytes)) {
        BufStream *pStream = new BufStream(mHeaderData, sizeof(mHeaderData), true);
        char tag[kTagSize];
        for (int i = 0; i < kTagSize; ++i) {
            pStream->Read(&tag[i], 1);
        }
        int nSize;
        pStream->ReadEndian(&nSize, sizeof(nSize));
        (void)pStream->Eof(); // Yes, the binary discards the check.
        delete pStream;
        if (memcmp(tag, mChunkTag, sizeof(tag)) == 0) {
            data.resize(nSize, 0);
            if (!data.empty()) {
                (void)mBankAsyncInputFile->ReadAsync(data.data(), nSize);
            }
            bStarted = true;
        } else {
            (void)mBankAsyncInputFile->Seek(nSize, BinStream::kSeekCurrent);
            (void)mBankAsyncInputFile->Eof(); // Yes, the binary discards the check.
            (void)mBankAsyncInputFile->ReadAsync(mHeaderData, sizeof(mHeaderData));
            bStarted = ReadChunkHeader(data);
        }
    }
    return bStarted;
}

bool BankFileIO::ReadChunk(std::vector<unsigned char> &data) {
    bool bStillReading = true;
    int nBytes = 0;
    switch (mReadState) {
    case kReadingHeader:
        if (ReadChunkHeader(data)) {
            mReadState = kReadingData;
            bStillReading = ReadChunk(data);
        }
        break;
    case kReadingData:
        if (mBankAsyncInputFile->ReadDone(&nBytes)) {
            mReadState = kReadDone;
            bStillReading = false;
        }
        break;
    case kReadDone:
        bStillReading = false;
        break;
    default:
        break;
    }
    return bStillReading;
}
