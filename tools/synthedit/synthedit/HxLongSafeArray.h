#pragma once

#include <vector>

#include "synthedit/HxSafeArray.h"

/**
 * One-dimensional array of 32-bit integers (`VT_I4`).
 *
 * The object is 12 bytes. The name is inferred.
 */
class HxLongSafeArray : public HxSafeArray {
public:
    /**
     * Wrap an array, or create an empty one that the object destroys.
     *
     * @param array The array, or null to create one.
     * @ghidraAddress 0x10001a2a
     */
    HxLongSafeArray(SAFEARRAY *array);

    /**
     * Create an array that the object destroys, with the elements of a vector.
     *
     * @param values The elements.
     * @ghidraAddress 0x10001a79
     */
    HxLongSafeArray(std::vector<int> &values);

    /**
     * Destroy the array when the object created it.
     *
     * @ghidraAddress 0x10001ab1
     */
    ~HxLongSafeArray();

    /**
     * Read an element. An index past the end is a failure.
     *
     * @param iIndex The index.
     * @return The element.
     * @ghidraAddress 0x10001ad5
     */
    long Get(long iIndex);

    /**
     * Write an element. An index past the end is a failure.
     *
     * @param iIndex The index.
     * @param value The element.
     * @ghidraAddress 0x10001b3e
     */
    void Set(long iIndex, long value);

    /**
     * Refer a variant to the array, without copying it.
     *
     * @param variant Receives the array.
     * @ghidraAddress 0x10001ba2
     */
    void GetVariant(VARIANT *variant);

    /**
     * Replace the elements with those of a vector.
     *
     * @param values The elements.
     * @ghidraAddress 0x10001bcd
     */
    void SetFromVector(std::vector<int> &values);

private:
    bool mOwned; /*!< Whether the object created the array and destroys it. */
};
