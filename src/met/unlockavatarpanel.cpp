#include "met/unlockavatarpanel.h"

UnlockAvatarPanel::UnlockAvatarPanel(DataArray *pData, const char *pszDir)
    : AvatarPanel(pData, pszDir) {
}

void UnlockAvatarPanel::SetParts(const AvatarPartSet &parts) {
    mParts = parts;
    SetAvatar(&mParts);
}

bool UnlockAvatarPanel::UpdateAvatar() {
    if (mAvatar == nullptr) {
        return true;
    }
    return mAvatar->UpdatePlayer();
}
