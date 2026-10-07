#pragma once

#include <list>

#include "rnd/object.h"

/**
 * Loader of one `.rnd` file, which reads the file over several frames.
 *
 * The object is 0x6c bytes. Rnd::Manager::AddLoader() creates it and queues it. Only the members
 * the front end reads are declared.
 */
class RndLoader {
public:
    /** Behaviour bits of a load, as Rnd::Manager::AddLoader() takes them. */
    enum Flags {
        kAsync = 1,    /*!< Load across frames instead of before AddLoader() returns. */
        kPostLoad = 2, /*!< Run the pass that follows the object bodies once they have loaded. */
        kDeleteObjects = 4, /*!< Delete the loaded objects when the loader is deleted. */
    };

    /**
     * Interface a loader consults before it creates each object of its file.
     *
     * The RTTI includes the nested class name. The class is abstract.
     */
    class Callback {
    public:
        /**
         * Decide whether the loader creates an object of its file.
         *
         * @param pExisting The loaded object with the same name, or null when none exists.
         * @param pszName The object name.
         * @param pszClass The class name the file records for the object.
         * @return A positive value to create the object, 0 to skip it, or a negative value to stop
         *         the load.
         */
        virtual int
        ShouldLoad(Rnd::Object *pExisting, const char *pszName, const char *pszClass) = 0;

        /** Destroy the callback. */
        virtual ~Callback() {
        }
    };

    /**
     * Destroy the loader and release what it opened.
     *
     * @ghidraAddress NTSC-U/C: 0x0022da38
     * @ghidraAddress PAL: 0x002366f0
     */
    ~RndLoader();

    /**
     * Report whether the loader has created every object of its file.
     *
     * @return Whether the load has finished.
     * @ghidraAddress NTSC-U/C: 0x0022c908
     * @ghidraAddress PAL: 0x002355d0
     */
    bool IsLoaded() const;

    // +0x00 to +0x3b are not yet identified.
    std::list<Rnd::Object *> mObjects; /*!< The objects the file created. +0x3c */
};
