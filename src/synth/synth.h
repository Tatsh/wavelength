#pragma once

#include "os/string.h"

/**
 * Abstract base of the game's sound output.
 *
 * The RTTI includes the class name and records no base. The vptr sits at offset 0. Two classes
 * implement it. SynthPS2 drives the sound hardware, and SynthNull supplies empty bodies.
 * Create() builds one of the two as TheSynth.
 *
 * The vtable has 34 slots. The slots up to slot 30 are declared in order, and the later slots are
 * not yet declared.
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

    /**
     * Vtable slot 10. The base body returns 0. The parameters are not yet recovered.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00395f90
     */
    virtual int VirtualSlot10();

    /**
     * Vtable slot 11. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395f98
     */
    virtual void VirtualSlot11();

    /**
     * Vtable slot 12. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fa0
     */
    virtual void VirtualSlot12();

    /**
     * Vtable slot 13. The base body returns 0. The parameters are not yet recovered.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00395fa8
     */
    virtual int VirtualSlot13();

    /** Vtable slot 14, pure in this class. The signature is not yet recovered. */
    virtual void VirtualSlot14() = 0;

    /** Vtable slot 15, pure in this class. The signature is not yet recovered. */
    virtual void VirtualSlot15() = 0;

    /** Vtable slot 16, pure in this class. The signature is not yet recovered. */
    virtual void VirtualSlot16() = 0;

    /**
     * Vtable slot 17. Report whether a sample bank finished loading into a slot.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nSlot The bank slot.
     * @return Whether the bank is loaded.
     */
    virtual bool IsBankLoaded(unsigned short nSlot) = 0;

    /**
     * Vtable slot 18. The base body returns 1. The parameters are not yet recovered.
     *
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x00395fb0
     */
    virtual int VirtualSlot18();

    /**
     * Vtable slot 19. Start loading a sample bank file into a slot.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nSlot The bank slot.
     * @param file The bank file.
     * @param bFlag The meaning is not yet recovered. Duel passes true.
     */
    virtual void LoadBank(unsigned short nSlot, const String &file, bool bFlag) = 0;

    /**
     * Vtable slot 20. Empty a sample bank slot.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nSlot The bank slot.
     */
    virtual void UnloadBank(unsigned short nSlot) = 0;

    /**
     * Vtable slot 21. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fb8
     */
    virtual void VirtualSlot21();

    /**
     * Vtable slot 22. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fc0
     */
    virtual void VirtualSlot22();

    /**
     * Vtable slot 23. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fc8
     */
    virtual void VirtualSlot23();

    /**
     * Vtable slot 24. The base body returns 0. The parameters are not yet recovered.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00395fd0
     */
    virtual int VirtualSlot24();

    /**
     * Vtable slot 25. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fe0
     */
    virtual void VirtualSlot25();

    /**
     * Vtable slot 26. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fe8
     */
    virtual void VirtualSlot26();

    /**
     * Vtable slot 27. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395ff0
     */
    virtual void VirtualSlot27();

    /** Vtable slot 28, pure in this class. The signature is not yet recovered. */
    virtual void VirtualSlot28() = 0;

    /** Vtable slot 29, pure in this class. The signature is not yet recovered. */
    virtual void VirtualSlot29() = 0;

    /**
     * Vtable slot 30. Set the output level that slot 31 reports.
     *
     * The base body is empty. SynthPS2 scales the level by 16383 and sends it to the sound
     * processor. The name is inferred.
     *
     * @param fLevel The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00395ff8
     */
    virtual void SetOutputLevel(float fLevel);
};

/**
 * The sound output Synth::Create() built, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0a84
 */
extern Synth *TheSynth;
