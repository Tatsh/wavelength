#include "synth_s/banksampledesc.h"

void BankSampleDesc::Dump() const {
    SampleDesc::Dump();
    if (mSample != nullptr) {
        mSample->Dump();
    }
}

void BankSampleDesc::Init() {
    SampleDesc::Init();
    mSample = nullptr;
}

Sample *BankSampleDesc::GetSample() const {
    return mSample;
}
