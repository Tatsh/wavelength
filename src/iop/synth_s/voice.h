#pragma once

#include "synth_s/banksampledesc.h"

/** The life of a Voice. */
enum VoiceState {
    kVoiceFree = 0,     /*!< Unused. */
    kVoiceKeyOn = 1,    /*!< Assigned to a note whose key-on is pending this tick. */
    kVoicePlaying = 2,  /*!< Keyed on. */
    kVoiceReleased = 3, /*!< Keyed off and fading out. */
};

/**
 * One SPU2 voice and the note it plays.
 *
 * The class has no RTTI, and its name is inferred from its role. The synthesiser has one per
 * hardware voice of each core and a list of retired copies of voices whose one-shot waveform
 * ended before the note-off.
 */
class Voice {
public:
    /** Construct a free voice with no descriptor. */
    Voice() : mSampleDesc(nullptr) {
        Init();
    }

    /**
     * Free the voice.
     *
     * @ghidraAddress NTSC-U/C: 0x00002190
     * @ghidraAddress PAL: 0x00002190
     */
    void Init();

    /**
     * Print the voice's descriptor to the console.
     *
     * @ghidraAddress NTSC-U/C: 0x000021dc
     * @ghidraAddress PAL: 0x000021dc
     */
    void Dump() const;

    /**
     * Find the first free voice of a core, or else the best voice to steal.
     *
     * The steal candidate is the playing voice with the lowest priority, the oldest among equals.
     * The search starts from the first voice whatever its state.
     *
     * @param voices The core's voices.
     * @param count Number of voices.
     * @param stealIndex Receives -1 when a voice is free, otherwise the steal candidate.
     * @return The free voice's index, or -1 when none is free.
     * @ghidraAddress NTSC-U/C: 0x0000220c
     * @ghidraAddress PAL: 0x0000220c
     */
    static int FindFree(Voice *voices, int count, int *stealIndex);

    /**
     * Compute the left and right volume registers of a note.
     *
     * Every level is an index into the volume curve. With @p surround set, one side is inverted
     * for the surround decoder.
     *
     * @param velocity Note velocity.
     * @param bankVolume Bank volume.
     * @param programVolume Program volume.
     * @param sampleVolume Descriptor volume.
     * @param channelVolume Channel volume.
     * @param channelExpression Channel expression.
     * @param pan Combined pan, zero to 127.
     * @param left Receives the left volume.
     * @param right Receives the right volume.
     * @param surround Nonzero to invert one side.
     * @ghidraAddress NTSC-U/C: 0x00002330
     * @ghidraAddress PAL: 0x00002330
     */
    static void CalculateVolume(unsigned char velocity,
                                unsigned char bankVolume,
                                unsigned char programVolume,
                                unsigned char sampleVolume,
                                unsigned char channelVolume,
                                unsigned char channelExpression,
                                unsigned char pan,
                                unsigned int *left,
                                unsigned int *right,
                                signed char surround);

    /**
     * Compute the pitch register of a note.
     *
     * @param note Note to play.
     * @param baseKey Note at which the sample plays at its sample rate.
     * @param semitones Transposition in semitones.
     * @param cents Transposition in cents, borrowed into @p semitones when negative.
     * @param sampleRate Rate of the sample in hertz.
     * @return The pitch register value scaled from 48000 hertz to @p sampleRate.
     * @ghidraAddress NTSC-U/C: 0x00002498
     * @ghidraAddress PAL: 0x00002498
     */
    static unsigned int CalculatePitch(
        unsigned char note, unsigned char baseKey, int semitones, int cents, int sampleRate);

    /**
     * Compute the voice's volume registers from its channel, bank, program, and descriptor.
     *
     * The outputs are not written when the channel has no program.
     *
     * @param left Receives the left volume.
     * @param right Receives the right volume.
     * @ghidraAddress NTSC-U/C: 0x000042f0
     * @ghidraAddress PAL: 0x000042f0
     */
    void ComputeVolume(unsigned int *left, unsigned int *right) const;

    /**
     * Compute the voice's pitch register from its channel, program, and descriptor, without
     * vibrato.
     *
     * @return The pitch register value.
     * @ghidraAddress NTSC-U/C: 0x00004698
     * @ghidraAddress PAL: 0x00004698
     */
    unsigned int ComputePitch() const;

    unsigned char mChannel;      /*!< MIDI channel of the note. */
    unsigned char mNote;         /*!< Note number. */
    unsigned short mBank;        /*!< Bank slot of the note's program. */
    unsigned char mVelocity;     /*!< Note velocity. */
    int mPitch;                  /*!< Pitch register value before vibrato. */
    int mVibratoPhase;           /*!< Offset into the vibrato table, or -1 for no vibrato. */
    int mVibratoTable;           /*!< Zero for the fast table, one for the slow one. */
    unsigned int mAge;           /*!< Note-on serial; smaller is older. */
    int mState;                  /*!< A #VoiceState. */
    BankSampleDesc *mSampleDesc; /*!< Descriptor of the note. */
    unsigned char mPriority;     /*!< Channel priority at the note-on. */
};
