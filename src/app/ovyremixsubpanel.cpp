#include "app/ovyremixsubpanel.h"

#include "math/interpolator.h"
#include "os/debug.h"
#include "os/string.h"
#include "rnd/transanim.h"

namespace {

// The ease of the slide.
constexpr float kSlideSeverity = 5.0f;

// The speed of the slide, in frames per unit of time.
constexpr float kSlideSpeed = 1.0f;

// Each panel type numbers its clones apart from the other types of the same player.
constexpr int kTypeCloneStride = 16;

// The scene names of the panel types, indexed by type.
const char *const kTypeNames[] = {nullptr, "pat", "chs", "stt", "eco", "bot", "bpm"};

constexpr int kNumTypes = static_cast<int>(sizeof(kTypeNames) / sizeof(kTypeNames[0]));

} // namespace

OvyRemixSubPanel::OvyRemixSubPanel(int nType, char chHud, int nPlayer)
    : mType(nType), mSlide(kHiddenFrame,
                           nullptr,
                           new ATanInterpolator(0.0f, 0.0f, 0.0f, 1.0f, kSlideSeverity),
                           kSlideSpeed) {
    const char *pszName = nullptr;
    if (nType > 0 && nType < kNumTypes) {
        pszName = kTypeNames[nType];
    } else {
        DebugWarn("unknown panel type: %d", nType);
    }
    String prefix(FormatString("HUD%cr_fxs_%s", chHud, pszName));
    Load(chHud, prefix.c_str(), nPlayer, nullptr);
    auto *pSlideAnim = dynamic_cast<Rnd::TransAnim *>(
        Clone(FormatString("HUD%cr_fxs.tnm", chHud), nPlayer + nType * kTypeCloneStride));
    pSlideAnim->SetTrans(mPanel);
    mSlide.SetAnim(pSlideAnim);
    mBackground->SetShowing(false);
}

OvyRemixSubPanel::~OvyRemixSubPanel() = default;

void OvyRemixSubPanel::Show(bool bShow) {
    mSlide.SetTarget(bShow ? kShownFrame : kHiddenFrame);
}

void OvyRemixSubPanel::Poll(float fDelta) {
    if (mSlide.Update(fDelta, false)) {
        mBackground->SetShowing(mSlide.mValue != kHiddenFrame);
    }
    OvyRemixGenPanel::Poll(fDelta);
}

void OvyRemixSubPanel::Draw() {
    mBackground->Draw();
}

bool OvyRemixSubPanel::IsType(int nType) const {
    return mType == nType;
}
