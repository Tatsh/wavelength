#pragma once

#include <vector>

#include "game/avatarplayer.h"
#include "math/color.h"
#include "rnd/cam.h"
#include "rnd/tex.h"
#include "script/dataarray.h"

/**
 * Parts and colours of a player's avatar, the Freq the Freq maker edits.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each Campaign stores
 * one at `+0x74`, and the object is 0x3c bytes. The parts come from the `parts` array of the
 * `avatar` entry of the "db" section. Each part of that array has a `types` array that lists the
 * choices, and the torso part also has an `emblem` array with another `types` array. The set
 * records a choice as its index in the `types` array, less one, and a colour as three bytes.
 *
 * While the set shows, it borrows an AvatarPlayer that draws it. Every change returns the player.
 */
class AvatarPartSet {
public:
    /** The parts of an avatar, the indices of mParts and the keys of the `parts` array. */
    enum Part {
        kPartLeftArm = 0,    /*!< The left arm. Setting it also sets the right arm. */
        kPartRightArm = 1,   /*!< The right arm. */
        kPartHead = 2,       /*!< The head. */
        kPartTorso = 3,      /*!< The torso, with the emblem. */
        kPartLowerBody = 4,  /*!< The legs. */
        kPartHeadGear = 5,   /*!< The head gear. It may be absent. */
        kPartFaceGear = 6,   /*!< The face gear. It may be absent. */
        kPartInstrument = 7, /*!< The instrument. The Freq maker does not edit it. */
        kNumParts = 8,       /*!< The number of parts. */
    };

    /** The choice and the colour of one part. */
    struct PartSetting {
        unsigned char mType;     /*!< The index of the choice in the `types` array, less one. */
        unsigned char mColor[3]; /*!< The red, green, and blue bytes of the colour. */
    };

    /**
     * Prepare the avatar players and find the `parts` array.
     *
     * @ghidraAddress NTSC-U/C: 0x002718a0
     * @ghidraAddress PAL: 0x0027b450
     */
    static void Init();

    /**
     * List the choices of a part.
     *
     * @param nPart One of Part.
     * @param pTypes Receives the symbols of the choices, in the order of the `types` array.
     * @ghidraAddress NTSC-U/C: 0x00271690
     * @ghidraAddress PAL: 0x0027b240
     */
    static void GetPartTypes(int nPart, std::vector<const char *> *pTypes);

    /**
     * List the emblems.
     *
     * @param pTypes Receives the symbols of the emblems, in the order of the `types` array.
     * @ghidraAddress NTSC-U/C: 0x00271790
     * @ghidraAddress PAL: 0x0027b340
     */
    static void GetEmblemTypes(std::vector<const char *> *pTypes);

    /**
     * Construct the set from the `default_settings` of the `avatar` entry of the "db" section.
     *
     * @ghidraAddress NTSC-U/C: 0x002719c0
     * @ghidraAddress PAL: 0x0027b570
     */
    AvatarPartSet();

    /**
     * Copy every member of another set, the avatar player included.
     *
     * @param other The set to copy.
     */
    AvatarPartSet(const AvatarPartSet &other) = default;

    /**
     * Return the avatar player and destroy the set.
     *
     * @ghidraAddress NTSC-U/C: 0x00271a58
     * @ghidraAddress PAL: 0x0027b608
     */
    ~AvatarPartSet();

    /**
     * Copy the choices and the colours of another set, and return the avatar player.
     *
     * @param other The set to copy.
     * @return The set.
     * @ghidraAddress NTSC-U/C: 0x00271aa0
     * @ghidraAddress PAL: 0x0027b650
     */
    AvatarPartSet &operator=(const AvatarPartSet &other);

    /**
     * Read the choices and the colours from a description such as a prefab.
     *
     * Each entry after the first is an array that starts with the part and has a `type`, an
     * optional `color` (grey when absent), and for the torso an `emblem`.
     *
     * @param pData The description.
     * @ghidraAddress NTSC-U/C: 0x00271b48
     * @ghidraAddress PAL: 0x0027b6f8
     */
    void Load(DataArray *pData);

    /**
     * Choose a part. Choosing the left arm chooses the same right arm.
     *
     * @param nPart One of Part.
     * @param pszType The name of the choice.
     * @ghidraAddress NTSC-U/C: 0x00271cd8
     * @ghidraAddress PAL: 0x0027b888
     */
    void SetPart(int nPart, const char *pszType);

    /**
     * Colour a part, and the part of the avatar player that draws it. Colouring the left arm
     * colours the right arm.
     *
     * @param nPart One of Part.
     * @param pColor The colour. The alpha is not stored.
     * @ghidraAddress NTSC-U/C: 0x00271d50
     * @ghidraAddress PAL: 0x0027b900
     */
    void SetColor(int nPart, const Color *pColor);

    /**
     * Choose the emblem of the torso.
     *
     * @param pszEmblem The name of the emblem.
     * @ghidraAddress NTSC-U/C: 0x00271e20
     * @ghidraAddress PAL: 0x0027b9d0
     */
    void SetEmblem(const char *pszEmblem);

