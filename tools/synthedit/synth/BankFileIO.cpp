#include "synth/BankFileIO.h"

#include <string.h>

#include "os/Debug.h"
#include "utl/BufStream.h"

namespace {

const int kTagSize = 4;
const int kAudioDataAlignment = 64;
// Origins of File::Seek().
const int kSeekBegin = 0;
const int kSeekCurrent = 1;

// 0x100321d8
const char kBankChunkTag[] = { 'B', 'A', 'N', 'K' };
// 0x100321dc
const char kInstrumentChunkTag[] = { 'I', 'N', 'S', 'T' };
// 0x100321e0
const char kSampleDescriptionChunkTag[] = { 'S', 'D', 'E', 'S' };
// 0x100321e4
const char kSampleChunkTag[] = { 'S', 'A', 'M', 'P' };
// 0x100321e8
const char kSampleFilenameChunkTag[] = { 'S', 'A', 'F', 'N' };
// 0x100321ec
const char kBankNameChunkTag[] = { 'B', 'K', 'N', 'M' };
// 0x100321f0
const char kInstrumentNameChunkTag[] = { 'I', 'N', 'N', 'M' };
// 0x100321f4
const char kSampleDescriptionNameChunkTag[] = { 'S', 'D', 'N', 'M' };
// 0x100321f8
const char kSampleNameChunkTag[] = { 'S', 'A', 'N', 'M' };
// 0x100321fc
const char kNullChunkTag[] = { 'X', 'X', 'X', 'X' };

} // namespace

