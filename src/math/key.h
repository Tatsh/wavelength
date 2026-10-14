#pragma once

#include <vector>

#include "math/color.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/prnstream.h"

/**
 * One keyframe of an animation channel, a value at a frame.
 *
 * The RTTI includes the template name and its argument.
 *
 * @tparam T The type of the value.
 */
template <typename T>
struct Key {
    T value;     /*!< The value at the frame. */
    float frame; /*!< The frame. */
};

/**
 * Find the keys around a frame.
 *
 * An empty list yields null keys. A frame at or before the first key yields the first key twice,
 * and one at or after the last key yields the last key twice. A frame between keys yields the keys
 * on either side by binary search, with the position between them, unless a key lies exactly on
 * the frame, which is then yielded twice. Every inline expansion in the program follows this
 * order of tests.
 *
 * @tparam T The type of the values.
 * @param keys The keys, in frame order.
 * @param fFrame The frame.
 * @param pPrev Receives the key at or before the frame, or null.
 * @param pNext Receives the key at or after the frame, or null.
 * @param fRatio Receives the position of the frame between the two keys, from 0 to 1. It is unset
 *        for an empty list.
 * @return The index of pNext. For a frame past the last key it is the key count, less one when the
 *         frame is exactly the last key's.
 */
template <typename T>
int AtFrame(const std::vector<Key<T>> &keys,
            float fFrame,
            const Key<T> *&pPrev,
            const Key<T> *&pNext,
            float &fRatio) {
    if (keys.empty()) {
        pNext = nullptr;
        pPrev = nullptr;
        return 0;
    }
    if (fFrame <= keys.front().frame) {
        pPrev = &keys.front();
        fRatio = 0.0f;
        pNext = &keys.front();
        return 0;
    }
    const Key<T> &last = keys.back();
    if (last.frame <= fFrame) {
        fRatio = 0.0f;
        pNext = &last;
        pPrev = &last;
        const int nSize = static_cast<int>(keys.size());
        return fFrame == last.frame ? nSize - 1 : nSize;
    }
    int nLow = 0;
    int nHigh = static_cast<int>(keys.size()) - 1;
    while (nLow + 1 < nHigh) {
        const int nMiddle = (nLow + nHigh) >> 1;
        const Key<T> &middle = keys.at(nMiddle);
        if (fFrame == middle.frame) {
            pPrev = &middle;
            pNext = &middle;
            fRatio = 0.0f;
            return nMiddle;
        }
        if (middle.frame < fFrame) {
            nLow = nMiddle;
        } else {
            nHigh = nMiddle;
        }
    }
    pPrev = &keys.at(nLow);
    pNext = &keys.at(nHigh);
    fRatio = (fFrame - keys.at(nLow).frame) / (keys.at(nHigh).frame - pPrev->frame);
    return nHigh;
}

/**
 * Blend two vectors, taking either end exactly at a position of 0 or 1.
 *
 * Between the ends the fourth word comes from the second vector.
 *
 * @param a The value at 0.
 * @param b The value at 1.
 * @param fRatio The position.
 * @param out Receives the blend.
 */
inline void Interp(const Vector3 &a, const Vector3 &b, float fRatio, Vector3 &out) {
    if (fRatio == 0.0f) {
        out = a;
    } else if (fRatio == 1.0f) {
        out = b;
    } else {
        const float fInverse = 1.0f - fRatio;
        out = b;
        out.x = b.x * fRatio + a.x * fInverse;
        out.y = b.y * fRatio + a.y * fInverse;
        out.z = b.z * fRatio + a.z * fInverse;
    }
}

/**
 * Blend two two-component vectors, taking either end exactly at a position of 0 or 1.
 *
 * @param a The value at 0.
 * @param b The value at 1.
 * @param fRatio The position.
 * @param out Receives the blend.
 */
inline void Interp(const Vector2 &a, const Vector2 &b, float fRatio, Vector2 &out) {
    if (fRatio == 0.0f) {
        out = a;
    } else if (fRatio == 1.0f) {
        out = b;
    } else {
        out.x = (b.x - a.x) * fRatio + a.x;
        out.y = (b.y - a.y) * fRatio + a.y;
    }
}

/**
 * Blend two colours.
 *
 * @param a The value at 0.
 * @param b The value at 1.
 * @param fRatio The position.
 * @param out Receives the blend.
 */
inline void Interp(const Color &a, const Color &b, float fRatio, Color &out) {
    const float fInverse = 1.0f - fRatio;
    out.r = b.r * fRatio + a.r * fInverse;
    out.g = b.g * fRatio + a.g * fInverse;
    out.b = b.b * fRatio + a.b * fInverse;
    out.a = b.a * fRatio + a.a * fInverse;
}

/**
 * Blend two floats.
 *
 * @param fA The value at 0.
 * @param fB The value at 1.
 * @param fRatio The position.
 * @param fOut Receives the blend.
 */
inline void Interp(float fA, float fB, float fRatio, float &fOut) {
    fOut = (fB - fA) * fRatio + fA;
}

