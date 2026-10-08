#pragma once

#include <vector>

#include "synthedit/HxSafeArray.h"
#include "utl/Str.h"

/**
 * One-dimensional array of strings (`VT_BSTR`), converted to and from multibyte text.
 *
 * The object is 12 bytes. The name is inferred.
 */
class HxStringSafeArray : public HxSafeArray {
public:
    /**
     * Wrap an array, or create an empty one that the object destroys.
     *
     * @param array The array, or null to create one.
     * @ghidraAddress 0x10001c2a
     */
    HxStringSafeArray(SAFEARRAY *array);

    /**
     * Create an array that the object destroys, with the elements of a vector.
     *
     * @param values The elements.
     * @ghidraAddress 0x10001c79
     */
    HxStringSafeArray(std::vector<String> &values);

    /**
     * Destroy the array when the object created it.
     *
     * @ghidraAddress 0x10001cb1
     */
    ~HxStringSafeArray();

    /**
     * Read an element, at most 499 characters of it. An index past the end, or text that does not
     * convert, is a failure.
     *
     * @param iIndex The index.
     * @return The element.
     * @ghidraAddress 0x10001cd5
     */
    String Get(long iIndex);

    /**
     * Write an element. An index past the end is a failure.
     *
     * @param iIndex The index.
     * @param value The element.
     * @ghidraAddress 0x10001e0e
     */
    void Set(long iIndex, const char *value);

    /**
     * Refer a variant to the array, without copying it.
     *
     * @param variant Receives the array.
     * @ghidraAddress 0x10001ed3
     */
    void GetVariant(VARIANT *variant);

    /**
     * Replace the elements with those of a vector.
     *
     * @param values The elements.
     * @ghidraAddress 0x10001efe
     */
    void SetFromVector(std::vector<String> &values);

private:
    bool mOwned; /*!< Whether the object created the array and destroys it. */
};
