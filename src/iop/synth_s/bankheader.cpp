#include "synth_s/bankheader.h"

namespace {

constexpr int kBankHeaderVersion = 1;
constexpr unsigned char kDefaultBankVolume = 100;

// The bank file stores records back to back. Records are read and written unaligned.
struct __attribute__((packed)) BankHeaderRecord {
    int mVersion;
    unsigned short mId;
    unsigned char mVolume;
    unsigned short mNumPrograms;
};

} // namespace

BankHeader::BankHeader() {
    Init();
}

void BankHeader::Init() {
    mId = 0;
    mVolume = kDefaultBankVolume;
    mNumPrograms = 0;
}

int BankHeader::Unpack(const unsigned char *data) {
    const auto *record = static_cast<const BankHeaderRecord *>(static_cast<const void *>(data));
    int consumed = sizeof record->mVersion;
    if (record->mVersion == kBankHeaderVersion) {
        Init();
        mId = record->mId;
        mVolume = record->mVolume;
        mNumPrograms = record->mNumPrograms;
        consumed = sizeof(BankHeaderRecord);
    }
    return consumed;
}

int BankHeader::Pack(unsigned char *data) const {
    auto *record = static_cast<BankHeaderRecord *>(static_cast<void *>(data));
    record->mVersion = kBankHeaderVersion;
    record->mId = mId;
    record->mVolume = mVolume;
    record->mNumPrograms = mNumPrograms;
    return sizeof(BankHeaderRecord);
}

int BankHeader::PackedSize() const {
    return sizeof(BankHeaderRecord);
}
