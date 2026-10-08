#pragma once

#include <windows.h>

#include <oleauto.h>

/**
 * One-dimensional OLE Automation array that a script host passes to or receives from the control.
 *
 * The object is 8 bytes. The class is not polymorphic, so its name and the names of its members
 * are inferred from the source file and the assertion text.
 */
class HxSafeArray {
public:
    /**
     * Wrap an array and count its elements.
     *
     * Failing to read a bound is a failure.
     *
     * @param array The array. It is not copied.
     * @ghidraAddress 0x10001940
     */
    HxSafeArray(SAFEARRAY *array);

    /**
     * Report the number of elements.
     *
     * @return The number of elements.
     * @ghidraAddress 0x100019e5
     */
    int NumItems();

    /**
     * Change the number of elements, with a lower bound of zero.
     *
     * @param numItems The new number of elements.
     * @ghidraAddress 0x100019f5
     */
    void Resize(int numItems);

protected:
    int mNumItems;     /*!< The number of elements. */
    SAFEARRAY *mArray; /*!< The array. */
};
