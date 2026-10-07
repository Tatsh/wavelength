#include "met/freqpanel.h"

#include <cmath>
#include <list>

#include "math/transform.h"
#include "met/gizmo.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/object.h"
#include "rnd/transanim.h"
#include "rnd/transformable.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

// Rows of a transform, and components of a row.
constexpr int kRowX = 0;
constexpr int kRowY = 1;
constexpr int kRowZ = 2;
constexpr int kRowTranslation = 3;
constexpr int kX = 0;
constexpr int kY = 1;
constexpr int kZ = 2;
constexpr int kW = 3;

// The copy flags of the panel's transform animation.
constexpr unsigned kTransAnimCopyFlags = 0x100;

// The share of each corner of the background box in its centre.
constexpr float kHalf = 0.5f;

// Copy a transform kept as four rows of four floats.
Transform ToTransform(const float xfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount]) {
    Transform result;
    Vector3 *rows[] = {&result.mBasisX, &result.mBasisY, &result.mBasisZ, &result.mTranslation};
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        rows[i]->x = xfm[i][kX];
        rows[i]->y = xfm[i][kY];
        rows[i]->z = xfm[i][kZ];
        rows[i]->w = xfm[i][kW];
    }
    return result;
}

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

// Store a row of a transform kept as four rows of four floats.
void SetRow(float *pRow, float x, float y, float z, float w) {
    pRow[kX] = x;
    pRow[kY] = y;
    pRow[kZ] = z;
    pRow[kW] = w;
}

} // namespace

const char *FreqPanel::sPanelAnimName = "";
float FreqPanel::sMatEnterStart = 0.0f;
float FreqPanel::sMatEnterStop = 150.0f;
float FreqPanel::sMatExitStart = 150.0f;
float FreqPanel::sMatExitStop = 300.0f;
Vector3 FreqPanel::sGizmoOrigPos;
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
    mPanelAnim = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(pszPanelAnim));
    if (mPanelAnim == nullptr) {
        mPanelAnim = dynamic_cast<Rnd::View *>(Rnd::TheManager.Create("View", pszPanelAnim));
        mLoader->mObjects.push_back(mPanelAnim);

        Rnd::TransAnim *pSource =
            dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find(sPanelAnimName));
        Rnd::TransAnim *pAnim = dynamic_cast<Rnd::TransAnim *>(
            Rnd::TheManager.Create("TransAnim", FormatString("%s_%s", mName, sPanelAnimName)));
        pAnim->Copy(pSource, kTransAnimCopyFlags);
        Rnd::Mesh *pPanelMesh =
            dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%s_panel.mesh", mName)));
        pAnim->SetTrans(pPanelMesh);
        mPanelAnim->AddAnim(pAnim);
        mLoader->mObjects.push_back(pAnim);

        if (!mData->FindInt("gizmoOffsetIndex", &mGizmoOffsetIndex, false)) {
            mGizmoOffsetIndex = 0;
        }

        std::list<Rnd::Object *> clones;
        Rnd::Mesh *pPyramid = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("pyramid.mesh"));
        Rnd::TheManager.Clone(pPyramid, mName, &clones, 0, 0, 0);
        Rnd::Object *pClone = clones.front();
        mGizmoMesh = pClone != nullptr ? dynamic_cast<Rnd::Mesh *>(pClone) : nullptr;
        mLoader->mObjects.push_back(mGizmoMesh);
        mBgMesh =
            dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%s_bg.mesh", mName)));
        UpdateBgBox();
    } else {
        mGizmoMesh =
            dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%spyramid.mesh", mName)));
        if (mGizmoMesh != nullptr) {
            mBgMesh =
                dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%s_bg.mesh", mName)));
            UpdateBgBox();
        }
    }

    mMatAlwaysAnim = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("mat_2d_always.anim"));
    mMatEnterExitAnim = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("mat_2d_EE.anim"));
    mMatEntering = true;
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
        mBgMesh->BoundingBox(&mBgBox);
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
    if (!mLoaded) {
        return;
    }
    UIPanel::Poll(fTime);
    if (mPanelAnim != nullptr && mFrame != mIdleFrame) {
        mPanelAnim->SetFrame(mFrame);
    }
    mView->UpdateWorldXfm(nullptr, 0);
}

void FreqPanel::DrawGizmo() {
    if (mLoader == nullptr || mGizmoMesh == nullptr || !mGizmoMesh->mShowing) {
        return;
    }

    const Transform gizmoXfm = TheMetagame.mGizmo->WorldXfm();
    const Vector3 tip = XfmPoint(gizmoXfm, sGizmoOffsets[mGizmoOffsetIndex]);
    SetRow(mGizmoMesh->mLocalXfm[kRowTranslation], tip.x, tip.y, tip.z, tip.w);
    mGizmoMesh->mDirty = 1;

    const Transform bgXfm = ToTransform(mBgMesh->mWorldXfm);
    const Vector3 low = XfmPoint(bgXfm, mBgBox.mMin);
    const Vector3 high = XfmPoint(bgXfm, mBgBox.mMax);
    const float fCentreX = high.x * kHalf + low.x * kHalf;
    const float fCentreY = high.y * kHalf + low.y * kHalf;
    const float fCentreZ = high.z * kHalf + low.z * kHalf;

    // Yes, the binary leaves the fourth word of each basis row unset.
    SetRow(mGizmoMesh->mLocalXfm[kRowX], std::fabs(high.x - low.x), 0.0f, 0.0f, 0.0f);
    SetRow(
        mGizmoMesh->mLocalXfm[kRowY], tip.x - fCentreX, tip.y - fCentreY, tip.z - fCentreZ, 0.0f);
    SetRow(mGizmoMesh->mLocalXfm[kRowZ], 0.0f, 0.0f, std::fabs(high.z - low.z), 0.0f);
    mGizmoMesh->mDirty = 1;

    mGizmoMesh->UpdateWorldXfm(mView, 0);
    mGizmoMesh->Draw();
}

void FreqPanel::Draw() {
    if (!mLoaded) {
        return;
    }
    const float fTime = TheUI.mTime;
    mMatAlwaysAnim->SetFrame(fTime);
    if (mMatStart != 0.0f) {
        float fFrame = fTime - mMatStart;
        float fStop = sMatEnterStop;
        if (!mMatEntering) {
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
    mMatEntering = true;
    if (bForce || mState == kStateShown) {
        mMatEnterExitAnim->SetFrame(mShownFrame);
    } else {
        mMatStart = fTime;
    }
    UIPanel::Enter(bForce, fTime);
}

void FreqPanel::Exit(bool bForce, float fTime) {
    mMatEntering = false;
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
