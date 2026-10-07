#include "game/bankloader.h"

#include "synth/synth.h"

namespace {

// The synthesiser bank slots, and the blocks of a slot the song does not use.
constexpr int kNumSynthSlots = 7;
constexpr int kUnusedSlotBlocks = -1;
constexpr int kEmptySlotBlocks = 0;

// The swapped banks take turns in two slots.
constexpr int kSwapSlots = 2;

} // namespace

BankLoader::BankLoader(
    unsigned short nFirstSlot, PlayMap *pPlayMap, int nIntroBars, int nNumBars, int nTicksPerBar)
    : mState(kStateIdle), mFirstSlot(nFirstSlot), mIntroBars(nIntroBars), mNumBars(nNumBars),
      mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap), mSwapBanks(nullptr), mLoadCount(0) {
}

BankLoader::~BankLoader() {
    Stop();
    delete mSwapBanks;
    for (auto *pBank : mPermBanks) {
        delete pBank;
    }
}

void BankLoader::AddPermBank(const char *pszFile) {
    mPermBanks.push_back(new PermBankLoader(GetEndBankSlot(), pszFile));
}

void BankLoader::AddSwapBank(const char *pszFile, int nBar) {
    if (mSwapBanks == nullptr) {
        mSwapBanks =
            new SwapBankLoader(GetEndBankSlot(), mPlayMap, mIntroBars, mNumBars, mTicksPerBar);
    }
    mSwapBanks->Add(pszFile, nBar);
}

void BankLoader::Load() {
    mState = kStateLoading;
    if (mLoadCount == 0) {
        PartitionSlots();
        for (auto *pBank : mPermBanks) {
            pBank->Load();
        }
    }
    if (mSwapBanks != nullptr) {
        mSwapBanks->Load();
    }
    ++mLoadCount;
}

bool BankLoader::PollLoad() {
    if (mState != kStateLoading) {
        return true;
    }
    for (auto *pBank : mPermBanks) {
        if (!pBank->IsLoaded()) {
            return false;
        }
    }
    if (mSwapBanks != nullptr && !mSwapBanks->PollLoad()) {
        return false;
    }
    mState = kStateLoaded;
    return true;
}

void BankLoader::Start() {
    mState = kStatePlaying;
    if (mSwapBanks != nullptr) {
        mSwapBanks->Start();
    }
}

void BankLoader::Stop() {
    if (mState == kStateIdle) {
        return;
    }
    mState = kStateIdle;
    if (mSwapBanks != nullptr) {
        mSwapBanks->Stop();
    }
}

void BankLoader::Restart() {
    if (mSwapBanks != nullptr) {
        mSwapBanks->Restart();
    }
}

int BankLoader::GetNumSlots() {
    const int nPermSlots = static_cast<int>(mPermBanks.size());
    return mSwapBanks == nullptr ? nPermSlots : nPermSlots + kSwapSlots;
}

unsigned short BankLoader::GetEndBankSlot() {
    return static_cast<unsigned short>(mFirstSlot + GetNumSlots());
}

void BankLoader::PartitionSlots() {
    (void)GetNumSlots(); // Yes, the binary discards this call's result.
    std::vector<int> blocks(kNumSynthSlots, kUnusedSlotBlocks);
    for (auto it = blocks.begin() + mFirstSlot; it != blocks.end(); ++it) {
        *it = kEmptySlotBlocks;
    }
    for (auto *pBank : mPermBanks) {
        blocks[pBank->mSlot] = pBank->mBlocks;
    }
    if (mSwapBanks != nullptr) {
        const int nBlocks = mSwapBanks->GetMaxBlocks();
        blocks[mSwapBanks->mSlot] = nBlocks;
        blocks[mSwapBanks->mSlot + 1] = nBlocks;
    }
    (void)TheSynth->Partition(blocks); // Yes, the binary discards this call's result.
}
