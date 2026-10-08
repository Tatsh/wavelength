#include "met/redbookplaybackscreen.h"

#include <iterator>

#include "math/rand.h"
#include "met/metagame.h"
#include "met/songpicpanel.h"
#include "met/songpreview.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kBandPanel[] = "jbox_band";
constexpr char kJukeboxScreen[] = "jbox_redbook";
constexpr char kHelpToken[] = "redbook_play_HELP";
constexpr char kActionToken[] = "redbook_play_ACTION";

// The rate at which the menu music fades out.
constexpr float kMusicFadeStep = 0.05f;

} // namespace

RedbookPlaybackScreen::RedbookPlaybackScreen(DataArray *pData)
    : FreqScreen(pData), mPlaying(0), mStopping(0) {
}

void RedbookPlaybackScreen::ClearSongs() {
    mSongsLeft.clear();
    mSongs.clear();
}

void RedbookPlaybackScreen::AddSong(const char *pszSong) {
    mSongsLeft.push_back(pszSong);
    mSongs.push_back(pszSong);
}

void RedbookPlaybackScreen::StopPlayback() {
    mNextSong.Clear();
    mSongsLeft.clear();
    mSongs.clear();
    SongPreview::End(true);
}

void RedbookPlaybackScreen::PickSong() {
    if (mSongsLeft.empty()) {
        mSongsLeft = mSongs;
    }
    auto song = mSongsLeft.begin();
    if (mShuffle != 0) {
        std::advance(song, RandomInt(0, static_cast<int>(mSongsLeft.size())));
    }
    const char *pszSong = *song;
    mSongsLeft.erase(song);

    auto *pBand = static_cast<SongPicPanel *>(TheUI.FindPanel(kBandPanel, false));
    pBand->SetBandPicture(pszSong, true, false, false);
    pBand->SetPictureShowing(true);
    mNextSong = pszSong;
}

void RedbookPlaybackScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mStopping != 0) {
        if (mNextSong.mLength == 0 && SongPreview::IsPlaying()) {
            TheUI.GotoScreen(kJukeboxScreen);
            mStopping = 0;
            mPlaying = 0;
        }
        return;
    }
    if (!SongPreview::IsWaiting() || !SongPreview::IsIdle() || mPlaying == 0) {
        return;
    }
    auto *pBand = static_cast<SongPicPanel *>(TheUI.FindPanel(kBandPanel, false));
    if (mNextSong.mLength == 0) {
        PickSong();
    } else if (pBand->mPictureReady != 0) {
        (void)SongPreview::IsIdle(); // Yes, the binary asks again and discards the answer.
        SongPreview::Load(mNextSong.c_str(), false);
        mNextSong.Clear();
    }
}

void RedbookPlaybackScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    TheMetagame.SetHelpText(TheLocale.Localize(kHelpToken, true));
    TheMetagame.SetActionText(TheLocale.Localize(kActionToken, true));
    (void)SongPreview::IsPlaying(); // Yes, the binary discards the answer.
    SongPreview::Stop(kMusicFadeStep);
    mStopping = 0;
    mPlaying = 1;
}

bool RedbookPlaybackScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RedbookPlaybackScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadStart && pMsg->mPressed != 0 && mStopping == 0) {
        StopPlayback();
        mStopping = 1;
    }
    return FreqScreen::HandleJoypad(pMsg);
}