    /**
     * Report whether a part that every avatar needs has no choice. The gear and the instrument
     * are not checked.
     *
     * @return True when an arm, the head, the torso, or the legs are missing.
     * @ghidraAddress NTSC-U/C: 0x00271e60
     * @ghidraAddress PAL: 0x0027ba10
     */
    bool IsIncomplete() const;

    /**
     * Remove every part but the instrument and the emblem, grey the colours, and return the
     * avatar player.
     *
     * @ghidraAddress NTSC-U/C: 0x00271ea0
     * @ghidraAddress PAL: 0x0027ba50
     */
    void Clear();

    /**
     * Convert a colour component to a byte.
     *
     * @param fComponent The component, from 0 to 1.
     * @return The byte.
     * @ghidraAddress NTSC-U/C: 0x00271f00
     * @ghidraAddress PAL: 0x0027bab0
     */
    static unsigned char ColorToByte(float fComponent);

    /**
     * Convert a byte to a colour component.
     *
     * @param nByte The byte.
     * @return The component, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00271f20
     * @ghidraAddress PAL: 0x0027bad0
     */
    static float ByteToColor(unsigned char nByte);

    /**
     * Find a choice of a part.
     *
     * @param nPart One of Part.
     * @param pszType The choice, a symbol.
     * @return The index of the choice in the `types` array less one, or -1 with a warning.
     * @ghidraAddress NTSC-U/C: 0x00271f60
     * @ghidraAddress PAL: 0x0027bb10
     */
    static int FindPartType(int nPart, const char *pszType);

    /**
     * Find an emblem.
     *
     * @param pszEmblem The emblem, a symbol.
     * @return The index of the emblem in its `types` array less one, or -1 with a warning.
     * @ghidraAddress NTSC-U/C: 0x00272020
     * @ghidraAddress PAL: 0x0027bbd0
     */
    static int FindEmblem(const char *pszEmblem);

    /**
     * Return the avatar player that draws the set, if any.
     *
     * @ghidraAddress NTSC-U/C: 0x002721c8
     * @ghidraAddress PAL: 0x0027bd78
     */
    void ReleasePlayer();

    /**
     * Apply the pending parts to the avatar player once their files have loaded.
     *
     * The name is inferred.
     *
     * @return Whether no part is left pending.
     * @ghidraAddress NTSC-U/C: 0x002723a0
     * @ghidraAddress PAL: 0x0027bf50
     */
    bool UpdatePlayer();

    /**
     * Choose the animation the avatar player plays, and start it at once when a player draws the
     * set.
     *
     * The name is inferred.
     *
     * @param pszAnim The animation.
     * @param nFlags Stored in mBaseAnimFlags.
     * @ghidraAddress NTSC-U/C: 0x002725e0
     * @ghidraAddress PAL: 0x0027c190
     */
    void SetBaseAnim(const char *pszAnim, int nFlags);

    /**
     * Apply the parts to the avatar and render it into the avatar texture.
     *
     * @param pScreenRect The part of the screen the avatar fills, or null for the projector.
     * @ghidraAddress NTSC-U/C: 0x00272518
     * @ghidraAddress PAL: 0x0027c0c8
     */
    void Render(const Rnd::Cam::Rect *pScreenRect);

    /**
     * Report the name of the emblem.
     *
     * @return The symbol.
     * @ghidraAddress NTSC-U/C: 0x002726a0
     * @ghidraAddress PAL: 0x0027c250
     */
    const char *EmblemName() const;

    /**
     * Report the name of the choice of a part.
     *
     * @param nPart One of Part.
     * @return The symbol.
     * @ghidraAddress NTSC-U/C: 0x00272718
     * @ghidraAddress PAL: 0x0027c2c8
     */
    const char *PartName(int nPart) const;

    /**
     * Report the colour of a part.
     *
     * @param nPart One of Part.
     * @return The colour, opaque.
     * @ghidraAddress NTSC-U/C: 0x00272778
     * @ghidraAddress PAL: 0x0027c328
     */
    Color PartColor(int nPart) const;

    unsigned char mVersion;        /*!< The format of the set, 1 once loaded or constructed. */
    PartSetting mParts[kNumParts]; /*!< The choice and colour of each part. +0x01 */
    unsigned char mEmblem;         /*!< The emblem of the torso, an index as in mParts. +0x21 */
    AvatarPlayer *mPlayer;         /*!< The avatar player that draws the set, or null. +0x24 */
    int mFrame;                    /*!< The frame Render() last drew the set in. */
    int mPending;                  /*!< Non-zero while the player has parts to apply. */
    const char *mBaseAnim;         /*!< The animation the player plays, or null. */
    int mBaseAnimFlags;            /*!< Passed with mBaseAnim. The meaning is not recovered. */
    const char *mPoseAnim;         /*!< The pose the player shows, or null. +0x38 */

    /**
     * The `parts` array of the `avatar` entry of the "db" section, as Init() finds it.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14fc
     */
    static DataArray *sParts;
};

/**
 * The texture the avatar renders into.
 *
 * @ghidraAddress NTSC-U/C: 0x003b14cc
 */
extern Rnd::Tex *g_pAvatarTex;
