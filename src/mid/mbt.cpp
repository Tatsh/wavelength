#include "mid/mbt.h"

MBT::MBT(int nTick, int nBeatsPerMeasure, int nTicksPerBeat) {
    const int nBeats = nTick / nTicksPerBeat;
    const int nInBeat = nTick % nTicksPerBeat;
    mMeasure = nBeats / nBeatsPerMeasure + 1;
    mBeat = nBeats % nBeatsPerMeasure + 1;
    mTick = nInBeat;
}

String MBT::ToString() const {
    return String(FormatString("%i:%02i:%03i", mMeasure, mBeat, mTick));
}
