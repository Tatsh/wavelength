#include "math/rect.h"

PrnStream &operator<<(PrnStream &stream, const Rect &rect) {
    stream << "(x:" << rect.x << " y:" << rect.y << " w:" << rect.w << " h:" << rect.h << ")";
    return stream;
}
