#include "synth/common/Bank.h"

#include <string.h>

#include "os/Debug.h"

namespace {

const int kBankVersion = 1;
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

Bank::Bank() {
    Init();
}

void Bank::Init() {
    mId = 0;
    mVolume = kDefaultVolume;
    mNumInstruments = 0;
}

int Bank::FromStream(const unsigned char *data) {
    int offset = 0;
    int version;
    ReadField(data, offset, version);
    if (version == kBankVersion) {
        Init();
        ReadField(data, offset, mId);
        ReadField(data, offset, mVolume);
        ReadField(data, offset, mNumInstruments);
    } else {
        ASSERT(false);
    }
    return offset;
}

int Bank::ToStream(unsigned char *data) const {
    int offset = 0;
    int version = kBankVersion;
    WriteField(data, offset, version);
    WriteField(data, offset, mId);
    WriteField(data, offset, mVolume);
    WriteField(data, offset, mNumInstruments);
    return offset;
}

int Bank::StreamSize() {
    return sizeof(int) + sizeof(mId) + sizeof(mVolume) + sizeof(mNumInstruments);
}
