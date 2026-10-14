#include "gfx/tnlblurs.h"

#include <string.h>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "rnd/ps.h"
#include "script/symbol.h"

namespace {

// The part of the screen the blur covers, x, y, w, and h: the whole screen.
constexpr float kScreenRect[] = {0.0f, 0.0f, 1.0f, 1.0f};

// The child array of a kind tagged with a symbol. A symbol tag matches by the value of its node.
inline DataArray *FindKeyArray(const DataArray *pKind, const char *pszKey) {
    DataArray::Node tag;
    tag.mSymbol = pszKey;
    return pKind->FindArray(tag.mInt);
}

} // namespace

TnlBlurs::TnlBlurs()
    : mAlpha(0.0f), mAlphaChangeUp(0.0f), mAlphaChangeDown(0.0f), mEnlarge(0), mEnlargeChangeUp(0),
      mEnlargeChangeDown(0), mConfig(nullptr), mStartOffsetKey(nullptr), mEndOffsetKey(nullptr),
      mAlphaKey(nullptr), mEnlargeKey(nullptr) {
}

TnlBlurs::~TnlBlurs() {
}

void TnlBlurs::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    FindConfigArray(pConfig, pDefaults, "blurs", &mConfig, true);
    mConfig->FindFloat("alpha_change_up", &mAlphaChangeUp, true);
    mConfig->FindFloat("alpha_change_down", &mAlphaChangeDown, true);
    mConfig->FindInt("enlarge_change_up", &mEnlargeChangeUp, true);
    mConfig->FindInt("enlarge_change_down", &mEnlargeChangeDown, true);
    mStartOffsetKey = LookupSymbol("start_offset");
    mEndOffsetKey = LookupSymbol("end_offset");
    mAlphaKey = LookupSymbol("alpha");
    mEnlargeKey = LookupSymbol("enlarge");
}

void TnlBlurs::Reset() {
    mBlurs.clear();
    mEnlarge = 0;
    mAlpha = 0.0f;
}

void TnlBlurs::Update() {
    const float flTick = TheGameDb->mSongTick;
    int nEnlarge = mEnlarge;
    float flAlpha = 0.0f;
    std::list<ActiveBlur>::iterator it = mBlurs.begin();
    while (it != mBlurs.end()) {
        if (flTick < it->mStartTick) {
            if (mAlpha == 0.0f) {
                ThePs.mBlurActive = 0;
                return;
            }
            break;
        }
        if (it->mEndTick <= flTick) {
            it = mBlurs.erase(it);
            continue;
        }
        nEnlarge = it->mEnlarge;
        flAlpha = it->mAlpha;
        if (mAlpha == 0.0f) {
            mEnlarge = nEnlarge;
        }
        break;
    }

    const float flAlphaDelta = flAlpha - mAlpha;
    if (0.0f <= flAlphaDelta) {
        if (flAlphaDelta <= mAlphaChangeUp) {
            mAlpha = flAlpha;
        } else {
            mAlpha = mAlpha + mAlphaChangeUp;
        }
    } else if (mAlphaChangeDown <= flAlphaDelta) {
        mAlpha = flAlpha;
    } else {
        mAlpha = mAlpha + mAlphaChangeDown;
    }

    const int nEnlargeDelta = nEnlarge - mEnlarge;
    if (nEnlargeDelta < 0) {
        if (nEnlargeDelta < mEnlargeChangeDown) {
            mEnlarge = mEnlarge + mEnlargeChangeDown;
        } else {
            mEnlarge = nEnlarge;
        }
    } else if (mEnlargeChangeUp < nEnlargeDelta) {
        mEnlarge = mEnlarge + mEnlargeChangeUp;
    } else {
        mEnlarge = nEnlarge;
    }

    if (mAlpha == 0.0f) {
        ThePs.mBlurActive = 0;
        return;
    }
    ThePs.mBlurAmount = mAlpha;
    ThePs.mBlurCount = mEnlarge;
    memcpy(ThePs.mBlurRect, kScreenRect, sizeof(kScreenRect));
    ThePs.mBlurActive = 1;
}

void TnlBlurs::Insert(const char *pszKind, float flStartTick, float flEndTick) {
    const DataArray *pKind = mConfig->FindArray(pszKind, true);
    const DataArray *pValue = FindKeyArray(pKind, mStartOffsetKey);
    if (pValue != nullptr) {
        flStartTick += pValue->Float(1);
    }
    pValue = FindKeyArray(pKind, mEndOffsetKey);
    if (pValue != nullptr) {
        flEndTick += pValue->Float(1);
    }
    if (flEndTick <= flStartTick) {
        return;
    }

    std::list<ActiveBlur>::iterator it = mBlurs.begin();
    while (it != mBlurs.end() && it->mStartTick <= flStartTick) {
        ++it;
    }
    pValue = FindKeyArray(pKind, mAlphaKey);
    const float flAlpha = (pValue != nullptr) ? pValue->Float(1) : 0.0f;
    pValue = FindKeyArray(pKind, mEnlargeKey);
    const int nEnlarge = (pValue != nullptr) ? pValue->Int(1) : 0;
    mBlurs.insert(it, ActiveBlur{flStartTick, flEndTick, flAlpha, nEnlarge});
}
