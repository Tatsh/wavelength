#include "met/controllerpanel.h"

#include "os/joypad.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

constexpr char kViewName[] = "o_controller_map.view";
constexpr char kAnimFormat[] = "o_controller_map_%s.tnm";
constexpr char kMeshFormat[] = "o_controller_map_%s.mesh";

} // namespace

ControllerPanel::ControllerPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    for (Region &region : mRegions) {
        region.SetAnim(nullptr);
        region.SetMesh(nullptr);
    }
}

void ControllerPanel::Enter(bool bForce, float fTime) {
    mView = dynamic_cast<RndView *>(TheManager.Find(kViewName));
    mRegions[kPadL2].Load("l2");
    mRegions[kPadR2].Load("r2");
    mRegions[kPadL1].Load("l1");
    mRegions[kPadR1].Load("r1");
    mRegions[kPadTriangle].Load("triangle");
    mRegions[kPadCircle].Load("circle");
    mRegions[kPadCross].Load("x");
    mRegions[kPadSquare].Load("square");
    mRegions[kPadSelect].Load("select");
    mRegions[kPadL3].Load("l3");
    mRegions[kPadR3].Load("r3");
    mRegions[kPadStart].Load("start");
    mRegions[kPadDUp].Load("dpadu");
    mRegions[kPadDRight].Load("dpadr");
    mRegions[kPadDDown].Load("dpadd");
    mRegions[kPadDLeft].Load("dpadl");
    mView->RemoveAllAnims();
    for (Region &region : mRegions) {
        region.GetMesh()->SetShowing(false);
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
        mView->RemoveAnim(previous.GetAnim());
        previous.GetMesh()->SetShowing(false);
    }
    if (nRegion != kPadNone) {
        Region &next = mRegions[nRegion];
        next.GetMesh()->SetShowing(true);
        mView->AddAnim(next.GetAnim());
    }
    mRegion = nRegion;
}

void ControllerPanel::Region::Load(const char *pszName) {
    mAnim = dynamic_cast<RndAnimatable *>(TheManager.Find(FormatString(kAnimFormat, pszName)));
    mMesh = dynamic_cast<RndDrawable *>(TheManager.Find(FormatString(kMeshFormat, pszName)));
}
