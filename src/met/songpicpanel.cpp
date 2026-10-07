#include "met/songpicpanel.h"

#include "os/fileutil.h"
#include "os/string.h"
#include "rnd/manager.h"

SongPicPanel::SongPicPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    mPictureShowing = 1;
    mPictureMesh = nullptr;
    mUnlocked = 0;
    mSong = nullptr;
    mPictureReady = 0;
}

void SongPicPanel::FinishLoad() {
    FreqPanel::FinishLoad();
    mPictureMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%s.mesh", mName)));
    mPictureMesh->SetShowing(false);
}

void SongPicPanel::SetBandPicture(const char *pszSong,
                                  bool bSmall,
                                  bool bUnlocked,
                                  bool bEncrypted) {
    mUnlocked = bUnlocked;
    mEncrypted = bEncrypted;
    mSong = pszSong;
    if (mPicturePath.mLength != 0) {
        Rnd::TheManager.ReleaseTexture(mPicturePath);
    }
    FilePath::SetRoot(FileRoot());
    if (mEncrypted != 0) {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_encrypt.bmp", pszSong, pszSong));
    } else if (bSmall) {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_band_sm.bmp", pszSong, pszSong));
    } else {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_band.bmp", pszSong, pszSong));
    }
    Rnd::TheManager.AcquireTexture(mPicturePath);
    mPictureReady = 0;
}

void SongPicPanel::SetArenaPicture(const char *pszArena) {
    mSong = "";
    mUnlocked = 0;
    if (mPicturePath.mLength != 0) {
        Rnd::TheManager.ReleaseTexture(mPicturePath);
    }
    FilePath::SetRoot(FileRoot());
    mPicturePath.Set(FormatString("Metagame\\image\\%s_band.bmp", pszArena));
    Rnd::TheManager.AcquireTexture(mPicturePath);
    mPictureReady = 0;
}

void SongPicPanel::SetPictureShowing(bool bShowing) {
    mPictureShowing = bShowing;
    if (!bShowing) {
        mPictureMesh->SetShowing(false);
    } else if (mLoaded && mPictureReady != 0) {
        mPictureMesh->SetShowing(true);
    }
}

void SongPicPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
}

void SongPicPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    if (mPicturePath.mLength != 0) {
        Rnd::TheManager.ReleaseTexture(mPicturePath);
    }
}
