#pragma once

#include "game/avatarpart.h"

class Avatar;

/**
 * The model of the instrument an avatar plays.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. Every
 * instrument mesh comes from the one file the `rnd_dir` of the part names. A choice of the `types`
 * array gives the number of its mesh, `avatar<n>_instr_<number>.mesh`.
 */
class InstrPart : public AvatarPart {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a3478
     * @ghidraAddress PAL: 0x00412158
     */
    ~InstrPart() override = default;

    /**
     * Construct the model of the instrument.
     *
     * @param nParent The part the model hangs from.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x00274498
     * @ghidraAddress PAL: 0x0027dfb8
     */
    InstrPart(int nParent, Avatar *pAvatar);

    /**
     * Start loading the instrument file on the first call, then show a choice once the file has
     * loaded.
     *
     * The name follows the base.
     *
     * @param pszType The choice, a symbol.
     * @ghidraAddress NTSC-U/C: 0x002744d8
     * @ghidraAddress PAL: 0x0027dff8
     */
    void SetType(const char *pszType) override;

    /**
     * Show the mesh of the pending choice once the instrument file has loaded.
     *
     * The first call after the load renames the objects of the file for the avatar. The name
     * follows the base.
     *
     * @return False while the file is still loading, otherwise true.
     * @ghidraAddress NTSC-U/C: 0x00274560
     * @ghidraAddress PAL: 0x0027e080
     */
    bool Rebuild() override;

    /**
     * Return to the choice the mesh shows. The instrument file stays loaded.
     *
     * The name follows the base.
     *
     * @ghidraAddress NTSC-U/C: 0x002747d8
     * @ghidraAddress PAL: 0x0027e2f8
     */
    void ReleaseType() override;

    /**
     * Non-zero once the objects of the instrument file have the avatar names. The constructor does
     * not set it, and the first SetType() clears it.
     */
    int mRenamed;
};
