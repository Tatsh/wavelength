#include "met/metamusicsong.h"

#include <memory>
#include <vector>

#include "os/fileutil.h"
#include "os/system.h"

MetaMusicSong::MetaMusicSong(Mix *pMix, DataArray *pConfig) {
    mMix = pMix;
    mBuilder = nullptr;
    mLoaded = 0;
    mConfig = pConfig;
    FindBankFile();
}

MetaMusicSong::~MetaMusicSong() = default;

void MetaMusicSong::StartLoad() {
    mBuilder = std::make_unique<MixMidiBuilder>(mConfig);
    mBuilder->StartLoad();
    mLoaded = 0;
}

void MetaMusicSong::Poll() {
    if (!mBuilder->IsDone()) {
        mBuilder->Poll();
    } else if (mLoaded == 0) {
        AddTracks();
        mLoaded = 1;
    }
}

int MetaMusicSong::IsLoaded() const {
    return mLoaded;
}

const char *MetaMusicSong::BankFile() const {
    return mBankFile.c_str();
}

void MetaMusicSong::FindBankFile() {
    DataArray *pBank = mConfig->FindArray("bank_file", true);
    mBankFile = String(FileGetPath(pBank->GetFile())) + "/" + pBank->Sym(1);
}

void MetaMusicSong::AddTracks() {
    int nBars = 0;
    // Yes, the binary reads the value and never uses it.
    SystemConfig()->FindArray("metagame", false)->FindInt("music_bars", &nBars, true);
    for (int i = 0; i < mBuilder->NumTracks(); ++i) {
        Muse *pMuse = mBuilder->TrackMuse(i);
        const unsigned char nChannel = mBuilder->TrackChannel(i);
        mMix->AddTrack(pMuse, nChannel, mBuilder->TrackName(i));
    }

    DataArray *pMixes = mConfig->FindArray("mix", true);
    for (int i = 1; i < pMixes->Size(); ++i) {
        DataArray *pMix = pMixes->Array(i);
        const int nMix = pMix->Int(0);
        std::vector<String> names;
        for (int j = 1; j < pMix->Size(); ++j) {
            names.push_back(String(pMix->Sym(j)));
        }
        mMix->SetMixTracks(nMix, names);
    }
}
