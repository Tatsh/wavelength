#include "met/songpicpanel.h"

#include <cstring>

#include "os/fileutil.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtex.h"

SongPicPanel::SongPicPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    mPictureShowing = 1;
    mPictureMesh = nullptr;
    mUnlocked = 0;
    mSong = nullptr;
    mPictureReady = 0;
}

void SongPicPanel::FinishLoad() {
    FreqPanel::FinishLoad();
    mPictureMesh = dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s.mesh", mName)));
    mPictureMesh->SetShowing(false);
}

void SongPicPanel::SetBandPicture(const char *pszSong,
                                  bool bSmall,
                                  bool bUnlocked,
                                  bool bEncrypted) {
    mUnlocked = bUnlocked;
    mEncrypted = bEncrypted;
    mSong = pszSong;
    if (mPicturePath.GetLength() != 0) {
        TheManager.ReleaseTexture(mPicturePath);
    }
    FilePath::SetRoot(FileRoot());
    if (mEncrypted != 0) {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_encrypt.bmp", pszSong, pszSong));
    } else if (bSmall) {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_band_sm.bmp", pszSong, pszSong));
#ifdef VIDEO_STANDARD_PAL
    } else if (strstr(pszSong, "TUT") != nullptr && !SystemLanguageIsEnglish()) {
        // A tutorial shows the picture of the console's language.
        mPicturePath.Set(
            FormatString("Songs\\%s\\%s_band_%s.bmp", pszSong, pszSong, GetSystemLanguage()));
#endif
    } else {
        mPicturePath.Set(FormatString("Songs\\%s\\%s_band.bmp", pszSong, pszSong));
    }
    TheManager.AcquireTexture(mPicturePath);
    mPictureReady = 0;
}

void SongPicPanel::SetArenaPicture(const char *pszArena) {
    mSong = "";
    mUnlocked = 0;
    if (mPicturePath.GetLength() != 0) {
        TheManager.ReleaseTexture(mPicturePath);
    }
    FilePath::SetRoot(FileRoot());
#ifdef VIDEO_STANDARD_PAL
    if (SystemLanguageIsEnglish()) {
        mPicturePath.Set(FormatString("Metagame\\image\\%s_band.bmp", pszArena));
    } else {
        mPicturePath.Set(
            FormatString("Metagame\\image\\%s_band_%s.bmp", pszArena, GetSystemLanguage()));
    }
#else
    mPicturePath.Set(FormatString("Metagame\\image\\%s_band.bmp", pszArena));
#endif
    TheManager.AcquireTexture(mPicturePath);
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
    if (mLoaded == 0 || mPicturePath.GetLength() == 0 ||
        !TheManager.IsTextureLoaded(mPicturePath)) {
        return;
    }

    if (mPictureShowing != 0) {
        mPictureMesh->SetShowing(true);
        RndMesh *pBeat = dynamic_cast<RndMesh *>(TheManager.Find("s_g_sel_song_pic_beat.mesh"));
        if (pBeat != nullptr) {
            const char *pszMat = mEncrypted != 0 ? "band_lock.mat" : "band_unlock.mat";
            pBeat->SetMat(dynamic_cast<RndMat *>(TheManager.Find(pszMat)));
        }
    }
    RndTex *pTex = dynamic_cast<RndTex *>(TheManager.Find("band_pic.bmp"));
    pTex->SetBitmap(0, 0, 0, mPicturePath, 0, 0);
    mPicturePath.Clear();
    mPictureReady = 1;
}

void SongPicPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    if (mPicturePath.GetLength() != 0) {
        TheManager.ReleaseTexture(mPicturePath);
    }
}
