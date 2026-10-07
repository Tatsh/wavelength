#include "met/songpreview.h"

#include <cstring>

#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synth.h"
#include "synth/syntheffects.h"

namespace {

constexpr char kMetagameKey[] = "metagame";
constexpr char kPreviewEffectsKey[] = "song_preview_effects";
constexpr char kEffectsKey[] = "effects";
constexpr char kAudioDir[] = "audio";
constexpr char kMetaClipFormat[] = "Songs/%s/%s_metaclip.str";
constexpr char kAudioClipFormat[] = "audio/%s.str";

// The characters of a song name an `audio` stream file retains.
constexpr int kAudioNameLength = 8;

constexpr float kMaxVolume = 1.4f;

// Poll() silences the expression of each of these channels once a clip has stopped.
constexpr int kMidiChannelCount = 15;
constexpr unsigned char kMidiControlChange = 0xb0;
constexpr unsigned char kMidiControlExpression = 0x11;

} // namespace

const char *SongPreview::sLevelEncrypt = "SoundFX/level_encrypt.str";
const char *SongPreview::sBonusEncrypt = "SoundFX/bonus_encrypt.str";
int SongPreview::sState = SongPreview::kStateMenu;
StreamPlayer *SongPreview::sPlayer = nullptr;
float SongPreview::sVolume = 0.0f;
float SongPreview::sFadeRate = 0.0f;
float SongPreview::sFadeStep = 0.05f;
int SongPreview::sRestoreMusic = 1;
String SongPreview::sClip;

void SongPreview::OnMenuRestored([[maybe_unused]] int nArg) {
    sState = kStateMenu;
}

void SongPreview::OnClipReady([[maybe_unused]] int nArg) {
    sState = kStateReady;
}

void SongPreview::Stop(float fFadeStep) {
    if (sState == kStateMenu) {
        TheSynth->SetOutputLevel(0.0f);
        SynthEffects effects(
            SystemConfig()->FindArray(kMetagameKey, true)->FindArray(kPreviewEffectsKey, true));
        effects.Apply();
    }
    sState = kStateStarting;
    TheSynth->QueueCallback(OnClipReady, 0);
    sFadeStep = fFadeStep;
}

void SongPreview::End(bool bRestoreMusic) {
    sRestoreMusic = bRestoreMusic;
    FadeOut();
    sClip.Clear();
    sState = kStateEnding;
}

bool SongPreview::IsWaiting() {
    return sState == kStateReady;
}

void SongPreview::Load(const char *pszSong, bool bMetaClip) {
    FadeOut();
    if (strcmp(pszSong, sLevelEncrypt) == 0 || strcmp(pszSong, sBonusEncrypt) == 0) {
        sClip = pszSong;
    } else if (bMetaClip) {
        sClip = FormatString(kMetaClipFormat, pszSong, pszSong);
    } else {
        char szName[kAudioNameLength + 1];
        strncpy(szName, pszSong, kAudioNameLength);
        szName[kAudioNameLength] = '\0';
        sClip = FormatString(kAudioClipFormat, szName);
    }
}

void SongPreview::FadeOut() {
    sFadeRate = -sFadeStep;
}

bool SongPreview::IsIdle() {
    return sPlayer == nullptr && sClip.mLength == 0;
}

bool SongPreview::IsPlaying() {
    return sState == kStateMenu;
}

void SongPreview::Poll() {
    if (sState == kStateMenu || sState == kStateRestoring) {
        return;
    }

    if (sPlayer != nullptr) {
        const float fLastVolume = sVolume;
        sVolume += sFadeRate;
        if (kMaxVolume <= sVolume) {
            sVolume = kMaxVolume;
        }
        if (sVolume <= 0.0f) {
            sVolume = 0.0f;
        }
        if (fLastVolume != sVolume) {
            TheSynth->SetOutputLevel(sVolume);
        }
        if (!sPlayer->IsPlaying() || sVolume == 0.0f) {
            TheSynth->SetStream(nullptr);
            delete sPlayer;
            sVolume = 0.0f;
            sPlayer = nullptr;
        }
    } else if (sClip.mLength != 0) {
        sFadeRate = sFadeStep;
        sVolume = 0.0f;
        const bool bAudio = sClip.Find(kAudioDir) != String::npos;
        sPlayer = new StreamPlayer(0, sClip.c_str(), bAudio);
        TheSynth->SetOutputLevel(sVolume);
        TheSynth->SetStream(sPlayer);
        sPlayer->SetPlaying(true);
        sClip.Clear();
    }

    if (sState != kStateEnding || sPlayer != nullptr) {
        return;
    }
    if (sRestoreMusic != 0) {
        SynthEffects effects(
            SystemConfig()->FindArray(kMetagameKey, true)->FindArray(kEffectsKey, true));
        effects.Apply();
        TheSynth->SetOutputLevel(0.0f);
    } else {
        (void)TheSynth->DisableSoftFx(); // Yes, the binary discards the result.
    }
    for (int i = 0; i < kMidiChannelCount; ++i) {
        TheSynth->SendMessage(kMidiControlChange | i, kMidiControlExpression, 0, 0);
    }
    sState = kStateRestoring;
    TheSynth->QueueCallback(OnMenuRestored, 0);
}
