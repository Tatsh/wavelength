#pragma once

#include <cstddef>
#include <list>
#include <vector>

#include "os/binstream.h"
#include "os/filepath.h"
#include "os/mem.h"
#include "os/string.h"
#include "rnd/rndobject.h"

/**
 * Loader of one `.rnd` file, which reads the file over several frames.
 *
 * The RTTI includes the class name. RndManager::AddLoader() creates the loader and queues it.
 * The load is a sequence of stages, each a member that does as much as a deadline allows and then
 * hands over to the next stage. A stage that finishes runs the next one at once.
 */
class RndLoader {
public:
    /** Behaviour bits of a load, as RndManager::AddLoader() takes them. */
    enum Flags {
        kAsync = 1,          /*!< Load across frames instead of before AddLoader() returns. */
        kReportExisting = 2, /*!< Report the objects of the file that exist and are not loaded. */
        kDeleteObjects = 4,  /*!< Delete the loaded objects when the loader is deleted. */
    };

    /**
     * Interface a loader consults before it creates each object of its file.
     *
     * The RTTI includes the nested class name. The class is abstract.
     */
    class Callback {
    public:
        /**
         * Receive the number of objects of the file before the first is created.
         *
         * The base does nothing. The name is inferred.
         *
         * @param nObjects The number of objects.
         */
        virtual void SetObjectCount([[maybe_unused]] int nObjects) {
        }

        /**
         * Decide whether the loader loads an object of its file.
         *
         * @param pExisting The loaded object with the same name, or null when none exists.
         * @param pszName The object name.
         * @param pszClass The class name the file records for the object.
         * @return A positive value to load the object, 0 to skip it, or a negative value to stop
         *         the load.
         */
        virtual int ShouldLoad(RndObject *pExisting, const char *pszName, const char *pszClass) = 0;

        /** Destroy the callback. */
        virtual ~Callback() {
        }
    };

    /**
     * One object of the file, with what the loader does with its record.
     *
     * The RTTI includes the nested class name.
     */
    class ObjectLoader {
    public:
        /**
         * Load the record of the object, or skip it.
         *
         * From version 2 every record ends with an end marker, and the stream is read up to the
         * marker. An earlier record that is skipped is loaded into a temporary object of the same
         * class.
         *
         * @param stream The stream to read from.
         * @param nVersion The version of the file.
         * @ghidraAddress NTSC-U/C: 0x0022c3e0
         */
        void Load(BinStream &stream, int nVersion);

        /**
         * Delete the object when the loader created it only to skip it.
         *
         * The name is inferred.
         *
         * @ghidraAddress NTSC-U/C: 0x0022c660
         * @ghidraAddress PAL: 0x00235328
         */
        void Release();

        RndObject *mObject; /*!< The object. */
        int mLoad;          /*!< Non-zero to load the record into mObject. */
        int mDelete;        /*!< Non-zero to delete mObject once the load ends. */
    };

    /** A stage of the load, which works until a deadline on the system clock in milliseconds. */
    typedef void (RndLoader::*State)(float fDeadlineMs);

    /**
     * Create a loader for a file.
     *
     * A synchronous load runs to the end before the constructor returns.
     *
     * @param pszFile The file.
     * @param nFlags The Flags of the load.
     * @param pCallback The callback consulted for each object, or null.
     * @param pStream The stream to read instead of the file, or null.
     * @ghidraAddress NTSC-U/C: 0x0022c6a0
     * @ghidraAddress PAL: 0x00235368
     */
    RndLoader(const char *pszFile, int nFlags, Callback *pCallback, BinStream *pStream);

    /**
     * End the load, delete the loaded objects when kDeleteObjects is set, and leave the queue.
     *
     * @ghidraAddress NTSC-U/C: 0x0022da38
     * @ghidraAddress PAL: 0x002366f0
     */
    ~RndLoader();

    /**
     * Allocate a loader, billed to `Loader`.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Loader", 0);
    }

    /**
     * Release a loader.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Report whether the load has ended.
     *
     * @return Whether the stage is DoneLoading().
     * @ghidraAddress NTSC-U/C: 0x0022c908
     * @ghidraAddress PAL: 0x002355d0
     */
    bool IsLoaded() const;

