#include "met/freqpanel.h"

#include <cmath>
#include <list>

#include "math/transform.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndobject.h"
#include "rnd/rndtransanim.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

// The copy flags of the panel's transform animation.
constexpr unsigned kTransAnimCopyFlags = 0x100;

// The share of each corner of the background box in its centre.
constexpr float kHalf = 0.5f;

// Transform a point. The fourth word of the result is the point's own.
Vector3 XfmPoint(const Transform &xfm, const Vector3 &point) {
    Vector3 result;
    result.x = xfm.mBasisX.x * point.x + xfm.mBasisY.x * point.y + xfm.mBasisZ.x * point.z +
               xfm.mTranslation.x;
    result.y = xfm.mBasisX.y * point.x + xfm.mBasisY.y * point.y + xfm.mBasisZ.y * point.z +
               xfm.mTranslation.y;
    result.z = xfm.mBasisX.z * point.x + xfm.mBasisY.z * point.y + xfm.mBasisZ.z * point.z +
               xfm.mTranslation.z;
    result.w = point.w;
    return result;
}

// Store a row of a transform.
void SetRow(Vector3 &row, float x, float y, float z, float w) {
    row.x = x;
    row.y = y;
    row.z = z;
    row.w = w;
}

} // namespace

const char *FreqPanel::sPanelAnimName = "";
float FreqPanel::sMatEnterStart = 0.0f;
float FreqPanel::sMatEnterStop = 150.0f;
float FreqPanel::sMatExitStart = 150.0f;
float FreqPanel::sMatExitStop = 300.0f;
Vector3 FreqPanel::sGizmoOrigPos;

// The unit's static initialiser constructs sGizmoOffsets, and the global constructor of the unit
// runs the initialiser.
// NTSC-U/C: 0x0019e348, PAL: 0x001a6060
// NTSC-U/C: 0x0019e390, PAL: 0x001a60a8
Vector3 FreqPanel::sGizmoOffsets[kNumGizmoOffsets];

void FreqPanel::Init(DataArray *pConfig) {
    DataArray *pPanel = pConfig->FindArray("freq_panel", true);
    if (pPanel == nullptr) {
        return;
    }
    pPanel->FindSymbol("panel_anim_name", &sPanelAnimName, false);
    pPanel->FindVector("panel_gizmo_orig_pos", &sGizmoOrigPos, false);
    pPanel->FindVector("panel_gizmo_offset_1", &sGizmoOffsets[0], false);
    pPanel->FindVector("panel_gizmo_offset_2", &sGizmoOffsets[1], false);
    pPanel->FindVector("panel_gizmo_offset_3", &sGizmoOffsets[2], false);
    pPanel->FindFloat("panel_mat_anim_enter_start", &sMatEnterStart, false);
    pPanel->FindFloat("panel_mat_anim_enter_stop", &sMatEnterStop, false);
    pPanel->FindFloat("panel_mat_anim_exit_start", &sMatExitStart, false);
    pPanel->FindFloat("panel_mat_anim_exit_stop", &sMatExitStop, false);
}

FreqPanel::FreqPanel(DataArray *pData, const char *pszDir)
    : UIPanel(pData, pszDir), mGizmoMesh(nullptr), mBgMesh(nullptr) {
    mGizmoOffsetIndex = 0;
}

FreqPanel::~FreqPanel() {
}

void FreqPanel::FinishLoad() {
    if (mLoaded) {
        return;
    }
    UIPanel::FinishLoad();

    const char *pszPanelAnim = FormatString("%s_f2EE.anim", mName);
    mPanelAnim = dynamic_cast<RndView *>(TheManager.Find(pszPanelAnim));
    if (mPanelAnim == nullptr) {
        mPanelAnim = dynamic_cast<RndView *>(TheManager.Create(RndView::sClassName, pszPanelAnim));
        mLoader->GetObjects().push_back(mPanelAnim);

        RndTransAnim *pSource = dynamic_cast<RndTransAnim *>(TheManager.Find(sPanelAnimName));
        RndTransAnim *pAnim = dynamic_cast<RndTransAnim *>(TheManager.Create(
            RndTransAnim::sClassName, FormatString("%s_%s", mName, sPanelAnimName)));
        pAnim->Copy(pSource, kTransAnimCopyFlags);
        RndMesh *pPanelMesh =
            dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s_panel.mesh", mName)));
        pAnim->SetTrans(pPanelMesh);
        mPanelAnim->AddAnim(pAnim);
        mLoader->GetObjects().push_back(pAnim);

        if (!mData->FindInt("gizmoOffsetIndex", &mGizmoOffsetIndex, false)) {
            mGizmoOffsetIndex = 0;
        }

        std::list<RndObject *> clones;
        RndMesh *pPyramid = dynamic_cast<RndMesh *>(TheManager.Find("pyramid.mesh"));
        TheManager.Clone(pPyramid, mName, &clones, 0, 0, 0);
        RndObject *pClone = clones.front();
        mGizmoMesh = pClone != nullptr ? dynamic_cast<RndMesh *>(pClone) : nullptr;
        mLoader->GetObjects().push_back(mGizmoMesh);
        mBgMesh = dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s_bg.mesh", mName)));
        UpdateBgBox();
    } else {
        mGizmoMesh =
            dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%spyramid.mesh", mName)));
        if (mGizmoMesh != nullptr) {
            mBgMesh = dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s_bg.mesh", mName)));
            UpdateBgBox();
        }
    }

    mMatAlwaysAnim = dynamic_cast<RndView *>(TheManager.Find("mat_2d_always.anim"));
    mMatEnterExitAnim = dynamic_cast<RndView *>(TheManager.Find("mat_2d_EE.anim"));
    mMatEntering = 1;
    mMatStart = 0.0f;
}