BankFileIO::BankFileIO(const String &filename, int ioDirection)
    : mAudioFileStream(NULL), mFilename(filename), mAudioDataOutputFile(NULL),
      mBankOutputFile(NULL), mBankAsyncInputFile(NULL), mReadState(kReadDone),
      mIODirection(ioDirection), mChunkStart(0) {
    if (ioDirection == kRead) {
        mBankAsyncInputFile = NewFile(mFilename.c_str(), FILE_OPEN_READ, 0);
        if (mBankAsyncInputFile == NULL) {
            return;
        }
    } else if (ioDirection == kWrite) {
        mBankOutputFile = new FileStream(mFilename.c_str(), FileStream::kWrite, true, 0);
        if (mBankOutputFile == NULL) {
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
    return mBankOutputFile == NULL && mBankAsyncInputFile == NULL;
}

void BankFileIO::PackChunkHeader(const char *header, std::vector<unsigned char> &data) {
    data.resize(sizeof(mHeaderData));
    int i = 0;
    for (i = 0; i < kTagSize; ++i) {
        data[i] = header[i];
    }
    memcpy(&data[i], header + kTagSize, sizeof(int));
}

void BankFileIO::UnpackChunkHeader(char *header, std::vector<unsigned char> &data) {
    int i = 0;
    for (i = 0; i < kTagSize; ++i) {
        header[i] = data[i];
    }
    memcpy(header + kTagSize, &data[i], sizeof(int));
}

String BankFileIO::AudioDataFilename(const String &iBankFileName) {
    ASSERT(iBankFileName.size() > 0);
    String filename(iBankFileName);
    int extension = filename.Find(kBankExtension.c_str());
    filename.Replace(extension, kAudioDataExtension.size(), kAudioDataExtension);
    return filename;
}

void BankFileIO::BeginAudioDataOutput() {
    String filename = AudioDataFilename(mFilename);
    ASSERT(mAudioDataOutputFile == NULL);
    mAudioDataOutputFile = new FileStream(filename.c_str(), FileStream::kWrite, true, 0);
    ASSERT(!mAudioDataOutputFile->Fail());
}

void BankFileIO::WriteAudioData(std::vector<unsigned char> &data) {
    ASSERT(mAudioDataOutputFile);
    if (data.size() > 0) {
        mAudioDataOutputFile->Write(&data[0], data.size());
    }
}

void BankFileIO::EndAudioDataOutput() {
    delete mAudioDataOutputFile;
    mAudioDataOutputFile = NULL;
}

BinStream *BankFileIO::BeginChunk(int chunkType) {
    ASSERT(mIODirection == kWrite);
    ASSERT(mBankOutputFile);
    const char *chunk = GetChunkTag(chunkType);
    ASSERT(chunk);
    memcpy(mCurTag, chunk, sizeof(mCurTag));
    mChunkSize = 0;
    mChunkStart = mBankOutputFile->Tell();
    for (int i = 0; i < kTagSize; ++i) {
        *mBankOutputFile << mCurTag[i];
    }
    *mBankOutputFile << mChunkSize;
    return mBankOutputFile;
}

const char *BankFileIO::GetChunkTag(int chunkType) {
    const char *chunk = NULL;
    switch (chunkType) {
    case kBankChunk:
        chunk = kBankChunkTag;
        break;
    case kInstrumentChunk:
        chunk = kInstrumentChunkTag;
        break;
    case kSampleDescriptionChunk:
        chunk = kSampleDescriptionChunkTag;
        break;
    case kSampleChunk:
        chunk = kSampleChunkTag;
        break;
    case kSampleFilenameChunk:
        chunk = kSampleFilenameChunkTag;
        break;
    case kBankNameChunk:
        chunk = kBankNameChunkTag;
        break;
    case kInstrumentNameChunk:
        chunk = kInstrumentNameChunkTag;
        break;
    case kSampleDescriptionNameChunk:
        chunk = kSampleDescriptionNameChunkTag;
        break;
    case kSampleNameChunk:
        chunk = kSampleNameChunkTag;
        break;
    default:
        ASSERT(false);
        break;
    }
    return chunk;
}

void BankFileIO::EndChunk(BinStream *stream) {
    int end = stream->Tell();
    stream->Seek(mChunkStart, BinStream::kSeekBegin);
    mChunkSize = end - mChunkStart - sizeof(mHeaderData);
    stream->Seek(mChunkStart, BinStream::kSeekBegin); // Yes, the binary seeks here twice.
    for (int i = 0; i < kTagSize; ++i) {
        *stream << mCurTag[i];
    }
    *stream << mChunkSize;
    stream->Seek(end, BinStream::kSeekBegin);
}

void BankFileIO::OpenAudioData(int bufferSize) {
    ASSERT(mAudioFileStream == NULL);
    String filename = AudioDataFilename(mFilename);
    mAudioFileStream = new LazyFileStream(filename, bufferSize);
}

bool BankFileIO::GetAudioData(std::vector<unsigned char> &oAudioData,
                              int *position,
                              bool *gotData) {
    ASSERT(mAudioFileStream);
    bool moreToCome = true;
    *gotData = false;
    *position = 0;
    *gotData = false;
    if (mAudioFileStream->ReadDone()) {
        DebugPrint("GetAudioData - finished reading.");
        int bytesAvailable;
        char *data = mAudioFileStream->GetData(position, &bytesAvailable);
        ASSERT(data);
        ASSERT(bytesAvailable >= 0);
        int zeroPadding =
            kAudioDataAlignment - static_cast<unsigned int>(bytesAvailable) % kAudioDataAlignment;
        if (zeroPadding == kAudioDataAlignment) {
            zeroPadding = 0;
        }
        ASSERT(bytesAvailable + zeroPadding <= oAudioData.capacity());
        oAudioData.resize(bytesAvailable + zeroPadding);
        *gotData = true;
        memcpy(&oAudioData[0], data, bytesAvailable);
        if (zeroPadding > 0) {
            memset(&oAudioData[bytesAvailable], 0, zeroPadding);
        }
        moreToCome = true;
    } else {
        DebugPrint("GetAudioData - still reading.");
    }
    if (mAudioFileStream->Eof()) {
        delete mAudioFileStream;
        mAudioFileStream = NULL;
        moreToCome = false;
    }
    return moreToCome;
}

void BankFileIO::FindChunk(int chunkType) {
    ASSERT(mIODirection == kRead);
    ASSERT(mBankAsyncInputFile);
    const char *chunk = GetChunkTag(chunkType);
    ASSERT(chunk);
    memcpy(mChunkTag, chunk, sizeof(mChunkTag));
    mBankAsyncInputFile->Seek(0, kSeekBegin);
    mBankAsyncInputFile->ReadAsync(mHeaderData, sizeof(mHeaderData));
    mReadState = kReadingHeader;
}

bool BankFileIO::ReadChunkHeader(std::vector<unsigned char> &data) {
    ASSERT(mBankAsyncInputFile);
    bool started = false;
    int bytes = 0;
    if (mBankAsyncInputFile->ReadDone(&bytes)) {
        ASSERT(bytes == sizeof(mHeaderData));
        BufStream *bs = new BufStream(mHeaderData, sizeof(mHeaderData), true);
        char tag[kTagSize];
        for (int i = 0; i < kTagSize; ++i) {
            *bs >> tag[i];
        }
        int size;
        *bs >> size;
        ASSERT(bs->Eof());
        delete bs;
        if (memcmp(tag, mChunkTag, sizeof(tag)) == 0) {
            DebugPrint(" ");
            data.resize(size, 0);
            if (data.size() > 0) {
                mBankAsyncInputFile->ReadAsync(&data[0], size);
            }
            started = true;
        } else {
            DebugPrint(" ");
            mBankAsyncInputFile->Seek(size, kSeekCurrent);
            ASSERT(!(mBankAsyncInputFile->Eof()));
            mBankAsyncInputFile->ReadAsync(mHeaderData, sizeof(mHeaderData));
            started = ReadChunkHeader(data);
        }
    } else {
        DebugPrint("Reading Bank file from disk...");
    }
    return started;
}

bool BankFileIO::ReadChunk(std::vector<unsigned char> &data) {
    ASSERT(mBankAsyncInputFile);
    ASSERT(mIODirection == kRead);
    bool stillReading = true;
    int bytes = 0;
    switch (mReadState) {
    case kReadingHeader:
        if (ReadChunkHeader(data)) {
            mReadState = kReadingData;
            stillReading = ReadChunk(data);
        }
        break;
    case kReadingData:
        if (mBankAsyncInputFile->ReadDone(&bytes)) {
            DebugPrint(" ");
            mReadState = kReadDone;
            stillReading = false;
        }
        break;
    case kReadDone:
        DebugPrint(" ");
        stillReading = false;
        break;
    default:
        ASSERT(false);
        break;
    }
    return stillReading;
}
