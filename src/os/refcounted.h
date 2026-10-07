#pragma once

/**
 * Base of an object whose lifetime a reference count manages.
 *
 * The count is the first word and the vptr follows it. Ptr is the usual owner. The name is
 * inferred.
 */
class RefCounted {
public:
    /** Release the object. */
    virtual ~RefCounted() {
    }

    /**
     * Add one reference.
     *
     * @ghidraAddress NTSC-U/C: 0x00293eb0
     * @ghidraAddress PAL: 0x0029d868
     */
    void AddRef();

    /**
     * Drop one reference, and destroy the object when no reference remains.
     *
     * @ghidraAddress NTSC-U/C: 0x00293ec0
     * @ghidraAddress PAL: 0x0029d878
     */
    void Release();

private:
    int mRefs = 0; /*!< The number of references. */
};
