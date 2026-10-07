#include "met/controllerpanel.h"

#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

constexpr char kViewName[] = "o_controller_map.view";
constexpr char kAnimFormat[] = "o_controller_map_%s.tnm";
constexpr char kMeshFormat[] = "o_controller_map_%s.mesh";

// The regions in the order of JoypadButton.
constexpr const char *kRegionNames[] = {
    "l2",
    "r2",
    "l1",
    "r1",
    "triangle",
    "circle",
    "x",
    "square",
    "select",
    "l3",
    "r3",
    "start",
    "dpadu",
    "dpadr",
    "dpadd",
    "dpadl",
};

} // namespace

ControllerPanel::ControllerPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    for (Region &region : mRegions) {
        region.mAnim = nullptr;
        region.mMesh = nullptr;
    }
}

void ControllerPanel::Enter(bool bForce, float fTime) {
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kViewName));
    for (int i = 0; i < kNumRegions; ++i) {
        mRegions[i].Load(kRegionNames[i]);
    }
    mView->RemoveAllAnims();
    for (Region &region : mRegions) {
        region.mMesh->SetShowing(false);
    }
    FreqPanel::Enter(bForce, fTime);
    mRegion = kPadNone;
}

void ControllerPanel::Poll(float fTime) {
    if (mView != nullptr) {
        mView->SetFrame(fTime);
    }
    FreqPanel::Poll(fTime);
}

void ControllerPanel::SetRegion(int nRegion) {
    if (mRegion != kPadNone) {
        Region &previous = mRegions[mRegion];
        mView->RemoveAnim(previous.mAnim);
        previous.mMesh->SetShowing(false);
    }
    if (nRegion != kPadNone) {
        Region &next = mRegions[nRegion];
        next.mMesh->SetShowing(true);
        mView->AddAnim(next.mAnim);
    }
    mRegion = nRegion;
}

void ControllerPanel::Region::Load(const char *pszName) {
    mAnim =
        dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(FormatString(kAnimFormat, pszName)));
    mMesh = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find(FormatString(kMeshFormat, pszName)));
}