void FreqPanel::Unload() {
    UIPanel::Unload();
    if (mLoadRefs == 0) {
        mPanelAnim = nullptr;
        mGizmoMesh = nullptr;
        mBgMesh = nullptr;
    }
}

void FreqPanel::UpdateBgBox() {
    if (mBgMesh != nullptr) {
        mBgMesh->BoundingBox(mBgBox);
    }
}

void FreqPanel::SetShowing(bool bShowing) {
    UIPanel::SetShowing(bShowing);
    ShowGizmoMesh(bShowing);
}

void FreqPanel::ShowGizmoMesh(bool bShowing) {
    if (mGizmoMesh != nullptr) {
        mGizmoMesh->SetShowing(bShowing);
    }
}

void FreqPanel::Poll(float fTime) {
    if (mLoaded == 0) {
        return;
    }
    UIPanel::Poll(fTime);
    if (mPanelAnim != nullptr && mFrame != mIdleFrame) {
        mPanelAnim->SetFrame(mFrame);
    }
    mView->UpdateWorldXfm(nullptr, 0);
}

void FreqPanel::DrawGizmo() {
    if (mLoader == nullptr || mGizmoMesh == nullptr || !mGizmoMesh->GetShowing()) {
        return;
    }

    const Transform gizmoXfm = TheMetagame.GetGizmo()->WorldXfm();
    const Vector3 tip = XfmPoint(gizmoXfm, sGizmoOffsets[mGizmoOffsetIndex]);
    SetRow(mGizmoMesh->GetLocalXfm().mTranslation, tip.x, tip.y, tip.z, tip.w);
    mGizmoMesh->SetDirty(1);

    const Transform &bgXfm = mBgMesh->GetWorldXfm();
    const Vector3 low = XfmPoint(bgXfm, mBgBox.mMin);
    const Vector3 high = XfmPoint(bgXfm, mBgBox.mMax);
    const float fCentreX = high.x * kHalf + low.x * kHalf;
    const float fCentreY = high.y * kHalf + low.y * kHalf;
    const float fCentreZ = high.z * kHalf + low.z * kHalf;

    // Yes, the binary leaves the fourth word of each basis row unset.
    SetRow(mGizmoMesh->GetLocalXfm().mBasisX, std::fabs(high.x - low.x), 0.0f, 0.0f, 0.0f);
    SetRow(mGizmoMesh->GetLocalXfm().mBasisY,
           tip.x - fCentreX,
           tip.y - fCentreY,
           tip.z - fCentreZ,
           0.0f);
    SetRow(mGizmoMesh->GetLocalXfm().mBasisZ, 0.0f, 0.0f, std::fabs(high.z - low.z), 0.0f);
    mGizmoMesh->SetDirty(1);

    mGizmoMesh->UpdateWorldXfm(mView, 0);
    mGizmoMesh->Draw();
}

void FreqPanel::Draw() {
    if (mLoaded == 0) {
        return;
    }
    const float fTime = TheUI.GetTime();
    mMatAlwaysAnim->SetFrame(fTime);
    if (mMatStart != 0.0f) {
        float fFrame = fTime - mMatStart;
        float fStop = sMatEnterStop;
        if (mMatEntering == 0) {
            fFrame += sMatExitStart;
            fStop = sMatExitStop;
        }
        if (fStop < fFrame) {
            mMatStart = 0.0f;
            fFrame = fStop;
        }
        mMatEnterExitAnim->SetFrame(fFrame);
    } else if (mMatEntering) {
        mMatEnterExitAnim->SetFrame(sMatEnterStop);
    } else {
        mMatEnterExitAnim->SetFrame(sMatExitStop);
    }
    UIPanel::Draw();
}

void FreqPanel::Enter(bool bForce, float fTime) {
    mMatEntering = 1;
    if (bForce || mState == kStateShown) {
        mMatEnterExitAnim->SetFrame(mShownFrame);
    } else {
        mMatStart = fTime;
    }
    UIPanel::Enter(bForce, fTime);
}

void FreqPanel::Exit(bool bForce, float fTime) {
    mMatEntering = 0;
    if (bForce || mState == kStateHidden) {
        mMatEnterExitAnim->SetFrame(mHiddenFrame);
    } else {
        mMatStart = fTime;
    }
    UIPanel::Exit(bForce, fTime);
}

void FreqPanel::SetFocus(UIComponent *pComponent, int nButton) {
    UIPanel::SetFocus(pComponent, nButton);
    switch (nButton) {
    case kPadDUp:
        FxMidi::PlayMenuUp();
        break;
    case kPadDRight:
        FxMidi::PlayMenuRight();
        break;
    case kPadDDown:
        FxMidi::PlayMenuDown();
        break;
    case kPadDLeft:
        FxMidi::PlayMenuLeft();
        break;
    default:
        break;
    }
}
