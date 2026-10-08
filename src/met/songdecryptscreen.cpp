#include "met/songdecryptscreen.h"

#include <iterator>
#include <list>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/bonuspicpanel.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/animatable.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

// The reveal is the texture channel of the first stage of the picture's material animation.
constexpr int kRevealStage = 0;

// The key of the channel that starts the decryption. The key after it ends the decryption.
constexpr int kDecryptKey = 1;

} // namespace

SongDecryptScreen::SongDecryptScreen(DataArray *pData)
    : FreqScreen(pData), mSong(nullptr), mPicAnim(nullptr) {
}

void SongDecryptScreen::SetSong(const char *pszSong) {
    mSong = pszSong;
    static_cast<BonusPicPanel *>(TheUI.FindPanel("s_g_bonus_pic", false))->mSong = pszSong;
}

void SongDecryptScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mPicAnim = dynamic_cast<Rnd::MatAnim *>(Rnd::TheManager.Find("band_pic.mnm"));
    Rnd::Text *pBonusText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("s_g_bonus_01.txt"));
    SongEntry entry{TheGameDb->FindSong(mSong)};
    const char *pszLabel;
    if (entry.GetType() == SongEntry::kTypeBonus) {
        pszLabel = "bonus_label";
    } else if (entry.GetType() == SongEntry::kTypeBoss) {
        pszLabel = "boss_label";
    } else {
        pszLabel = "secret_label";
    }
    const char *pszType = TheLocale.Localize(pszLabel, true);
    pBonusText->SetText(FormatString(TheLocale.Localize("s_g_bonus_01", true), pszType));
    pBonusText->UpdateCursors();

    Rnd::Text *pPicText =
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(FormatString("s_g_bonus_pic_label.txt")));
    pPicText->SetText(FormatString(TheLocale.Localize("s_g_bonus_pic_label", true), pszType));
    pPicText->UpdateCursors();

    mAnimPlayer.SetAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("s_g_bonus.view")));
    mAnimPlayer.Start(TheUI.mTime);
    mDecryptPlayed = 0;
    FreqScreen::Enter(pPrevScreen, fTime);
}

void SongDecryptScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    const float fFrame = mPicAnim->mFilteredFrame;
    mAnimPlayer.Poll(TheUI.mTime); // Yes, the binary polls with the front-end time.
    // The second texture key of the first stage starts the decryption, and the third ends it.
    const std::list<Rnd::MatAnim::Stage::TexKey> &keys =
        mPicAnim->mKeysOwner->mStages[kRevealStage].mTexKeys;
    auto key = std::next(keys.begin(), kDecryptKey);
    if (!mDecryptPlayed && key->mFrame <= fFrame) {
        FxMidi::PlayDecrypt();
        mDecryptPlayed = 1;
    }
    ++key;
    if (key->mFrame <= fFrame) {
        TheUI.GotoScreen("song_decrypt_done");
    }
}

void SongDecryptScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mAnimPlayer.Stop();
    FreqScreen::Exit(pNextScreen, fTime);
}
