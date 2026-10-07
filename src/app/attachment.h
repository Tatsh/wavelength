#pragma once

/**
 * Reference-counted root of the engine's polymorphic object graph.
 *
 * The RTTI includes the class name and records no base. The one data word is declared first. The
 * vptr therefore sits after it at `+0x04`, and the class is eight bytes. The vtable at `0x003d6dd8`
 * runs the type function and the destructor.
 *
 * A fresh object starts with no reference. Each owner takes one through AddRef(), Ptr does so for
 * the object it holds, and the last Release() deletes the object through the virtual destructor.
 *
 * Some callers increment mRefs directly rather than through AddRef(). Access from an unrelated
 * class is why the field is public rather than protected.
 */
class Attachment {
public:
    /**
     * Construct an object with no reference.
     *
     * Inline. Every derived constructor expands it.
     */
    Attachment() : mRefs(0) {
    }

    /**
     * Release the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00293e80
     * @ghidraAddress PAL: 0x0029d838
     */
    virtual ~Attachment();

    /**
     * Take one reference.
     *
     * @ghidraAddress NTSC-U/C: 0x00293eb0
     * @ghidraAddress PAL: 0x0029d868
     */
    void AddRef();

    /**
     * Give back one reference and delete the object once the last one is gone.
     *
     * @ghidraAddress NTSC-U/C: 0x00293ec0
     * @ghidraAddress PAL: 0x0029d878
     */
    void Release();

    /**
     * Give back one reference to an object that may be null.
     *
     * Passed as a function to std::for_each by the command scheduler, the phrase database, and the
     * playback reader. The body is inline. The title is inferred.
     *
     * @param pAttachment The object, or null.
     */
    static void ReleaseIfSet(Attachment *pAttachment) {
        if (pAttachment != nullptr) {
            pAttachment->Release();
        }
    }

    int mRefs; /*!< The number of references. */
};
