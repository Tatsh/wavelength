#include "met/netsongpanel.h"

#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "ui/uilist.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kBgMeshFormat[] = "%s_bg.mesh";
constexpr char kTabHiliteMat[] = "panel_tab_hi.mat";
constexpr char kTabMat[] = "panel_tab.mat";
constexpr char kBgHiliteMat[] = "bg_hi.mat";
constexpr char kBgMat[] = "bg_no.mat";

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

void SetMeshMat(const char *pszMeshFormat, const char *pszPanel, const char *pszMat) {
    Rnd::Mesh *pMesh = FindObject<Rnd::Mesh>(FormatString(pszMeshFormat, pszPanel));
    pMesh->SetMat(FindObject<Rnd::Mat>(pszMat));
}

} // namespace

void NetSongPanel::Focus() {
    if (mData == nullptr) {
        return;
    }
    UIPanel::Focus();
    static_cast<UIList *>(FindComponent(kListComponent, false))->UpdateCursor();
    SetMeshMat(kPanelMeshFormat, mName, kTabHiliteMat);
    SetMeshMat(kBgMeshFormat, mName, kBgHiliteMat);
}

void NetSongPanel::Unfocus() {
    if (mData == nullptr) {
        return;
    }
    static_cast<UIList *>(FindComponent(kListComponent, false))->SetCursorSelected(false);
    SetMeshMat(kPanelMeshFormat, mName, kTabMat);
    SetMeshMat(kBgMeshFormat, mName, kBgMat);
}
