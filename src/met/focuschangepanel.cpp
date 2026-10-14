#include "met/focuschangepanel.h"

#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"

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
inline RndMesh *FindMesh(const char *pszFormat, const char *pszArg) {
    return dynamic_cast<RndMesh *>(TheManager.Find(FormatString(pszFormat, pszArg)));
}

// Find a material by name.
inline RndMat *FindMat(const char *pszName) {
    return dynamic_cast<RndMat *>(TheManager.Find(pszName));
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
    RndMesh *pPanel = FindMesh(kPanelMeshFormat, mName);
    pPanel->SetMat(FindMat(FormatString(kFocusedPanelMatFormat, mMaterialPrefix.c_str())));
    RndMesh *pBg = FindMesh(kBgMeshFormat, mName);
    pBg->SetMat(FindMat(kFocusedBgMat));
}

void FocusChangePanel::Unfocus() {
    if (!mLoaded) {
        return;
    }
    RndMesh *pPanel = FindMesh(kPanelMeshFormat, mName);
    pPanel->SetMat(FindMat(FormatString(kPanelMatFormat, mMaterialPrefix.c_str())));
    RndMesh *pBg = FindMesh(kBgMeshFormat, mName);
    pBg->SetMat(FindMat(kBgMat));
}