/**
 * Take the first of two pointers, which do not blend.
 *
 * The addresses are of the RndTex instance.
 *
 * @tparam T The type pointed at.
 * @param pA The value at 0.
 * @param pB The value at 1. The routine ignores it.
 * @param fRatio The position. The routine ignores it.
 * @param pOut Receives pA.
 * @ghidraAddress NTSC-U/C: 0x00232058
 * @ghidraAddress PAL: 0x0023ac20
 */
template <typename T>
void Interp(T *pA, [[maybe_unused]] T *pB, [[maybe_unused]] float fRatio, T *&pOut) {
    pOut = pA;
}

/**
 * Interpolate keys at a frame.
 *
 * Nothing is written for no keys.
 *
 * @tparam T The type of the values.
 * @param keys The keys.
 * @param fFrame The frame.
 * @param out Receives the value.
 */
template <typename T>
void InterpKeys(const std::vector<Key<T>> &keys, float fFrame, T &out) {
    const Key<T> *pPrev;
    const Key<T> *pNext;
    float fRatio = 0.0f;
    AtFrame(keys, fFrame, pPrev, pNext, fRatio);
    if (pPrev != nullptr) {
        Interp(pPrev->value, pNext->value, fRatio, out);
    }
}

/**
 * Report the frame of the first key, or 0 for no keys.
 *
 * @tparam T The type of the values.
 * @param keys The keys.
 * @return The frame.
 */
template <typename T>
float FirstFrame(const std::vector<Key<T>> &keys) {
    return keys.empty() ? 0.0f : keys.front().frame;
}

/**
 * Report the frame of the last key, or 0 for no keys.
 *
 * @tparam T The type of the values.
 * @param keys The keys.
 * @return The frame.
 */
template <typename T>
float LastFrame(const std::vector<Key<T>> &keys) {
    return keys.empty() ? 0.0f : keys.back().frame;
}

/**
 * Write a float key value.
 *
 * @param stream The stream to write to.
 * @param fValue The value.
 */
