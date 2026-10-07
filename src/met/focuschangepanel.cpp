#include "met/focuschangepanel.h"

#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"

namespace {

constexpr char kDefaultPrefix[] = "sub";
constexpr char kPrefixTag[] = "focus_material_prefix";
constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kBgMeshFormat[] = "%s_bg.mesh";
constexpr char kFocusedPanelMatFormat[] = "panel_%s_hi.mat";
constexpr char kPanelMatFormat[] = "panel_%s.mat";
constexpr char kFocusedBgMat[] = "bg_hi.mat";
constexpr char kBgMat[] = "bg_no.mat";

// Find a mesh by a name built from a format.
inline Rnd::Mesh *FindMesh(const char *pszFormat, const char *pszArg) {
    return dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString(pszFormat, pszArg)));
}

// Find a material by name.
inline Rnd::Mat *FindMat(const char *pszName) {
    return dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszName));
}

} // namespace

FocusChangePanel::FocusChangePanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mMaterialPrefix(kDefaultPrefix) {
    pData->FindString(kPrefixTag, &mMaterialPrefix, false);
}

void FocusChangePanel::Focus() {
    UIPanel::Focus();
    if (!mLoaded) {
        return;
    }
    Rnd::Mesh *pPanel = FindMesh(kPanelMeshFormat, mName);
    pPanel->SetMat(FindMat(FormatString(kFocusedPanelMatFormat, mMaterialPrefix.c_str())));
    Rnd::Mesh *pBg = FindMesh(kBgMeshFormat, mName);
    pBg->SetMat(FindMat(kFocusedBgMat));
}

void FocusChangePanel::Unfocus() {
    if (!mLoaded) {
        return;
    }
    Rnd::Mesh *pPanel = FindMesh(kPanelMeshFormat, mName);
    pPanel->SetMat(FindMat(FormatString(kPanelMatFormat, mMaterialPrefix.c_str())));
    Rnd::Mesh *pBg = FindMesh(kBgMeshFormat, mName);
    pBg->SetMat(FindMat(kBgMat));
}
