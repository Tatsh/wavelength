#pragma once

#include <list>
#include <map>
#include <memory>
#include <vector>

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
         * @param pFile The open file. The loader closes it once the read completes.
         * @param pszTag The tag the buffer is billed to.
         * @ghidraAddress NTSC-U/C: 0x0023a140
         * @ghidraAddress PAL: 0x00242cc0
         */
        ResourceLoader(std::unique_ptr<File> pFile, const char *pszTag);

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

        /** Take one more reference, for one more GetData(). */
        void AddRef() {
            ++mRefs;
        }

    private:
        std::unique_ptr<File> mFile; /*!< The file being read, or null once the read completed. */
        const char *mTag;            /*!< The tag mBuffer is billed to. */
        char *mCompressed;           /*!< Where a compressed file is read in mBuffer, or null. */
        char *mBuffer;               /*!< The data. */
        int mRefs;                   /*!< References GetData() has not yet consumed. */
        int mSize;                   /*!< The size of mBuffer in bytes. */
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
     * Register the factory of every class a `.rnd` file may instantiate.
     *
     * The class `Tunnel` is registered with NewTunnel(). The current camera is cleared.
     *
     * @ghidraAddress NTSC-U/C: 0x00236800
     * @ghidraAddress PAL: 0x0023f380
     */
    void Init();

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
     * @return The loader, which the caller owns. It leaves the queue when destroyed.
     * @ghidraAddress NTSC-U/C: 0x00238e98
     * @ghidraAddress PAL: 0x00241a18
     */
    std::unique_ptr<RndLoader>
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
     * Write the objects to a `.rnd` file.
     *
     * A file that does not open reports "Could not open file: %s" and nothing is written.
     * Otherwise the directory of the file becomes the root that relative paths resolve against.
     *
     * @param pszFile The file.
     * @ghidraAddress NTSC-U/C: 0x00238fe8
     * @ghidraAddress PAL: 0x00241b68
     */
    void SaveFile(const char *pszFile);

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
     * Clone an object under a prefixed name, optionally with its descendants and its parents.
     *
     * The descendants are cloned without their descendants, and every clone then has each
     * reference to an original replaced with the clone of that original. The pairing assumes that
     * pClones started empty. A linked clone is attached to each parent of the source.
     *
     * @param pSource The object to clone.
     * @param pszPrefix The prefix for the name of every clone.
     * @param pClones Receives the clone first, followed by the clones of the descendants.
     * @param nFlags The copy flags.
     * @param bRecurse Non-zero to clone the descendants.
     * @param bLink Non-zero to attach the clone to the parents of the source.
     * @ghidraAddress NTSC-U/C: 0x002382b0
     * @ghidraAddress PAL: 0x00240e30
     */
    void Clone(RndObject *pSource,
               const char *pszPrefix,
               std::list<RndObject *> *pClones,
               int nFlags,
               int bRecurse,
               int bLink);

    /**
     * Order a list of objects for writing, the textures first and sorted by file, followed by the
     * animations, the materials, the fonts, the texts, the meshes, the blurs, and the rest.
     *
     * The name is inferred.
     *
     * @param objects The list to order.
     * @ghidraAddress NTSC-U/C: 0x00238958
     * @ghidraAddress PAL: 0x002414d8
     */
    void SortObjects(std::list<RndObject *> &objects);

    /**
     * Write every object that is not internal as a `.rnd` file.
     *
     * The file is the version, the class and the name of each object, the files of the textures
     * and the movies, and the record of each object, each record followed by an end marker.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00239088
     * @ghidraAddress PAL: 0x00241c08
     */
    void Write(BinStream &stream);

    /**
     * Create a view in place of a tunnel, the factory registered for the class `Tunnel`.
     *
     * A notice reports the substitution. The name is inferred.
     *
     * @param pszName The object name.
     * @return The view, or null when the name is taken.
     * @ghidraAddress NTSC-U/C: 0x002367b8
     * @ghidraAddress PAL: 0x0023f338
     */
    static RndObject *NewTunnel(const char *pszName);

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

    /** @return The live objects by name. */
    const std::map<const char *, RndObject *, StrLess> &GetObjects() const {
        return mObjects;
    }

    /**
     * Register an object under a name, replacing any object registered under it.
     *
     * @param pszName The name, which must outlive the registration.
     * @param pObject The object.
     */
    void SetObject(const char *pszName, RndObject *pObject) {
        mObjects[pszName] = pObject;
    }

    /**
     * Remove the registration of a name.
     *
     * @param pszName The name.
     */
    void RemoveObject(const char *pszName) {
        mObjects.erase(pszName);
    }

    /**
     * Register the factory that Create() calls for a class.
     *
     * @param pszClass The class name a `.rnd` file writes.
     * @param factory The factory.
     */
    void SetClassFactory(const char *pszClass, ClassFactory factory) {
        mClasses[pszClass] = factory;
    }

    /**
     * The format version Write() writes and the newest a loader accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a00
     * @ghidraAddress PAL: 0x0042f898
     */
    static int sRev;

