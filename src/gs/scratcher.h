#pragma once

#include "gs/muse.h"

/**
 * Player of the three recorded pieces one scratch button plays.
 *
 * The RTTI includes the class name and records no base. The vtable at `0x003ca460` has eight
 * slots after the type function: the destructor, Start(), Stop(), IsScratching(),
 * GetFirstLooperState(), SetPitch(), and SetPosition(). ScratchTrackBuilder builds one per button
 * of each scratch set, and ScratchTrack drives the scratchers of the current set. The member layout
 * is not yet recovered.
 */
class Scratcher {
public:
    /**
     * Construct a scratcher over three recorded pieces.
     *
     * @param pFirst The first piece.
     * @param pSecond The second piece.
     * @param pThird The third piece.
     * @param nQuantumTicks The quantisation the scratch position snaps to.
     * @param nLengthTicks The length of each piece.
     * @param nChannel The MIDI channel of the pieces.
     * @ghidraAddress NTSC-U/C: 0x00153e88
     * @ghidraAddress PAL: 0x001556f0
     */
    Scratcher(Muse *pFirst,
              Muse *pSecond,
              Muse *pThird,
              int nQuantumTicks,
              int nLengthTicks,
              int nChannel);

    /**
     * Stop scratching and release the pieces.
     *
     * @ghidraAddress NTSC-U/C: 0x00154210
     * @ghidraAddress PAL: 0x00155a78
     */
    virtual ~Scratcher();

    /**
     * Start scratching for a player.
     *
     * @param nPlayer The player index.
     * @param nValue The value the scratcher records with the player. Its use is not yet
     * identified.
     * @ghidraAddress NTSC-U/C: 0x00154360
     * @ghidraAddress PAL: 0x00155bc8
     */
    virtual void Start(int nPlayer, int nValue);

    /**
     * Stop scratching and silence the scratch sound.
     *
     * @ghidraAddress NTSC-U/C: 0x00154440
     * @ghidraAddress PAL: 0x00155ca8
     */
    virtual void Stop();

    /**
     * Report whether the scratcher is scratching.
     *
     * Start() sets the word this reads and Stop() clears it. The name is inferred.
     *
     * @return Non-zero while scratching.
     * @ghidraAddress NTSC-U/C: 0x001544b0
     */
    virtual int IsScratching();

    /**
     * Report a state word of the looper of the first piece.
     *
     * The purpose of the word is not yet identified. The name is inferred.
     *
     * @return The word.
     * @ghidraAddress NTSC-U/C: 0x00154340
     * @ghidraAddress PAL: 0x00155ba8
     */
    virtual int GetFirstLooperState();

    /**
     * Set the pitch, which takes effect when it moves by more than 0.02.
     *
     * @param fPitch The pitch.
     * @ghidraAddress NTSC-U/C: 0x001544b8
     * @ghidraAddress PAL: 0x00155d20
     */
    virtual void SetPitch(float fPitch);

    /**
     * Set the scratch position from the stick.
     *
     * @param fPosition The stick position.
     * @ghidraAddress NTSC-U/C: 0x00154508
     * @ghidraAddress PAL: 0x00155d70
     */
    virtual void SetPosition(float fPosition);
};
