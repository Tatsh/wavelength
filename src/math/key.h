#pragma once

#include <vector>

#include "math/color.h"
#include "math/quaternion.h"
#include "math/transform.h"
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
        const Key<T> &middle = keys[nMiddle];
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
    pPrev = &keys[nLow];
    pNext = &keys[nHigh];
    fRatio = (fFrame - pPrev->frame) / (pNext->frame - pPrev->frame);
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
 * @tparam T The type pointed at.
 * @param pA The value at 0.
 * @param pB The value at 1. The routine ignores it.
 * @param fRatio The position. The routine ignores it.
 * @param pOut Receives pA.
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
 * Write a key as its value followed by its frame.
 *
 * Each value type has its own instance. The vector instance is at `0x00387528`.
 *
 * @tparam T The type of the value.
 * @param stream The stream to write to.
 * @param key The key.
 * @return The stream.
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
 * Each value type has its own instance. The vector instance is at `0x00387df8`.
 *
 * @tparam T The type of the value.
 * @param stream The stream to read from.
 * @param key Receives the key.
 * @return The stream.
 */
template <typename T>
BinStream &operator>>(BinStream &stream, Key<T> &key) {
    ReadKeyValue(stream, key.value);
    ReadKeyValue(stream, key.frame);
    return stream;
}

/**
 * Write keys as their count followed by each key.
 *
 * Each value type has its own instance. The vector instance is at `0x003875b8`.
 *
 * @tparam T The type of the values.
 * @param stream The stream to write to.
 * @param keys The keys.
 * @return The stream.
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::vector<Key<T>> &keys) {
    const int nSize = static_cast<int>(keys.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const Key<T> &key : keys) {
        stream << key;
    }
    return stream;
}

/**
 * Read keys the key list writer wrote.
 *
 * Each value type has its own instance. The vector instance is at `0x00387e60`.
 *
 * @tparam T The type of the values.
 * @param stream The stream to read from.
 * @param keys Receives the keys.
 * @return The stream.
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::vector<Key<T>> &keys) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    keys.resize(nSize, Key<T>());
    for (Key<T> &key : keys) {
        stream >> key;
    }
    return stream;
}

/**
 * Write a key as its frame and its value.
 *
 * Each value type has its own instance. The vector instance is at `0x00387288`.
 *
 * @tparam T The type of the value.
 * @param stream The stream to write to.
 * @param key The key.
 * @return The stream.
 */
template <typename T>
PrnStream &operator<<(PrnStream &stream, const Key<T> &key) {
    stream << "(frame:" << key.frame << " value:" << key.value << ")";
    return stream;
}

/**
 * Write keys as their count followed by one tab-indented line per key.
 *
 * Each value type has its own instance. The vector instance is at `0x00387300`.
 *
 * @tparam T The type of the values.
 * @param stream The stream to write to.
 * @param keys The keys.
 * @return The stream.
 */
template <typename T>
PrnStream &operator<<(PrnStream &stream, const std::vector<Key<T>> &keys) {
    stream << "(size:" << static_cast<unsigned int>(keys.size()) << ")";
    int nIndex = 0;
    for (const Key<T> &key : keys) {
        stream << "\n" << nIndex << "\t" << key;
        ++nIndex;
    }
    return stream;
}
