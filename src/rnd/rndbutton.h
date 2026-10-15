#pragma once

#include <cstddef>
#include <vector>

#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndfont.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndobject.h"
#include "rnd/rndtext.h"

/**
 * Selectable widget, as a mesh and a text with a material and a font for each state.
 *
 * The RTTI includes the class name and records RndObject as the one base, non-virtual. The class
 * draws nothing. The front end reads the state and the palettes.
 */
class RndButton : public RndObject {
public:
    /** The number of entries each palette starts with. */
    static constexpr int kStateCount = 4;

    /**
     * Construct a button in state 0 without targets and with four empty entries in each palette.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0024af90
     * @ghidraAddress PAL: 0x002539f8
     */
    explicit RndButton(const char *pszName);

    /**
     * Drop the references on the targets and on every palette entry.
     *
     * @ghidraAddress NTSC-U/C: 0x0024aee8
     * @ghidraAddress PAL: 0x00253950
     */
    ~RndButton() override;

    /**
     * Allocate a button, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Button", 0);
    }

    /**
     * Release a button.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Write a description of the button and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0024a668
     * @ghidraAddress PAL: 0x002530d0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the version, the state, the names of the targets, and the palettes.
     *
     * The base writes nothing.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0024a788
     * @ghidraAddress PAL: 0x002531f0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a target or a palette entry with another object.
     *
     * A replacement of another class, or null, clears the target or the entry.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0024ac38
     * @ghidraAddress PAL: 0x002536a0
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00395900
     * @ghidraAddress PAL: 0x00403ff0
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy the state and the palettes of another button.
     *
     * The targets are cleared rather than copied.
     *
     * @param pSource The button to copy from.
     * @param nFlags The set of fields to copy. The routine ignores it.
     * @ghidraAddress NTSC-U/C: 0x0024a5c8
     * @ghidraAddress PAL: 0x00253030
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0024a880
     * @ghidraAddress PAL: 0x002532e8
     */
    void Load(BinStream &stream) override;

    /**
     * Create a button.
     *
     * @param pszName The registry key.
     * @return The button.
     * @ghidraAddress NTSC-U/C: 0x00395920
     * @ghidraAddress PAL: 0x00404010
     */
    static RndObject *New(const char *pszName) {
        return new RndButton(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Button`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a74
     * @ghidraAddress PAL: 0x0042f90c
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a78
     * @ghidraAddress PAL: 0x0042f910
     */
    static int sRev;

    /**
     * The version Load() read last.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c824
     */
    static int sLoadRev;

    int mState;                    /*!< The index of the current state. */
    RndMesh *mMesh;                /*!< The mesh target, or null. */
    RndText *mText;                /*!< The text target, or null. */
    std::vector<RndMat *> mMats;   /*!< The material of each state. */
    std::vector<RndFont *> mFonts; /*!< The font of each state. */

protected:
    /**
     * Drop the references on the targets and on every palette entry.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0024ab58
     * @ghidraAddress PAL: 0x002535c0
     */
    void ReleaseRefs();

    /**
     * Take a reference on the targets and on every palette entry.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0024aa78
     * @ghidraAddress PAL: 0x002534e0
     */
    void AcquireRefs();
};
