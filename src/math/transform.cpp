#include "math/transform.h"

namespace {

// Write the three basis rows, each on its own tab-indented line.
// NTSC-U/C: 0x00293018, PAL: 0x0029c9e0
PrnStream &WriteBasis(PrnStream &stream, const Transform &transform) {
    stream << "\n\t" << transform.mBasisX << "\n\t" << transform.mBasisY << "\n\t"
           << transform.mBasisZ;
    return stream;
}

} // namespace

PrnStream &operator<<(PrnStream &stream, const Transform &transform) {
    WriteBasis(stream, transform) << "\n\t" << transform.mTranslation;
    return stream;
}
