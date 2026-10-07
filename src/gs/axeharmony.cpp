#include "gs/axeharmony.h"

namespace {

constexpr float kHalf = 0.5f;

} // namespace

AxeHarmony::AxeHarmony() : mNotes(), mCenter(0.0f) {
}

AxeHarmony::~AxeHarmony() {
}

void AxeHarmony::AddNote(unsigned char nNote) {
    mNotes.insert(nNote);
    const int nSum = GetLowest() + GetHighest();
    mCenter = static_cast<float>(nSum) * kHalf;
}

unsigned char AxeHarmony::GetHighest() const {
    return *--mNotes.end();
}

unsigned char AxeHarmony::GetLowest() const {
    return *mNotes.begin();
}

void AxeHarmony::SnapContour(AxeContour *pContour, int nTranspose) const {
    const int nContourSum = pContour->GetHighestKey() + pContour->GetLowestKey();
    nTranspose += static_cast<int>(mCenter - static_cast<float>(nContourSum) * kHalf);
    int nPreviousKey = 0;
    int nPreviousNote = 0;
    for (int i = 0; i < pContour->GetNumNotes(); ++i) {
        const int nKey = pContour->GetNoteKey(i);
        int nNote;
        if (i == 0) {
            nNote = Snap(nKey + nTranspose);
        } else if (nKey == nPreviousKey) {
            nNote = nPreviousNote;
        } else {
            nNote = Snap(nPreviousNote + (nKey - nPreviousKey));
            if (nNote == nPreviousNote) {
                // Step on to the neighbouring note in the direction the contour moves.
                auto it = mNotes.find(static_cast<unsigned char>(nPreviousNote));
                if (nPreviousKey < nKey) {
                    if (nPreviousNote != GetHighest()) {
                        ++it;
                    }
                } else if (nPreviousNote != GetLowest()) {
                    --it;
                }
                nNote = *it;
            }
        }
        pContour->SetNoteKey(i, static_cast<unsigned char>(nNote));
        nPreviousKey = nKey;
        nPreviousNote = nNote;
    }
}

unsigned char AxeHarmony::Snap(int nNote) const {
    // The search takes the low byte of the note, and the comparisons take the whole note.
    const int nClamped = nNote > -1 ? nNote : 0;
    auto it = mNotes.lower_bound(static_cast<unsigned char>(nClamped));
    if (it == mNotes.end()) {
        return *--it;
    }
    const int nAbove = *it;
    if (nAbove == nClamped || it == mNotes.begin()) {
        return static_cast<unsigned char>(nAbove);
    }
    const int nBelow = *--it;
    return static_cast<unsigned char>(nClamped - nBelow < nAbove - nClamped ? nBelow : nAbove);
}
