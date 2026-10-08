#pragma once

#include <cstddef>
#include <list>

#include "os/binstream.h"
#include "os/filepath.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndtex.h"

/**
 * Animation that plays a movie file into a texture.
 *
 * The RTTI includes the class name and records RndAnimatable as the one base. The platform
 * subclass decodes the file. This class records the file and the texture only.
 */
class RndMovie : public RndAnimatable {
public:
    /**
     * Construct a movie without a file or a texture.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0023a728
     * @ghidraAddress PAL: 0x002432a8
     */
    explicit RndMovie(const char *pszName);

    /**
     * Close the movie.
     *
     * @ghidraAddress NTSC-U/C: 0x0023a9e8
     * @ghidraAddress PAL: 0x00243568
     */
    ~RndMovie() override;

    /**
     * Allocate a movie, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Movie", 0);
    }

    /**
     * Release a movie.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Add the texture to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0023b0c8
     * @ghidraAddress PAL: 0x00243c48
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Take a reference on the texture.
     *
     * The platform subclass then prepares the texture for the file. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023ac40
     * @ghidraAddress PAL: 0x002437c0
     */
    virtual void Open();

    /**
     * Drop the reference on the texture.
     *
     * The platform subclass first releases what Open() prepared. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023ac70
     * @ghidraAddress PAL: 0x002437f0
     */
    virtual void Close();

    /**
     * Write a description of the movie and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023aca0
     * @ghidraAddress PAL: 0x00243820
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the movie and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023ad50
     * @ghidraAddress PAL: 0x002438d0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace the texture with another.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0023b018
     * @ghidraAddress PAL: 0x00243b98
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0038fa48
     * @ghidraAddress PAL: 0x003fe150
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another movie and its base, closing the movie first and opening it after.
     *
     * @param pSource The movie to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0023af50
     * @ghidraAddress PAL: 0x00243ad0
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, closing the movie first and opening
     * it after.
     *
     * An early version does not include the texture, which is then unchanged.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0023ade8
     * @ghidraAddress PAL: 0x00243968
     */
    void Load(BinStream &stream) override;

    /**
     * Create a movie.
     *
     * @param pszName The registry key.
     * @return The movie.
     * @ghidraAddress NTSC-U/C: 0x0038fa58
     * @ghidraAddress PAL: 0x003fe160
     */
    static RndObject *New(const char *pszName) {
        return new RndMovie(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Movie`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a28
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a2c
     */
    static int sRev;

    FilePath mFile; /*!< The movie file. */
    RndTex *mTex;   /*!< The texture the movie plays into. */
};
