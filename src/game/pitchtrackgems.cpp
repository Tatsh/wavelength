#include "game/pitchtrackgems.h"

#include <algorithm>

namespace {

constexpr int kNoOwner = -1;
constexpr int kNoPattern = -1;

} // namespace

PitchTrackGems::PitchTrackGems(SectionBoundaries *pSections,
                               SlotGrid *pPatterns,
                               int nNumBars,
                               int nTicksPerBar)
    : mTicksPerBar(nTicksPerBar), mSections(pSections), mPatterns(pPatterns),
      mSecStats(pSections->NumSections(), SecStats{kNoOwner, 0}),
      mBars(nNumBars, std::vector<PitchGem>()) {
}

PitchTrackGems::~PitchTrackGems() {
}

bool PitchTrackGems::AddGem(signed char nSlot, int nTick, signed char nOwner) {
    const int nBar = nTick / mTicksPerBar;
    const PitchGem gem{nSlot, nTick, nOwner};
    std::vector<PitchGem> &gems = mBars[nBar];
    const auto it = std::lower_bound(gems.begin(), gems.end(), gem);
    if (it != gems.end() && it->mTick == nTick) {
        RemoveOwner(it->mOwner, nBar);
        AddOwner(nOwner, nBar);
        *it = gem;
        return true;
    }
    AddOwner(nOwner, nBar);
    gems.insert(it, gem);
    return false;
}

bool PitchTrackGems::RemoveGem(int nTick) {
    const int nBar = nTick / mTicksPerBar;
    const PitchGem key{0, nTick, kNoOwner};
    std::vector<PitchGem> &gems = mBars[nBar];
    const auto it = std::lower_bound(gems.begin(), gems.end(), key);
    if (it == gems.end() || it->mTick != nTick) {
        return false;
    }
    RemoveOwner(it->mOwner, nBar);
    gems.erase(it);
    return true;
}

bool PitchTrackGems::ToggleGem(signed char nSlot, int nTick, signed char nOwner) {
    const int nBar = nTick / mTicksPerBar;
    const PitchGem gem{nSlot, nTick, nOwner};
    std::vector<PitchGem> &gems = mBars[nBar];
    const auto it = std::lower_bound(gems.begin(), gems.end(), gem);
    if (it != gems.end() && it->mTick == nTick) {
        RemoveOwner(it->mOwner, nBar);
        if (it->mSlot == nSlot) {
            gems.erase(it);
            return false;
        }
        AddOwner(nOwner, nBar);
        *it = gem;
        return true;
    }
    AddOwner(nOwner, nBar);
    gems.insert(it, gem);
    return true;
}

int PitchTrackGems::GetOwner(int nBar) {
    return mSecStats[mSections->SectionAt(nBar)].mOwner;
}

void PitchTrackGems::ClearBar(int nBar) {
    std::vector<PitchGem> &gems = mBars[nBar];
    for (unsigned int i = 0; i < gems.size(); ++i) {
        RemoveOwner(gems[i].mOwner, nBar);
    }
    gems.clear();
}

void PitchTrackGems::ClearSection(int nSection) {
    mSecStats[nSection] = SecStats{kNoOwner, 0};
    int nStartBar = 0;
    int nEndBar = 0;
    mSections->GetSectionRange(nSection, &nStartBar, &nEndBar);
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        mBars[nBar].clear();
    }
}

void PitchTrackGems::CopyBars(int nStartBar, int nEndBar, int nDestBar) {
    CopyBars(nStartBar, nEndBar, nDestBar, this, kNoOwner);
}

void PitchTrackGems::CopyBarsFrom(int nStartBar, int nEndBar, PitchTrackGems *pSource, int nOwner) {
    CopyBars(nStartBar, nEndBar, nStartBar, pSource, nOwner);
}

