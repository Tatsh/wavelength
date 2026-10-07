#pragma once

/**
 * Abstract base of the game's sound output.
 *
 * The RTTI includes the class name and records no base. The vptr sits at offset 0. Two classes
 * implement it. SynthPS2 drives the sound hardware, and SynthNull supplies empty bodies.
 * Create() builds one of the two as TheSynth.
 *
 * The vtable runs past seventeen slots. The slots up to slot 9 are declared in order, and the
 * later slots are not yet declared. Slots 1, 2, 5, 6, 8, and 9 are pure in this class.
 */
class Synth {
public:
    /**
     * Build TheSynth and initialise it.
     *
     * A true "use_null_synth" key in the "synth" section of the configuration selects SynthNull.
     * Otherwise SynthPS2 is built under the allocation tag "SynthPs2".
     *
     * @ghidraAddress NTSC-U/C: 0x0024bc10
     * @ghidraAddress PAL: 0x00254678
     */
    static void Create();

    /**
     * Terminate TheSynth, destroy it, and clear the pointer.
     *
     * @ghidraAddress NTSC-U/C: 0x0024bcc8
     * @ghidraAddress PAL: 0x00254730
     */
    static void Destroy();

    /** Vtable slot 1. Bring up the sound output. */
    virtual void Init() = 0;

    /** Vtable slot 2. Shut down the sound output. */
    virtual void Terminate() = 0;

    /**
     * Vtable slot 3. The base body is empty, and the purpose is not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395f58
     */
    virtual void VirtualSlot3();

    /**
     * Vtable slot 4. Release the sound output.
     *
     * @ghidraAddress NTSC-U/C: 0x00395f60
     */
    virtual ~Synth();

    /**
     * Vtable slot 5. Send one packed message.
     *
     * SynthPS2 splits the word into its four bytes, lowest first, and passes them to
     * SendMessage().
     *
     * @param nPacked The four message bytes.
     */
    virtual void SendPackedMessage(unsigned int nPacked) = 0;

    /**
     * Vtable slot 6. Send one message.
     *
     * @param nByte0 The first message byte.
     * @param nByte1 The second message byte.
     * @param nByte2 The third message byte.
     * @param nByte3 The fourth message value.
     */
    virtual void
    SendMessage(unsigned char nByte0, unsigned char nByte1, unsigned char nByte2, int nByte3) = 0;

    /**
     * Vtable slot 7. Report an unsupported request.
     *
     * The base body passes "talk to denny" to the empty diagnostic routine at `0x00333d50`.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024bd28
     */
    virtual int Unsupported();

    /**
     * Vtable slot 8. Sample the system clock for the frame about to run.
     *
     * The main loop calls it at the start of each frame. The name is inferred.
     */
    virtual void UpdateTime() = 0;

    /**
     * Vtable slot 9. Service the sound output once per frame.
     *
     * The main loop calls it after the game database has been serviced. The name is inferred.
     */
    virtual void Poll() = 0;
};

/**
 * The sound output Synth::Create() built, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0a84
 */
extern Synth *TheSynth;