inline void WriteKeyValue(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

/**
 * Write a vector key value as three floats.
 *
 * @param stream The stream to write to.
 * @param v The value.
 */
inline void WriteKeyValue(BinStream &stream, const Vector3 &v) {
    WriteKeyValue(stream, v.x);
    WriteKeyValue(stream, v.y);
    WriteKeyValue(stream, v.z);
}

/**
 * Write a two-component vector key value as two floats.
 *
 * @param stream The stream to write to.
 * @param v The value.
 */
inline void WriteKeyValue(BinStream &stream, const Vector2 &v) {
    WriteKeyValue(stream, v.x);
    WriteKeyValue(stream, v.y);
}

/**
 * Write a quaternion key value as four floats.
 *
 * @param stream The stream to write to.
 * @param quat The value.
 */
inline void WriteKeyValue(BinStream &stream, const Quat &quat) {
    WriteKeyValue(stream, quat.x);
    WriteKeyValue(stream, quat.y);
    WriteKeyValue(stream, quat.z);
    WriteKeyValue(stream, quat.w);
}

/**
 * Write a colour key value as four floats.
 *
 * @param stream The stream to write to.
 * @param color The value.
 */
inline void WriteKeyValue(BinStream &stream, const Color &color) {
    WriteKeyValue(stream, color.r);
    WriteKeyValue(stream, color.g);
    WriteKeyValue(stream, color.b);
    WriteKeyValue(stream, color.a);
}

/**
 * Read a float key value.
 *
 * @param stream The stream to read from.
 * @param fValue Receives the value.
 */
inline void ReadKeyValue(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

/**
 * Read a vector key value as three floats.
 *
 * @param stream The stream to read from.
 * @param v Receives the value.
 */
inline void ReadKeyValue(BinStream &stream, Vector3 &v) {
    ReadKeyValue(stream, v.x);
    ReadKeyValue(stream, v.y);
    ReadKeyValue(stream, v.z);
}

/**
 * Read a two-component vector key value as two floats.
 *
 * @param stream The stream to read from.
 * @param v Receives the value.
 */
inline void ReadKeyValue(BinStream &stream, Vector2 &v) {
    ReadKeyValue(stream, v.x);
    ReadKeyValue(stream, v.y);
}

/**
 * Read a quaternion key value as four floats.
 *
 * @param stream The stream to read from.
 * @param quat Receives the value.
 */
inline void ReadKeyValue(BinStream &stream, Quat &quat) {
    ReadKeyValue(stream, quat.x);
    ReadKeyValue(stream, quat.y);
    ReadKeyValue(stream, quat.z);
    ReadKeyValue(stream, quat.w);
}

/**
 * Read a colour key value as four floats.
 *
 * @param stream The stream to read from.
 * @param color Receives the value.
 */
inline void ReadKeyValue(BinStream &stream, Color &color) {
    ReadKeyValue(stream, color.r);
    ReadKeyValue(stream, color.g);
    ReadKeyValue(stream, color.b);
    ReadKeyValue(stream, color.a);
}

/**
 * Write a list key value as its count followed by each value.
 *
 * Each value type has its own instance, one address each.
 *
 * @tparam T The type of the values.
 * @param stream The stream to write to.
 * @param values The value.
 * @ghidraAddress NTSC-U/C: 0x0038bf40
 * @ghidraAddress PAL: 0x003fa648
 * @ghidraAddress NTSC-U/C: 0x0038c0e8
 * @ghidraAddress PAL: 0x003fa7f0
 * @ghidraAddress NTSC-U/C: 0x0038c278
 * @ghidraAddress PAL: 0x003fa980
 */
template <typename T>
void WriteKeyValue(BinStream &stream, const std::vector<T> &values) {
    const int nSize = static_cast<int>(values.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const T &value : values) {
        WriteKeyValue(stream, value);
    }
}

/**
 * Read a list key value the list key value writer wrote.
 *
 * Each value type has its own instance, one address each.
 *
 * @tparam T The type of the values.
 * @param stream The stream to read from.
 * @param values Receives the value.
 * @ghidraAddress NTSC-U/C: 0x0038c878
 * @ghidraAddress PAL: 0x003faf80
 * @ghidraAddress NTSC-U/C: 0x0038d080
 * @ghidraAddress PAL: 0x003fb788
 * @ghidraAddress NTSC-U/C: 0x0038d868
 * @ghidraAddress PAL: 0x003fbf70
 */
template <typename T>
void ReadKeyValue(BinStream &stream, std::vector<T> &values) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    values.resize(nSize, T());
    for (T &value : values) {
        ReadKeyValue(stream, value);
    }
}

/**
 * Write a key as its value followed by its frame.
 *
 * Each value type has its own instance, one address each.
 *
 * @tparam T The type of the value.
 * @param stream The stream to write to.
 * @param key The key.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387528
 * @ghidraAddress PAL: 0x003f5c30
 * @ghidraAddress NTSC-U/C: 0x00385478
 * @ghidraAddress PAL: 0x003f3b80
 * @ghidraAddress NTSC-U/C: 0x003855a8
 * @ghidraAddress PAL: 0x003f3cb0
 * @ghidraAddress NTSC-U/C: 0x00392128
 * @ghidraAddress PAL: 0x00400830
 * @ghidraAddress NTSC-U/C: 0x0038c000
 * @ghidraAddress PAL: 0x003fa708
 * @ghidraAddress NTSC-U/C: 0x0038c190
 * @ghidraAddress PAL: 0x003fa898
 * @ghidraAddress NTSC-U/C: 0x0038c350
 * @ghidraAddress PAL: 0x003faa58
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const Key<T> &key) {
    WriteKeyValue(stream, key.value);
    WriteKeyValue(stream, key.frame);
    return stream;
}

/**
 * Read a key the key writer wrote.
 *
 * Each value type has its own instance, one address each.
 *
 * @tparam T The type of the value.
 * @param stream The stream to read from.
 * @param key Receives the key.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387df8
 * @ghidraAddress PAL: 0x003f6500
 * @ghidraAddress NTSC-U/C: 0x00385b30
 * @ghidraAddress PAL: 0x003f4238
 * @ghidraAddress NTSC-U/C: 0x00386010
 * @ghidraAddress PAL: 0x003f4718
 * @ghidraAddress NTSC-U/C: 0x00392258
 * @ghidraAddress PAL: 0x00400960
 * @ghidraAddress NTSC-U/C: 0x0038c978
 * @ghidraAddress PAL: 0x003fb080
 * @ghidraAddress NTSC-U/C: 0x0038d170
 * @ghidraAddress PAL: 0x003fb878
 * @ghidraAddress NTSC-U/C: 0x0038d978
 * @ghidraAddress PAL: 0x003fc080
 */
template <typename T>
BinStream &operator>>(BinStream &stream, Key<T> &key) {
    ReadKeyValue(stream, key.value);
    ReadKeyValue(stream, key.frame);
    return stream;
}

/**
 * Write a key as its frame and its value.
 *
 * Each value type has its own instance, one address each.
 *
 * @tparam T The type of the value.
 * @param stream The stream to write to.
 * @param key The key.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387288
 * @ghidraAddress PAL: 0x003f5990
 * @ghidraAddress NTSC-U/C: 0x00385140
 * @ghidraAddress PAL: 0x003f3848
 * @ghidraAddress NTSC-U/C: 0x00385290
 * @ghidraAddress PAL: 0x003f3998
 * @ghidraAddress NTSC-U/C: 0x003873d8
 * @ghidraAddress PAL: 0x003f5ae0
 * @ghidraAddress NTSC-U/C: 0x00391fd8
 * @ghidraAddress PAL: 0x004006e0
 * @ghidraAddress NTSC-U/C: 0x0038b940
 * @ghidraAddress PAL: 0x003fa048
 * @ghidraAddress NTSC-U/C: 0x0038bb88
 * @ghidraAddress PAL: 0x003fa290
 * @ghidraAddress NTSC-U/C: 0x0038bdd0
 * @ghidraAddress PAL: 0x003fa4d8
 */
template <typename T>
PrnStream &operator<<(PrnStream &stream, const Key<T> &key) {
    stream << "(frame:" << key.frame << " value:" << key.value << ")";
    return stream;
}
