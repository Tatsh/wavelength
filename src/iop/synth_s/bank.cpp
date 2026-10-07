#include "synth_s/bank.h"

// NTSC-U/C: 0x00009d18
Bank Bank::sBanks[kMaxBanks];
// NTSC-U/C: 0x000151f8
PoolAllocator Bank::sProgramPool;
// NTSC-U/C: 0x00015208
PoolAllocator Bank::sSampleDescPool;
// NTSC-U/C: 0x00015218
PoolAllocator Bank::sSamplePool;

void Bank::Init() {
    mHeader.Init();
    for (int i = 0; i < kMaxPrograms; ++i) {
        mPrograms[i] = nullptr;
    }
    for (int i = 0; i < kMaxSampleDescs; ++i) {
        mSampleDescs[i] = nullptr;
    }
    for (int i = 0; i < kMaxSamples; ++i) {
        mSamples[i] = nullptr;
    }
    mLoaded = 0;
}

void Bank::SetProgram(BankProgram *program, int index) {
    mPrograms[index] = program;
    int first = 0;
    for (int i = 0; i < index; ++i) {
        first += mPrograms[i]->mNumSampleDescs;
    }
    program->mSampleDescs = &mSampleDescs[first];
}

void Bank::SetSampleDesc(BankSampleDesc *sampleDesc, int index) {
    mSampleDescs[index] = sampleDesc;
    sampleDesc->mSample = mSamples[sampleDesc->mSampleIndex];
}

bool Bank::SetSample(Sample *sample, int index) {
    mSamples[index] = sample;
    return true;
}

BankProgram *Bank::GetProgram(int index) {
    if (index < mHeader.mNumPrograms) {
        return mPrograms[index];
    }
    // Retail counts the stored programs here and discards the count.
    for (int i = 0; mPrograms[i] != nullptr; ++i) {
    }
    return nullptr;
}

BankSampleDesc *Bank::GetSampleDesc(BankProgram *program, int index) {
    return program->mSampleDescs[index];
}

void Bank::Clear() {
    for (int i = 0; (i < kMaxPrograms) && (mPrograms[i] != nullptr); ++i) {
        sProgramPool.Free(mPrograms[i]);
        mPrograms[i] = nullptr;
    }
    for (int i = 0; (i < kMaxSampleDescs) && (mSampleDescs[i] != nullptr); ++i) {
        sSampleDescPool.Free(mSampleDescs[i]);
        mSampleDescs[i] = nullptr;
    }
    for (int i = 0; (i < kMaxSamples) && (mSamples[i] != nullptr); ++i) {
        sSamplePool.Free(mSamples[i]);
        mSamples[i] = nullptr;
    }
    Init();
}

void Bank::NewProgram(BankProgram **program) {
    *program = static_cast<BankProgram *>(sProgramPool.Alloc());
    (*program)->Init();
}

void Bank::NewSampleDesc(BankSampleDesc **sampleDesc) {
    *sampleDesc = static_cast<BankSampleDesc *>(sSampleDescPool.Alloc());
    (*sampleDesc)->Init();
}

void Bank::NewSample(Sample **sample) {
    *sample = static_cast<Sample *>(sSamplePool.Alloc());
    (*sample)->Init();
}

int Bank::Find(unsigned short id) {
    int found = -1;
    for (int i = 0; i < kMaxBanks; ++i) {
        if ((sBanks[i].mHeader.mId == id) && (sBanks[i].mLoaded != 0)) {
            found = i;
            break;
        }
    }
    return found;
}

void Bank::InitPools() {
    sProgramPool.Init(sizeof(BankProgram), kMaxPrograms);
    sSampleDescPool.Init(sizeof(BankSampleDesc), kMaxSampleDescs);
    sSamplePool.Init(sizeof(Sample), kMaxSamples);
}

int Bank::ReleasePools() {
    sProgramPool.Release();
    sSampleDescPool.Release();
    sSamplePool.Release();
    return 0;
}