void PitchTrackGems::CopyBars(
    int nStartBar, int nEndBar, int nDestBar, PitchTrackGems *pSource, int nOwner) {
    const int nDestEndBar = nDestBar + nEndBar - nStartBar;
    // Yes, the binary copies the whole range once for every bar in it.
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        std::copy(pSource->mBars.begin() + nStartBar,
                  pSource->mBars.begin() + nEndBar,
                  mBars.begin() + nDestBar);
    }

    const int nShift = (nDestBar - nStartBar) * mTicksPerBar;
    if (nShift != 0) {
        for (int nBar = nDestBar; nBar < nDestEndBar; ++nBar) {
            for (auto &gem : mBars[nBar]) {
                gem.mTick += nShift;
            }
        }
    }
    if (nOwner != kNoOwner) {
        for (int nBar = nDestBar; nBar < nDestEndBar; ++nBar) {
            for (auto &gem : mBars[nBar]) {
                gem.mOwner = static_cast<signed char>(nOwner);
            }
        }
    }

    const int nFirstSection = mSections->SectionAt(nDestBar);
    int nLastSection = nFirstSection;
    if (nDestEndBar - nDestBar >= 2) {
        nLastSection = mSections->SectionAt(nDestEndBar - 1);
    }
    for (int nSection = nFirstSection; nSection <= nLastSection; ++nSection) {
        if (mPatterns != nullptr && mPatterns->HasPattern(nSection)) {
            continue;
        }
        int nSectionStart = 0;
        int nSectionEnd = 0;
        mSections->GetSectionRange(nSection, &nSectionStart, &nSectionEnd);
        mSecStats[nSection] = SecStats{kNoOwner, 0};
        for (int nBar = nSectionStart; nBar < nSectionEnd; ++nBar) {
            const std::vector<PitchGem> &gems = mBars[nBar];
            for (unsigned int i = 0; i < gems.size(); ++i) {
                AddOwner(gems[i].mOwner, nBar);
            }
        }
    }
}

void PitchTrackGems::ApplyPatterns() {
    if (mPatterns == nullptr) {
        return;
    }
    const int nSections = mSections->NumSections();
    for (int nSection = 0; nSection < nSections; ++nSection) {
        const int nPattern = mPatterns->GetPattern(nSection);
        if (nPattern != kNoPattern) {
            int nPatternStart = 0;
            int nPatternEnd = 0;
            mSections->GetSectionRange(nPattern, &nPatternStart, &nPatternEnd);
            CopyBars(nPatternStart, nPatternEnd, mSections->SectionStart(nSection));
        }
    }
}

std::vector<PitchGem> *PitchTrackGems::GetBar(int nBar) {
    return &mBars[nBar];
}

int PitchTrackGems::CountGems(int nStartBar, int nEndBar) {
    int nGems = 0;
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        nGems += static_cast<int>(mBars[nBar].size());
    }
    return nGems;
}

void PitchTrackGems::Save(BinStream &stream) {
    const int nSections = mSections->NumSections();
    std::vector<int> sections;
    for (int nSection = 0; nSection < nSections; ++nSection) {
        if (mPatterns != nullptr && mPatterns->HasPattern(nSection)) {
            continue;
        }
        sections.push_back(nSection);
    }

    int nGems = 0;
    for (unsigned int i = 0; i < sections.size(); ++i) {
        int nStartBar = 0;
        int nEndBar = 0;
        mSections->GetSectionRange(sections[i], &nStartBar, &nEndBar);
        nGems += CountGems(nStartBar, nEndBar);
    }
    stream.WriteEndian(&nGems, sizeof(nGems));

    for (unsigned int i = 0; i < sections.size(); ++i) {
        int nStartBar = 0;
        int nEndBar = 0;
        mSections->GetSectionRange(sections[i], &nStartBar, &nEndBar);
        for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
            for (const auto &gem : mBars[nBar]) {
                int nTick = gem.mTick;
                stream.WriteEndian(&nTick, sizeof(nTick));
                signed char nSlot = gem.mSlot;
                stream.Write(&nSlot, sizeof(nSlot));
            }
        }
    }
}

void PitchTrackGems::Load(BinStream &stream) {
    for (unsigned int i = 0; i < mBars.size(); ++i) {
        mBars[i].clear();
    }
    for (unsigned int i = 0; i < mSecStats.size(); ++i) {
        mSecStats[i] = SecStats{kNoOwner, 0};
    }

    int nGems = 0;
    stream.ReadEndian(&nGems, sizeof(nGems));
    const int nBars = static_cast<int>(mBars.size());
    for (; nGems != 0; --nGems) {
        int nTick = 0;
        stream.ReadEndian(&nTick, sizeof(nTick));
        signed char nSlot = 0;
        stream.Read(&nSlot, sizeof(nSlot));
        const int nBar = nTick / mTicksPerBar;
        if (nBar < nBars) {
            mBars[nBar].push_back(PitchGem{nSlot, nTick, kNoOwner});
        }
    }
}

void PitchTrackGems::AddOwner(int nOwner, int nBar) {
    if (nOwner == kNoOwner) {
        return;
    }
    SecStats &stats = mSecStats[mSections->SectionAt(nBar)];
    stats.mOwner = nOwner;
    ++stats.mNumGems;
}

void PitchTrackGems::RemoveOwner(int nOwner, int nBar) {
    if (nOwner == kNoOwner) {
        return;
    }
    SecStats &stats = mSecStats[mSections->SectionAt(nBar)];
    if (stats.mOwner == kNoOwner) {
        return;
    }
    if (--stats.mNumGems == 0) {
        stats.mOwner = kNoOwner;
    }
}
