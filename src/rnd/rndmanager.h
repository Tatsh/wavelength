#pragma once

#include <list>
#include <map>

#include "os/binstream.h"
#include "os/file.h"
#include "os/filepath.h"
#include "os/strless.h"
#include "rnd/rndloader.h"
#include "rnd/rndobject.h"

/**
 * Registry of every live renderer object, of every class a `.rnd` file may instantiate, and of the
 * texture files being read.
 *
 * The class has no virtual and no RTTI. Its name is the tag it bills its loaders' storage to, and
 * the RTTI of its nested ResourceLoader records it as the enclosing class.
 */
class RndManager {
public:
    /**
     * Builds one instance of a registered class.
     *
     * @param pszName The object name for the new instance.
     * @return The new object.
     */
    typedef RndObject *(*ClassFactory)(const char *pszName);

    /** Results of IsTextureLoaded(). */
    enum TextureState {
        kTextureLoading = 0, /*!< The file is still being read. */
        kTextureLoaded = 1,  /*!< The file is in memory. */
        kTextureUnknown = 2, /*!< No reference on the file exists. */
    };

    /**
     * Read of one resource file into memory, shared by every reference to the file.
     *
     * The RTTI includes the nested class name. A compressed file is read to the end of the buffer
     * and inflated forward over it once the read completes.
     */
    class ResourceLoader {
    public:
        /**
         * Start reading a file.
         *
         * The loader has one reference.
         *
         * @param pFile The open file. The loader deletes it once the read completes.
         * @param pszTag The tag the buffer is billed to.
         * @ghidraAddress NTSC-U/C: 0x0023a140
         * @ghidraAddress PAL: 0x00242cc0
         */
        ResourceLoader(File *pFile, const char *pszTag);

        /**
         * Release the buffer and the file.
         *
         * @ghidraAddress NTSC-U/C: 0x0023a230
         * @ghidraAddress PAL: 0x00242db0
         */
        ~ResourceLoader();

        /**
         * Finish the read when the file reports it complete.
         *
         * A compressed file is inflated, and the file is deleted.
         *
         * @return Whether the data is in memory.
         * @ghidraAddress NTSC-U/C: 0x0023a2a8
         * @ghidraAddress PAL: 0x00242e28
         */
        bool Poll();

        /**
         * Wait for the data and hand it out, dropping one reference.
         *
         * The last reference receives the buffer itself. Every earlier one receives a copy.
         *
         * @param ppData Receives the data, which the caller frees.
         * @param pnSize Receives the size of the data in bytes.
         * @return The references left.
         * @ghidraAddress NTSC-U/C: 0x0023a358
         * @ghidraAddress PAL: 0x00242ed8
         */
        int GetData(void **ppData, int *pnSize);

        File *mFile;       /*!< The file being read, or null once the read completed. */
        const char *mTag;  /*!< The tag mBuffer is billed to. */
        char *mCompressed; /*!< Where a compressed file is read in mBuffer, or null. */
        char *mBuffer;     /*!< The data. */
        int mRefs;         /*!< References GetData() has not yet consumed. */
        int mSize;         /*!< The size of mBuffer in bytes. */
    };

    /**
     * Construct an empty registry.
     *
     * @ghidraAddress NTSC-U/C: 0x0038f578
     * @ghidraAddress PAL: 0x003fdc80
     */
    RndManager();

    /**
     * Destroy the registry.
     *
     * @ghidraAddress NTSC-U/C: 0x0038f748
     * @ghidraAddress PAL: 0x003fde50
     */
    ~RndManager();

    /**
     * Advance the queued loaders until a time budget is spent.
     *
     * The front loader is advanced, and it leaves the queue once it has loaded. The budget is
     * measured on the system clock.
     *
     * @param fBudgetMs The time the loads may take, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00237bc0
     * @ghidraAddress PAL: 0x00240740
     */
    void PollLoaders(float fBudgetMs);

    /**
     * Remove a loader from the queue.
     *
     * The name is inferred.
     *
     * @param pLoader The loader.
     * @ghidraAddress NTSC-U/C: 0x00237d60
     * @ghidraAddress PAL: 0x002408e0
     */
    void RemoveLoader(RndLoader *pLoader);

    /**
     * Resolve a live object by name.
     *
     * @param pszName The object name.
     * @return The object, or null when no object has the name.
     * @ghidraAddress NTSC-U/C: 0x00237e00
     * @ghidraAddress PAL: 0x00240980
     */
    RndObject *Find(const char *pszName);

    /**
     * Create a loader for a `.rnd` file and queue it.
     *
     * A synchronous load first finishes the loads already queued.
     *
     * @param pszFile The file.
     * @param nFlags The RndLoader::Flags of the load.
     * @param pCallback The callback the loader consults for each object, or null.
     * @param pStream The stream to read instead of the file, or null.
     * @return The loader.
     * @ghidraAddress NTSC-U/C: 0x00238e98
     * @ghidraAddress PAL: 0x00241a18
     */
    RndLoader *
    AddLoader(const char *pszFile, int nFlags, RndLoader::Callback *pCallback, BinStream *pStream);

