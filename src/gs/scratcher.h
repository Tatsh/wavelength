#pragma once

#include <vector>

#include "gs/muse.h"
#include "gs/muselooper.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Player of the three recorded pieces one scratch button plays.
 *
 * The RTTI includes the class name and records no base, and the nested Scratcher::ScratchNoteCB.
 * The vtable at `0x003ca460` has eight slots after the type function: the destructor, Start(),
 * Stop(), IsScratching(), GetLoopLength(), SetPitch(), and SetPosition(). ScratchTrackBuilder
 * builds one per button of each scratch set, and ScratchTrack drives the scratchers of the current
 * set. The stick picks the piece that plays, and the pitch bends the channel.
 */
class Scratcher {
public:
    /** Number of pieces, one for each stick position. */
    static constexpr int kNumPieces = 3;

    /**
     * Receiver that shows each note a piece starts on the scratching player's track.
     *
     * The RTTI includes the nested name and records Muse::NoteCB as the base. Its constructor and
     * destructor are inline, and Start() sets its members.
     */
    class ScratchNoteCB : public Muse::NoteCB {
    public:
        /**
         * Show a note at the current tick.
         *
         * @param nNote The MIDI note number. The body does not read it.
         * @param nDuration The length of the note in ticks.
         * @ghidraAddress NTSC-U/C: 0x00153e38
         * @ghidraAddress PAL: 0x001556a0
         */
        void OnNote(unsigned char nNote, int nDuration) override;

        int mPlayer;     /*!< The scratching player. */
        int mTickOffset; /*!< The ticks added to the current tick of each note. */
    };

    /**
     * Construct a scratcher over three recorded pieces.
     *
     * The scratcher loops each piece and keeps a copy of each.
     *
     * @param pFirst The piece for the stick held left.
     * @param pSecond The piece for the stick at rest.
     * @param pThird The piece for the stick held right.
     * @param nQuantumTicks The ticks of the beat a piece starts on.
     * @param nLengthTicks The ticks of one loop of each piece.
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
     * A piece starts at once in the first half of a beat and at the next beat otherwise.
     *
     * @param nPlayer The player index.
     * @param nTickOffset The ticks the notes of the pieces are shown ahead of the current tick.
     * @ghidraAddress NTSC-U/C: 0x00154360
     * @ghidraAddress PAL: 0x00155bc8
     */
    virtual void Start(int nPlayer, int nTickOffset);

    /**
     * Stop scratching and bend the channel back.
     *
     * @ghidraAddress NTSC-U/C: 0x00154440
     * @ghidraAddress PAL: 0x00155ca8
     */
    virtual void Stop();

    /**
     * Report whether the scratcher is scratching.
     *
     * @return Non-zero while scratching.
     * @ghidraAddress NTSC-U/C: 0x001544b0
     */
    virtual int IsScratching();

    /**
     * Report the ticks of one loop of the pieces.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x00154340
     * @ghidraAddress PAL: 0x00155ba8
     */
    virtual int GetLoopLength();

    /**
     * Set the pitch. A change of 0.02 or less is ignored.
     *
     * @param fPitch The pitch, from -3 to 3 semitones.
     * @ghidraAddress NTSC-U/C: 0x001544b8
     * @ghidraAddress PAL: 0x00155d20
     */
    virtual void SetPitch(float fPitch);

    /**
     * Pick the piece from the horizontal position of the stick.
     *
     * @param fPosition The stick position, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x00154508
     * @ghidraAddress PAL: 0x00155d70
     */
    virtual void SetPosition(float fPosition);

private:
    /**
     * Bend the channel.
     *
     * @param fPitch The bend in semitones, from -3 to 3.
     * @ghidraAddress NTSC-U/C: 0x001545a0
     * @ghidraAddress PAL: 0x00155e08
     */
    void SendPitchBend(float fPitch);

    /**
     * Make a piece the current one, moving playback to it at the same place in the beat.
     *
     * @param nPiece The piece.
     * @ghidraAddress NTSC-U/C: 0x00154630
     * @ghidraAddress PAL: 0x00155e98
     */
    void SwitchPiece(int nPiece);

    /**
     * Start the current piece one tick into its loop.
     *
     * @ghidraAddress NTSC-U/C: 0x001546e8
     * @ghidraAddress PAL: 0x00155f50
     */
    void Restart();

    int mChannel;                       /*!< The MIDI channel of the pieces. */
    int mQuantumTicks;                  /*!< The ticks of the beat a piece starts on. */
    std::vector<MuseLooper *> mLoopers; /*!< The looper of each piece. */
    std::vector<Ptr<Muse> > mCopies;    /*!< A copy of each piece. */
    Ptr<Command> mRestartCmd;           /*!< The command that calls Restart(). */
    int mScratching;                    /*!< Whether the scratcher is scratching. */
    float mPitch;                       /*!< The pitch SetPitch() last took. */
    int mReserved34;                    // +0x34, cleared by the constructor and not yet identified.
    int mPiece;                         /*!< The current piece. */
    ScratchNoteCB mNoteCB;              /*!< The receiver every piece reports its notes to. */
};
