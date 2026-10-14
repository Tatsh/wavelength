#pragma once

#include "app/attachment.h"

/**
 * Owner of one reference to an Attachment.
 *
 * The RTTI includes the template name. The object is the one pointer, which may be null. Each
 * instantiation has its own out-of-line copies of the members, and each address listed is one
 * instantiation's. Get() and operator->() compile to the same instructions, and each copy is listed
 * under the one its callers use.
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
     * @ghidraAddress NTSC-U/C: 0x003384b0
     * @ghidraAddress PAL: 0x003a5a60
     * @ghidraAddress NTSC-U/C: 0x0034a8d0
     * @ghidraAddress PAL: 0x003b7d00
     * @ghidraAddress NTSC-U/C: 0x0034d5a8
     * @ghidraAddress PAL: 0x003ba9d8
     */
    Ptr() : mObject(nullptr) {
    }

    /**
     * Take a reference to an object.
     *
     * @param pObject The object, or null.
     * @ghidraAddress NTSC-U/C: 0x00334260
     * @ghidraAddress PAL: 0x003a1810
     * @ghidraAddress NTSC-U/C: 0x00333d88
     * @ghidraAddress PAL: 0x003a1338
     * @ghidraAddress NTSC-U/C: 0x00338510
     * @ghidraAddress PAL: 0x003a5ac0
     * @ghidraAddress NTSC-U/C: 0x00335ae0
     * @ghidraAddress PAL: 0x003a3090
     * @ghidraAddress NTSC-U/C: 0x00340b98
     * @ghidraAddress PAL: 0x003ae0d0
     * @ghidraAddress NTSC-U/C: 0x003439f0
     * @ghidraAddress PAL: 0x003b0f28
     * @ghidraAddress NTSC-U/C: 0x0034ce30
     * @ghidraAddress PAL: 0x003ba260
     * @ghidraAddress NTSC-U/C: 0x0034d2a8
     * @ghidraAddress PAL: 0x003ba6d8
     * @ghidraAddress NTSC-U/C: 0x0034d520
     * @ghidraAddress PAL: 0x003ba950
     * @ghidraAddress NTSC-U/C: 0x0034e940
     * @ghidraAddress PAL: 0x003bbd68
     * @ghidraAddress NTSC-U/C: 0x0034ef48
     * @ghidraAddress PAL: 0x003bc370
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
     * @ghidraAddress NTSC-U/C: 0x00333e10
     * @ghidraAddress PAL: 0x003a13c0
     * @ghidraAddress NTSC-U/C: 0x00343a28
     * @ghidraAddress PAL: 0x003b0f60
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
     * @ghidraAddress NTSC-U/C: 0x00333dc0
     * @ghidraAddress PAL: 0x003a1370
     * @ghidraAddress NTSC-U/C: 0x003384c0
     * @ghidraAddress PAL: 0x003a5a70
     * @ghidraAddress NTSC-U/C: 0x00335b18
     * @ghidraAddress PAL: 0x003a30c8
     * @ghidraAddress NTSC-U/C: 0x00340bd0
     * @ghidraAddress PAL: 0x003ae108
     * @ghidraAddress NTSC-U/C: 0x003439a0
     * @ghidraAddress PAL: 0x003b0ed8
     * @ghidraAddress NTSC-U/C: 0x0034ce68
     * @ghidraAddress PAL: 0x003ba298
     * @ghidraAddress NTSC-U/C: 0x0034d2e0
     * @ghidraAddress PAL: 0x003ba710
     * @ghidraAddress NTSC-U/C: 0x0034d558
     * @ghidraAddress PAL: 0x003ba988
     * @ghidraAddress NTSC-U/C: 0x0034e978
     * @ghidraAddress PAL: 0x003bbda0
     * @ghidraAddress NTSC-U/C: 0x0034ef80
     * @ghidraAddress PAL: 0x003bc3a8
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
     * @ghidraAddress NTSC-U/C: 0x00338548
     * @ghidraAddress PAL: 0x003a5af8
     * @ghidraAddress NTSC-U/C: 0x0033b618
     * @ghidraAddress PAL: 0x003a8b50
     * @ghidraAddress NTSC-U/C: 0x0034d5c8
     * @ghidraAddress PAL: 0x003ba9f8
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
     * @ghidraAddress NTSC-U/C: 0x003a67c0
     * @ghidraAddress PAL: 0x004154a0
     * @ghidraAddress NTSC-U/C: 0x00333ea8
     * @ghidraAddress PAL: 0x003a1458
     * @ghidraAddress NTSC-U/C: 0x003385a8
     * @ghidraAddress PAL: 0x003a5b58
     * @ghidraAddress NTSC-U/C: 0x003385b0
     * @ghidraAddress PAL: 0x003a5b60
     * @ghidraAddress NTSC-U/C: 0x00335cb0
     * @ghidraAddress PAL: 0x003a3260
     * @ghidraAddress NTSC-U/C: 0x0033a6c8
     * @ghidraAddress PAL: 0x003a7c00
     * @ghidraAddress NTSC-U/C: 0x00340c98
     * @ghidraAddress PAL: 0x003ae1d0
     * @ghidraAddress NTSC-U/C: 0x0034ceb8
     * @ghidraAddress PAL: 0x003ba2e8
     * @ghidraAddress NTSC-U/C: 0x0034cec0
     * @ghidraAddress PAL: 0x003ba2f0
     * @ghidraAddress NTSC-U/C: 0x0034d330
     * @ghidraAddress PAL: 0x003ba760
     * @ghidraAddress NTSC-U/C: 0x0034d338
     * @ghidraAddress PAL: 0x003ba768
     * @ghidraAddress NTSC-U/C: 0x0034d5b8
     * @ghidraAddress PAL: 0x003ba9e8
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
     * @ghidraAddress NTSC-U/C: 0x003a6a90
     * @ghidraAddress PAL: 0x00415770
     * @ghidraAddress NTSC-U/C: 0x00335ca8
     * @ghidraAddress PAL: 0x003a3258
     * @ghidraAddress NTSC-U/C: 0x00343a60
     * @ghidraAddress PAL: 0x003b0f98
     * @ghidraAddress NTSC-U/C: 0x0034d5c0
     * @ghidraAddress PAL: 0x003ba9f0
     * @ghidraAddress NTSC-U/C: 0x0034e290
     * @ghidraAddress PAL: 0x003bb6b8
     * @ghidraAddress NTSC-U/C: 0x0034e9c8
     * @ghidraAddress PAL: 0x003bbdf0
     * @ghidraAddress NTSC-U/C: 0x0034e9d0
     * @ghidraAddress PAL: 0x003bbdf8
     * @ghidraAddress NTSC-U/C: 0x0034efd0
     * @ghidraAddress PAL: 0x003bc3f8
     * @ghidraAddress NTSC-U/C: 0x0034efd8
     * @ghidraAddress PAL: 0x003bc400
     */
    T *operator->() const {
        return mObject;
    }

private:
    T *mObject; /*!< The referenced object, or null. */
};