    /**
     * Load a `.rnd` file before the call returns.
     *
     * The loader is deleted once it finishes, and the objects remain.
     *
     * @param pszFile The file.
     * @ghidraAddress NTSC-U/C: 0x00238fb0
     * @ghidraAddress PAL: 0x00241b30
     */
    void LoadFile(const char *pszFile);

    /**
     * Create an instance of a registered class.
     *
     * An unregistered class, or a name already in use, yields null.
     *
     * @param pszClass The class name a `.rnd` file writes.
     * @param pszName The object name for the new instance.
     * @return The new object, or null.
     * @ghidraAddress NTSC-U/C: 0x002397f8
     * @ghidraAddress PAL: 0x00242378
     */
    RndObject *Create(const char *pszClass, const char *pszName);

    /**
     * Report the tag a resource file's buffer is billed to, from the file's extension.
     *
     * The name is inferred.
     *
     * @param pszFile The file.
     * @return "Resource_bmp", "Resource_ipu", or "Resource_other".
     * @ghidraAddress NTSC-U/C: 0x00239900
     * @ghidraAddress PAL: 0x00242480
     */
    static const char *ResourceTag(const char *pszFile);

    /**
     * Report the file to read for a resource.
     *
     * A bitmap is converted to the renderer's texture format and cached. The name is inferred.
     *
     * @param pszFile The resource file.
     * @return The file to read, or null when the bitmap cannot be converted.
     * @ghidraAddress NTSC-U/C: 0x00239970
     * @ghidraAddress PAL: 0x002424f0
     * @stub
     */
    const char *ResourcePath(const char *pszFile);

    /**
     * Take a reference on a texture file, starting to read it on the first reference.
     *
     * An empty path, or a renderer without textures, is ignored. The name is inferred.
     *
     * @param path The file.
     * @return Whether the file is referenced, false when it could not be opened.
     * @ghidraAddress NTSC-U/C: 0x00239c90
     * @ghidraAddress PAL: 0x00242810
     */
    bool AcquireTexture(const FilePath &path);

    /**
     * Wait for a resource file and hand out its data, dropping one reference.
     *
     * A file without a reference is read first. The read is released with its last reference.
     * The name is inferred.
     *
     * @param path The file.
     * @param ppData Receives the data, or null when the file cannot be read.
     * @param pnSize Receives the size of the data in bytes.
     * @ghidraAddress NTSC-U/C: 0x00239e98
     * @ghidraAddress PAL: 0x00242a18
     */
    void GetResource(const FilePath &path, void **ppData, int *pnSize);

    /**
     * Report whether a texture file finished reading.
     *
     * The name is inferred.
     *
     * @param path The file.
     * @return A TextureState.
     * @ghidraAddress NTSC-U/C: 0x0023a080
     * @ghidraAddress PAL: 0x00242c00
     */
    int IsTextureLoaded(const FilePath &path);

    /**
     * Delete every object a file created, leaving the internal objects.
     *
     * The resource reads are released as well.
     *
     * @ghidraAddress NTSC-U/C: 0x0023a420
     * @ghidraAddress PAL: 0x00242fa0
     */
    void DeleteLoadedObjects();

    /**
     * Release the read of a texture file whatever its references.
     *
     * The name is inferred.
     *
     * @param path The file.
     * @ghidraAddress NTSC-U/C: 0x0023a4e0
     * @ghidraAddress PAL: 0x00243060
     */
    void ReleaseTexture(const FilePath &path);

    /**
     * Release every resource read.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023a5f0
     * @ghidraAddress PAL: 0x00243170
     */
    void ClearResources();

    std::map<const char *, RndObject *, StrLess> mObjects;  /*!< Live objects by name. */
    std::map<const char *, ClassFactory, StrLess> mClasses; /*!< Factories by class name. */
    std::map<FilePath, ResourceLoader *> mResources;        /*!< Resource reads by file. */
    std::list<RndLoader *> mLoaders;                        /*!< Queued loaders, oldest first. */
};

/**
 * The renderer's registry.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c798
 */
extern RndManager TheManager;

/**
 * Write a list of objects as its size followed by each object's name.
 *
 * A null entry writes an empty name.
 *
 * @param stream The stream to write to.
 * @param list The list.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00380540
 * @ghidraAddress PAL: 0x003eec58
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::list<T *> &list) {
    const int nSize = static_cast<int>(list.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (T *pElement : list) {
        const RndObject *pObject = pElement;
        stream.WriteString(pObject != nullptr ? pObject->mName.c_str() : "");
    }
    return stream;
}

/**
 * Read a list of objects the list writer wrote, resolving each name in TheManager.
 *
 * An empty name, an unknown name, or an object of another class reads as null.
 *
 * @param stream The stream to read from.
 * @param list Receives the objects.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00380780
 * @ghidraAddress PAL: 0x003eee98
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::list<T *> &list) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    list.resize(nSize, nullptr);
    for (T *&pElement : list) {
        String name;
        stream >> name;
        if (name.mLength == 0) {
            pElement = nullptr;
        } else {
            pElement = dynamic_cast<T *>(TheManager.Find(name.c_str()));
        }
    }
    return stream;
}
