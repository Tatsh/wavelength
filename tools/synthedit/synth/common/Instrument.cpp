#include "synth/common/Instrument.h"

#include <string.h>

#include "os/Debug.h"

namespace {

const int kInstrumentVersion = 1;
const unsigned char kCentrePan = 64;
const unsigned char kDefaultVolume = 100;

template <class T>
inline void ReadField(const unsigned char *data, int &offset, T &field) {
    memcpy(&field, data + offset, sizeof(field));
    offset += sizeof(field);
}

template <class T>
inline void WriteField(unsigned char *data, int &offset, const T &field) {
    memcpy(data + offset, &field, sizeof(field));
    offset += sizeof(field);
}

} // namespace

Instrument::Instrument() {
    Init();
}

void Instrument::Init() {
    mProgram = 0;
    mPan = kCentrePan;
    mVolume = kDefaultVolume;
    mTranspose = 0;
    mFineTranspose = 0;
    mNumSampleDescs = 0;
}

int Instrument::FromStream(const unsigned char *data) {
    int offset = 0;
    int version;
    ReadField(data, offset, version);
    if (version == kInstrumentVersion) {
        Init();
        ReadField(data, offset, mProgram);
        ReadField(data, offset, mPan);
        ReadField(data, offset, mVolume);
        ReadField(data, offset, mTranspose);
        ReadField(data, offset, mFineTranspose);
        ReadField(data, offset, mNumSampleDescs);
    } else {
        TheDebug.Printf("version# = %i.\n", version);
        ASSERT(false);
    }
    return offset;
}

int Instrument::ToStream(unsigned char *data) const {
    int offset = 0;
    int version = kInstrumentVersion;
    WriteField(data, offset, version);
    WriteField(data, offset, mProgram);
    WriteField(data, offset, mPan);
    WriteField(data, offset, mVolume);
    WriteField(data, offset, mTranspose);
    WriteField(data, offset, mFineTranspose);
    WriteField(data, offset, mNumSampleDescs);
    return offset;
}

int Instrument::StreamSize() {
    return sizeof(int) + sizeof(mProgram) + sizeof(mPan) + sizeof(mVolume) + sizeof(mTranspose) +
           sizeof(mFineTranspose) + sizeof(mNumSampleDescs);
}
