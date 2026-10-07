#pragma once

#include "app/attachment.h"

/**
 * Owner of one reference to an Attachment.
 *
 * The RTTI includes the template name. The object is the one pointer, which may be null. Each
 * instantiation has its own out-of-line copies of the members, and the addresses listed are those
 * of the instantiations for Command and Muse.
 *
 * @tparam T The referenced class.
 */
template <typename T>
class Ptr {
public:
    /**
     * Construct a null owner.
     *
     * @ghidraAddress NTSC-U/C: 0x0033fe30
     * @ghidraAddress PAL: 0x003ad368
     */
    Ptr() : mObject(nullptr) {
    }

    /**
     * Take a reference to an object.
     *
     * @param pObject The object, or null.
     * @ghidraAddress NTSC-U/C: 0x00334260
     * @ghidraAddress PAL: 0x003a1810
     */
    explicit Ptr(T *pObject) : mObject(pObject) {
        if (pObject != nullptr) {
            pObject->AddRef();
        }
    }

    /**
     * Take another reference to the object of another owner.
     *
     * @param other The other owner.
     * @ghidraAddress NTSC-U/C: 0x00337438
     * @ghidraAddress PAL: 0x003a49e8
     */
    Ptr(const Ptr &other) : mObject(other.mObject) {
        if (mObject != nullptr) {
            mObject->AddRef();
        }
    }

    /**
     * Give back the reference.
     *
     * @ghidraAddress NTSC-U/C: 0x00334298
     * @ghidraAddress PAL: 0x003a1848
     */
    ~Ptr() {
        if (mObject != nullptr) {
            mObject->Release();
        }
    }

    /**
     * Take a reference to the object of another owner and give back the current one.
     *
     * @param other The other owner.
     * @return The owner.
     * @ghidraAddress NTSC-U/C: 0x00333e48
     * @ghidraAddress PAL: 0x003a13f8
     */
    Ptr &operator=(const Ptr &other) {
        if (this != &other) {
            if (mObject != nullptr) {
                mObject->Release();
            }
            mObject = other.mObject;
            if (mObject != nullptr) {
                mObject->AddRef();
            }
        }
        return *this;
    }

    /**
     * Report the object.
     *
     * @return The object, or null.
     * @ghidraAddress NTSC-U/C: 0x00334608
     * @ghidraAddress PAL: 0x003a1bb8
     */
    T *Get() const {
        return mObject;
    }

    /**
     * Report the object for a member access.
     *
     * @return The object.
     * @ghidraAddress NTSC-U/C: 0x0033fe40
     * @ghidraAddress PAL: 0x003ad378
     */
    T *operator->() const {
        return mObject;
    }

private:
    T *mObject; /*!< The referenced object, or null. */
};
