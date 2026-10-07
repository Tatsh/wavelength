#include "met/songpicpanel.h"

#include "os/fileutil.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/tex.h"

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
    if (!mLoaded || mPicturePath.mLength == 0 || !Rnd::TheManager.IsTextureLoaded(mPicturePath)) {
        return;
    }

    if (mPictureShowing != 0) {
        mPictureMesh->SetShowing(true);
        Rnd::Mesh *pBeat =
            dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("s_g_sel_song_pic_beat.mesh"));
        if (pBeat != nullptr) {
            const char *pszMat = mEncrypted != 0 ? "band_lock.mat" : "band_unlock.mat";
            pBeat->SetMat(dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszMat)));
        }
    }
    Rnd::Tex *pTex = dynamic_cast<Rnd::Tex *>(Rnd::TheManager.Find("band_pic.bmp"));
    pTex->SetBitmapConfig(0, 0, 0, mPicturePath, 0, 0);
    mPicturePath.Clear();
    mPictureReady = 1;
}

void SongPicPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    if (mPicturePath.mLength != 0) {
        Rnd::TheManager.ReleaseTexture(mPicturePath);
    }
}
