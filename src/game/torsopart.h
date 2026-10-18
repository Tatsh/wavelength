#pragma once

#include <memory>

#include "game/avatarpart.h"
#include "rnd/rndloader.h"
#include "rnd/rndtex.h"

class Avatar;

/**
 * The model of the torso of an avatar, with the emblem.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. The
 * emblem is a texture from a file the `emblem` array of the part lists, shown on the last stage of
 * the material of the torso.
 */
class TorsoPart : public AvatarPart {
public:
    /**
     * Construct the model of the torso without an emblem.
     *
     * @param nParent The part the model hangs from.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x002747e8
     * @ghidraAddress PAL: 0x0027e308
     */
    TorsoPart(int nParent, Avatar *pAvatar);

    /**
     * Abandon the load of the emblem and destroy the model.
     *
     * @ghidraAddress NTSC-U/C: 0x00274840
     * @ghidraAddress PAL: 0x0027e360
     */
    ~TorsoPart() override;

    /**
     * Build the meshes of the torso and show the emblem on them.
     *
     * The name follows the base.
     *
     * @return False while a file is still loading, otherwise true.
     * @ghidraAddress NTSC-U/C: 0x00274990
     * @ghidraAddress PAL: 0x0027e4b0
     */
    bool Rebuild() override;

    /**
     * Abandon the loads of the torso and of the emblem.
     *
     * The name follows the base.
     *
     * @ghidraAddress NTSC-U/C: 0x002749e0
     * @ghidraAddress PAL: 0x0027e500
     */
    void ReleaseType() override;

    /**
     * Start loading the file of an emblem.
     *
     * The load replaces any load of an emblem in progress. The name is inferred.
     *
     * @param pszEmblem The emblem, a symbol.
     * @ghidraAddress NTSC-U/C: 0x00274898
     * @ghidraAddress PAL: 0x0027e3b8
     */
    void SetEmblem(const char *pszEmblem);

    /**
     * Take the texture of the emblem SetEmblem() loaded and show it.
     *
     * The texture is taken out of the file before the loader is deleted, and renamed for the
     * avatar. The name is inferred.
     *
     * @return False while the file is still loading, otherwise true.
     * @ghidraAddress NTSC-U/C: 0x00274a28
     * @ghidraAddress PAL: 0x0027e548
     */
    bool RebuildEmblem();

    const char *mEmblem;                      /*!< The emblem shown, Avatar::sNone for none. */
    const char *mPendingEmblem;               /*!< The emblem SetEmblem() last requested. */
    RndTex *mEmblemTex;                       /*!< The texture of the emblem, or null. */
    std::unique_ptr<RndLoader> mEmblemLoader; /*!< The load of the file of mPendingEmblem. */

private:
    /**
     * Show the emblem on the last stage of the material, or hide that stage for no emblem.
     *
     * A material with fewer than two stages is not changed. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00274ba8
     * @ghidraAddress PAL: 0x0027e6c8
     */
    void ApplyEmblem();
};
