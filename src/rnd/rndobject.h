#pragma once

#include <list>

#include "os/binstream.h"
#include "os/prnstream.h"
#include "os/string.h"

/**
 * Base of every object the renderer can load, resolve by name, and serialise.
 *
 * The RTTI includes the class name and records no base. Every renderer mix-in derives from the
 * class virtually.
 *
 * Construction registers the object in TheManager under its name, and destruction erases that
 * registration. A second object that stores a pointer to this one registers itself through
 * AddRef(). The destructor then notifies every referrer through Replace() before the object goes
 * away.
 */
class RndObject {
public:
    /**
     * Construct an object and register it under a name.
     *
     * An object already registered under the name loses its registration to this one.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0023bfa0
     * @ghidraAddress PAL: 0x00244b20
     */
    explicit RndObject(const char *pszName);

    /**
     * Notify every referrer, then erase the registration.
     *
     * Adjacent duplicate referrers are collapsed first. Each referrer then has Replace() called
     * with this object and null.
     *
     * @ghidraAddress NTSC-U/C: 0x0023c0f8
     * @ghidraAddress PAL: 0x00244c78
     */
    virtual ~RndObject();

    /**
     * Write a description of the object.
     *
     * The referrers are included only when the dump level of the stream is above 0.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023c648
     * @ghidraAddress PAL: 0x002451c8
     */
    virtual void DumpText(PrnStream &stream);

    /**
     * Write the object's serialised form.
     *
     * @param stream The stream to write to.
     */
    virtual void Save(BinStream &stream) = 0;

    /**
     * Replace every stored pointer to one object with another.
     *
     * A null replacement means the object is going away and the pointer is to be dropped.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     */
    virtual void Replace(RndObject *pFrom, RndObject *pTo) = 0;

    /**
     * Report the name of the object's most derived class.
     *
     * @return The type name a `.rnd` file writes for the class.
     */
    virtual const char *ClassName() const = 0;

    /**
     * Copy the state of another object into this one.
     *
     * @param pSource The object to copy from.
     * @param nFlags The set of fields to copy.
     */
    virtual void Copy(const RndObject *pSource, int nFlags) = 0;

    /**
     * Read the object's serialised form.
     *
     * @param stream The stream to read from.
     */
    virtual void Load(BinStream &stream) = 0;

    /**
     * Register the object under a different name.
     *
     * A name equal to the current one changes nothing. Otherwise every registration under the
     * current name is erased first.
     *
     * @param pszName The new registry key.
     * @ghidraAddress NTSC-U/C: 0x0023c380
     * @ghidraAddress PAL: 0x00244f00
     */
    void SetName(const char *pszName);

    /**
     * Register an object as a store of a pointer to this one.
     *
     * The object itself is not registered. The same referrer may register more than once.
     *
     * @param pReferrer The object that stores a pointer to this one.
     * @ghidraAddress NTSC-U/C: 0x0023c750
     * @ghidraAddress PAL: 0x002452d0
     */
    void AddRef(RndObject *pReferrer);

    /**
     * Drop the first registration AddRef() made for a referrer.
     *
     * The destructor's walk of the referrers is not disturbed. While it runs, the call does
     * nothing.
     *
     * @param pReferrer The object whose registration is to be dropped.
     * @ghidraAddress NTSC-U/C: 0x0023c7e8
     * @ghidraAddress PAL: 0x00245368
     */
    void RemoveRef(RndObject *pReferrer);

    /** The bit of the Copy() flags that copies the child lists of the mix-ins. */
    static constexpr int kCopyChildLists = 0x200;

    std::list<RndObject *> mRefs; /*!< Objects that store a pointer to this one. */
    String mName;                 /*!< Registry key. */
    int mInternal;                /*!< Non-zero for an object the renderer made, not a file. */
    int mDeleting;                /*!< Set while the destructor notifies the referrers. */
};

/**
 * Write an object's name, or "no object" for null.
 *
 * @param stream The stream to write to.
 * @param pObject The object, or null.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0023c870
 * @ghidraAddress PAL: 0x002453f0
 */
PrnStream &operator<<(PrnStream &stream, const RndObject *pObject);