    /**
     * Advance the load until a deadline.
     *
     * @param fDeadlineMs The system clock time the step must finish by, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0022c958
     * @ghidraAddress PAL: 0x00235620
     */
    void Poll(float fDeadlineMs);

    /**
     * Open a stream on the file unless one was given, then start PreloadHeader().
     *
     * A file that fails to open ends the load.
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022c988
     * @ghidraAddress PAL: 0x00235650
     */
    void LoadFile(float fDeadlineMs);

    /**
     * Rewrite a class name of an older version to the name the registry knows.
     *
     * Each rewrite applies below the version that introduced it. The name is inferred.
     *
     * @param pszClass The class name to rewrite in place.
     * @ghidraAddress NTSC-U/C: 0x0022ca60
     * @ghidraAddress PAL: 0x00235728
     */
    void RemapClassName(char *pszClass);

    /**
     * Wait for the stream, read the version and the object count, then start CreateObjects().
     *
     * A version newer than RndManager::sRev ends the load, as does an asynchronous load of a
     * version before 7.
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022cce8
     * @ghidraAddress PAL: 0x002359b0
     */
    void PreloadHeader(float fDeadlineMs);

    /**
     * Read the object table, find or create each object, and record what to do with its record.
     *
     * A callback that returns a negative value stops the stage without ending the load. An
     * existing object that is internal or of another class ends the load. The texture count is
     * read next, and the stage continues with LoadResources() from version 9, otherwise with
     * LoadObjects().
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022ce90
     * @ghidraAddress PAL: 0x00235b58
     */
    void CreateObjects(float fDeadlineMs);

    /**
     * Read the texture files of the file and start reading them, then start LoadObjects().
     *
     * An asynchronous load keeps up to mTextures.size() reads in flight and finishes once every
     * read is done. From version 10 a texture of an object that is not loaded is not read. A
     * synchronous load skips the list.
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022d3a0
     * @ghidraAddress PAL: 0x00236058
     */
    void LoadResources(float fDeadlineMs);

    /**
     * Load or skip the record of each object while the stream has data, then start
     * DeleteObjects().
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022d6d8
     * @ghidraAddress PAL: 0x00236390
     */
    void LoadObjects(float fDeadlineMs);

    /**
     * Delete the objects created only to be skipped, then end the load.
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022d850
     * @ghidraAddress PAL: 0x00236508
     */
    void DeleteObjects(float fDeadlineMs);

    /**
     * The final stage, which does nothing.
     *
     * @param fDeadlineMs The deadline.
     * @ghidraAddress NTSC-U/C: 0x0022d960
     * @ghidraAddress PAL: 0x00236618
     */
    void DoneLoading(float fDeadlineMs);

    /**
     * End the load unless it has ended, releasing the stream it opened.
     *
     * The resources the renderer read are released unless the stage is DeleteObjects(). The
     * name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022d968
     * @ghidraAddress PAL: 0x00236620
     */
    void Finish();

    State mState;                    /*!< The stage the next Poll() runs. */
    int mOwnsStream;                 /*!< Non-zero when LoadFile() opened mStream. */
    String mFile;                    /*!< The file. */
    int mFlags;                      /*!< The Flags of the load. */
    String mDir;                     /*!< The directory the paths of the file are relative to. */
    BinStream *mStream;              /*!< The stream being read. */
    std::list<RndObject *> mObjects; /*!< The objects the file created and loaded. */
    int mVersion;                    /*!< The version of the file. */
    int mRemaining;                  /*!< The entries of the current table not read. */
    std::vector<FilePath> mTextures; /*!< The texture reads in flight. */
    std::list<ObjectLoader> mObjectLoaders;       /*!< The objects of the file, in file order. */
    std::list<ObjectLoader>::iterator mCurLoader; /*!< The object the current stage is at. */
    Callback *mCallback;                          /*!< The callback, or null. */
};
