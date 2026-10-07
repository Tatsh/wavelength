#include "game/bankloader.h"

#include <algorithm>

#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/synth.h"

namespace {

// The synthesiser bank slots, and the blocks of a slot the song does not use.
constexpr int kNumSynthSlots = 7;
constexpr int kUnusedSlotBlocks = -1;
constexpr int kEmptySlotBlocks = 0;

// The swapped banks take turns in two slots.
constexpr int kSwapSlots = 2;

constexpr int kNoBank = -1;
constexpr int kLeadBar = -1;
constexpr int kFirstBank = 0;

// The ticks into a bar the next load waits for, past the change of bank.
constexpr int kAdvanceDelayTicks = 800;

constexpr int kUnknownBlockCount = -1;

} // namespace

BankLoader::PermBankLoader::PermBankLoader(unsigned short nSlot, const char *pszFile)
    : mSlot(nSlot), mFile(pszFile), mBlockCount(kUnknownBlockCount), mLoading(0) {
    mBlockCount = TheSynth->GetBankBlockCount(mFile);
}

BankLoader::PermBankLoader::~PermBankLoader() {
    if (mLoading) {
        TheSynth->UnloadBank(mSlot);
    }
}

void BankLoader::PermBankLoader::Load() {
    mLoading = 1;
    TheSynth->LoadBank(mSlot, mFile, true);
}

bool BankLoader::PermBankLoader::IsLoaded() const {
    if (!mLoading) {
        return false;
    }
    return TheSynth->IsBankLoaded(mSlot);
}

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
    mSwapBanks->AddBank(pszFile, nBar);
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
        mSwapBanks->Unload();
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
        blocks[pBank->mSlot] = pBank->mBlockCount;
    }
    if (mSwapBanks != nullptr) {
        const int nBlocks = mSwapBanks->GetMaxBankSize();
        blocks[mSwapBanks->mBaseSlot] = nBlocks;
        blocks[mSwapBanks->mBaseSlot + 1] = nBlocks;
    }
    (void)TheSynth->Partition(blocks); // Yes, the binary discards this call's result.
}

BankLoader::SwapBankLoader::SwapBankLoader(
    unsigned short nBaseSlot, PlayMap *pPlayMap, int nLeadBars, int nNumBars, int nTicksPerBar)
    : mBaseSlot(nBaseSlot), mLeadBars(nLeadBars), mNumBars(nNumBars), mBanks(),
      mBankOfBar(nNumBars + nLeadBars, kNoBank), mState(kStateIdle), mLoadCount(0),
      mAdvanceCmd(NewMemFunCommand(this, &SwapBankLoader::Advance)) {
    mTicksPerBar = nTicksPerBar;
    mPlayMap = pPlayMap;
    mLoaded[1] = nullptr;
    mLoaded[0] = nullptr;
}

BankLoader::SwapBankLoader::~SwapBankLoader() {
    Unload();
}

void BankLoader::SwapBankLoader::AddBank(const char *pszFile, int nStartBar) {
    if (!mBanks.empty()) {
        (void)GetBank(nStartBar); // The binary discards the bank.
    }
    const int nSize = TheSynth->GetBankBlockCount(String(pszFile));
    mBanks.push_back(SwapBank{String(pszFile), nSize});
    for (int nBar = nStartBar; nBar < mNumBars; ++nBar) {
        SetBank(nBar, static_cast<int>(mBanks.size()) - 1);
    }
}

int BankLoader::SwapBankLoader::GetMaxBankSize() const {
    int nMax = 0;
    for (const auto &bank : mBanks) {
        if (nMax < bank.mSize) {
            nMax = bank.mSize;
        }
    }
    return nMax;
}

void BankLoader::SwapBankLoader::Load() {
    mState = kStateLoading;
    if (GetBank(kLeadBar) != kNoBank) {
        LoadBank(kFirstBank);
    }
}

bool BankLoader::SwapBankLoader::PollLoad() {
    if (mState != kStateLoading) {
        return true;
    }
    if (!TheSynth->IsBankLoaded(mBaseSlot)) {
        return false;
    }
    mState = kStateLoaded;
    return true;
}

void BankLoader::SwapBankLoader::Start() {
    Advance();
}

void BankLoader::SwapBankLoader::Unload() {
    if (mState == kStateIdle) {
        return;
    }
    mState = kStateIdle;
    TheSongScheduler.Cancel(mAdvanceCmd.Get());
    if (mLoadCount > 0) {
        TheSynth->UnloadBank(mBaseSlot);
        mLoaded[0] = nullptr;
    }
    if (mLoadCount >= kNumSlots) {
        TheSynth->UnloadBank(static_cast<unsigned short>(mBaseSlot + 1));
        mLoaded[1] = nullptr;
    }
    mLoadCount = 0;
}

void BankLoader::SwapBankLoader::Restart() {
    TheSongScheduler.Cancel(mAdvanceCmd.Get());
    Advance();
}

void BankLoader::SwapBankLoader::Advance() {
    const int nTick = TheSongScheduler.mTick;
    int nBar = nTick / mTicksPerBar;
    int nNext;
    if (!mPlayMap->mLooping) {
        const int nCurrent = GetBank(nBar % mNumBars);
        do {
            ++nBar;
            nNext = GetBank(nBar % mNumBars);
        } while (nNext == nCurrent);
    } else {
        // Look for a change of bank up to the end of the stretch that follows the current one.
        const int nCurrent = GetBank(mPlayMap->MapBar(nBar));
        int nChangeTick;
        int nEnd;
        int nNextStart;
        int nNextLength;
        int nIsLast;
        mPlayMap->GetSegment(nTick, &nChangeTick, &nEnd, &nNextStart, &nNextLength, &nIsLast);
        nNext = nCurrent;
        while (nBar < nChangeTick / mTicksPerBar + nNextLength / mTicksPerBar) {
            ++nBar;
            nNext = GetBank(mPlayMap->MapBar(nBar));
            if (nNext != nCurrent) {
                break;
            }
        }
        if (nNext == nCurrent) {
            return;
        }
    }
    LoadBank(nNext);
    TheSongScheduler.PostAt(mAdvanceCmd.Get(), nBar * mTicksPerBar + kAdvanceDelayTicks, false);
}

unsigned short BankLoader::SwapBankLoader::GetLoadSlot() const {
    return static_cast<unsigned short>(mBaseSlot + mLoadCount % kNumSlots);
}

void BankLoader::SwapBankLoader::LoadBank(int nBank) {
    SwapBank &bank = mBanks[nBank];
    const char *pszText = bank.mFile.c_str();
    const int nHeld = static_cast<int>(std::find(mLoaded, mLoaded + kNumSlots, pszText) - mLoaded);
    if (nHeld < kNumSlots) {
        // The slot the next load would use already has the bank. The other slot changes nothing.
        if (mBaseSlot + nHeld == GetLoadSlot()) {
            ++mLoadCount;
        }
        return;
    }
    const unsigned short nSlot = GetLoadSlot();
    TheSynth->UnloadBank(nSlot);
    TheSynth->LoadBank(nSlot, bank.mFile, false);
    mLoaded[nSlot - mBaseSlot] = bank.mFile.c_str();
    ++mLoadCount;
}
