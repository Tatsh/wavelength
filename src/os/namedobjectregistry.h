#pragma once

#include <memory>
#include <vector>

#include "os/string.h"

/**
 * Registry of named objects and of named groups of objects.
 *
 * The class is not polymorphic. The name is inferred. The registry is built on first use, and only
 * NamedObjectRegistryTerminate() uses it. No routine of the image adds an entry, and the class of
 * the objects is not yet identified.
 */
class NamedObjectRegistry {
public:
    /** An object of the registry, which the registry deletes through its virtual destructor. */
    class Object {
    public:
        /** Release the object. */
        virtual ~Object() = default;
    };

    /** A named object. */
    struct Entry {
        String mName;                    /*!< The name. */
        std::unique_ptr<Object> mObject; /*!< The object, or null. */
    };

    /** A named group of objects. */
    struct Group {
        String mName;                   /*!< The name. */
        int mReserved14[2];             // +0x14, not yet identified.
        std::vector<Object *> mMembers; /*!< The objects of the group. Borrowed. */
    };

    /**
     * Construct an empty registry.
     *
     * @ghidraAddress NTSC-U/C: 0x0029e778
     * @ghidraAddress PAL: 0x002a8440
     */
    NamedObjectRegistry();

    /**
     * Release the groups and the list of entries, without deleting the objects.
     *
     * @ghidraAddress NTSC-U/C: 0x0029e7a0
     * @ghidraAddress PAL: 0x002a8468
     */
    ~NamedObjectRegistry();

    /**
     * Report the registry, constructing it on first use.
     *
     * @return The registry.
     * @ghidraAddress NTSC-U/C: 0x0029e6f0
     * @ghidraAddress PAL: 0x002a83b8
     */
    static NamedObjectRegistry *Get();

    /**
     * Delete every entry and its object.
     *
     * @ghidraAddress NTSC-U/C: 0x0029e890
     * @ghidraAddress PAL: 0x002a8558
     */
    void DeleteEntries();

    /** @return The named objects. */
    const std::vector<std::unique_ptr<Entry>> &GetEntries() const {
        return mEntries;
    }

    /** @return The named groups. */
    const std::vector<Group> &GetGroups() const {
        return mGroups;
    }

private:
    std::vector<std::unique_ptr<Entry>> mEntries; /*!< The named objects. */
    std::vector<Group> mGroups;                   /*!< The named groups. */
};

/**
 * Prepare the registry of named objects.
 *
 * The body does nothing. The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029e748
 * @ghidraAddress PAL: 0x002a8410
 */
void NamedObjectRegistryInit();

/**
 * Delete every entry of the registry of named objects.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029e750
 * @ghidraAddress PAL: 0x002a8418
 */
void NamedObjectRegistryTerminate();