private:
    std::map<const char *, RndObject *, StrLess> mObjects;          /*!< Live objects by name. */
    std::map<const char *, ClassFactory, StrLess> mClasses;         /*!< Factories by class name. */
    std::map<FilePath, std::unique_ptr<ResourceLoader>> mResources; /*!< Resource reads by file. */
    std::list<RndLoader *> mLoaders; /*!< Queued loaders, oldest first. The callers own them. */
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
 * A null entry writes an empty name. Each element type has its own copy, one address each.
 *
 * @param stream The stream to write to.
 * @param list The list.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00380540
 * @ghidraAddress PAL: 0x003eec58
 * @ghidraAddress NTSC-U/C: 0x0037de10
 * @ghidraAddress PAL: 0x003ec540
 * @ghidraAddress NTSC-U/C: 0x0037f5c0
 * @ghidraAddress PAL: 0x003edcd8
 * @ghidraAddress NTSC-U/C: 0x00380c28
 * @ghidraAddress PAL: 0x003ef340
 * @ghidraAddress NTSC-U/C: 0x00391a68
 * @ghidraAddress PAL: 0x00400170
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::list<T *> &list) {
    const int nSize = static_cast<int>(list.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (T *pElement : list) {
        const RndObject *pObject = pElement;
        stream.WriteString(pObject != nullptr ? pObject->GetName() : "");
    }
    return stream;
}

/**
 * Read a list of objects the list writer wrote, resolving each name in TheManager.
 *
 * An empty name, an unknown name, or an object of another class reads as null. Each element
 * type has its own copy, one address each.
 *
 * @param stream The stream to read from.
 * @param list Receives the objects.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00380780
 * @ghidraAddress PAL: 0x003eee98
 * @ghidraAddress NTSC-U/C: 0x0037e250
 * @ghidraAddress PAL: 0x003ec980
 * @ghidraAddress NTSC-U/C: 0x0037f800
 * @ghidraAddress PAL: 0x003edf18
 * @ghidraAddress NTSC-U/C: 0x00380e68
 * @ghidraAddress PAL: 0x003ef580
 * @ghidraAddress NTSC-U/C: 0x00387908
 * @ghidraAddress PAL: 0x003f6010
 * @ghidraAddress NTSC-U/C: 0x00391ca8
 * @ghidraAddress PAL: 0x004003b0
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::list<T *> &list) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    list.resize(nSize, nullptr);
    for (T *&pElement : list) {
        String name;
        stream >> name;
        if (name.GetLength() == 0) {
            pElement = nullptr;
        } else {
            pElement = dynamic_cast<T *>(TheManager.Find(name.c_str()));
        }
    }
    return stream;
}

/**
 * Write a vector of objects as its size followed by each object's name.
 *
 * A null entry writes an empty name. Each element type has its own instance, and the addresses
 * are those of the RndMat and the RndFont instances.
 *
 * @param stream The stream to write to.
 * @param objects The vector.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003951a0
 * @ghidraAddress PAL: 0x00403890
 * @ghidraAddress NTSC-U/C: 0x00395250
 * @ghidraAddress PAL: 0x00403940
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::vector<T *> &objects) {
    const int nSize = static_cast<int>(objects.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (T *pElement : objects) {
        const RndObject *pObject = pElement;
        stream.WriteString(pObject != nullptr ? pObject->GetName() : "");
    }
    return stream;
}

/**
 * Read a vector of objects the vector writer wrote, resolving each name in TheManager.
 *
 * An empty name, an unknown name, or an object of another class reads as null. Each element type
 * has its own instance, and the addresses are those of the RndMat and the RndFont instances.
 *
 * @param stream The stream to read from.
 * @param objects Receives the objects.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00395300
 * @ghidraAddress PAL: 0x004039f0
 * @ghidraAddress NTSC-U/C: 0x00395718
 * @ghidraAddress PAL: 0x00403e08
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::vector<T *> &objects) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    objects.resize(nSize, nullptr);
    for (T *&pElement : objects) {
        String name;
        stream >> name;
        if (name.GetLength() == 0) {
            pElement = nullptr;
        } else {
            pElement = dynamic_cast<T *>(TheManager.Find(name.c_str()));
        }
    }
    return stream;
}
