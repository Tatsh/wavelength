#include "met/bonuspicpanel.h"

#include "os/string.h"
#include "rnd/manager.h"

namespace {

// Bit 0 lets the queued loads continue in the background. The meaning of bit 2 is not recovered.
constexpr int kBonusLoadFlags = 5;

} // namespace

BonusPicPanel::BonusPicPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mSong(nullptr), mBonusLoader(nullptr) {
}

void BonusPicPanel::Load() {
    if (mLoadRefs == 0) {
        mBonusLoader = Rnd::TheManager.AddLoader(
            FormatString("Songs\\%s\\bonus.rnd", mSong), kBonusLoadFlags, nullptr, nullptr);
    }
    UIPanel::Load();
}

void BonusPicPanel::Unload() {
    FreqPanel::Unload();
    if (mLoadRefs == 0) {
        delete mBonusLoader;
        mBonusLoader = nullptr;
    }
}
