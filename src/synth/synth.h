#pragma once

#include <vector>

#include "os/string.h"
#include "synth/softfxfilter.h"
#include "synth/streamplayer.h"

/**
 * Abstract base of the game's sound output.
 *
 * The RTTI includes the class name and records no base. The vptr sits at offset 0. Two classes
 * implement it. SynthPS2 drives the sound hardware, and SynthNull supplies empty bodies.
 * Create() builds one of the two as TheSynth.
 *
 * The vtable has 38 slots, declared in order.
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
     * Vtable slot 11. Set the stream the output plays, or none.
     *
     * The base body is empty. The name is inferred.
     *
     * @param pStream The stream, or null.
     * @ghidraAddress NTSC-U/C: 0x00395f98
     */
    virtual void SetStream(StreamPlayer *pStream);

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

    /**
     * Vtable slot 15. Queue the division of the sound memory among the bank slots.
     *
     * The member is pure in this class. SynthPS2 copies the sizes into a queued job and returns 0.
     * The name is inferred.
     *
     * @param blocks The size of each bank slot in 64-byte blocks, indexed by slot. A negative size
     *     leaves the slot as it is.
     * @return 0.
     */
    virtual int Partition(const std::vector<int> &blocks) = 0;

    /**
     * Vtable slot 16. Report the size of a bank file in 64-byte blocks, rounded up.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param file The bank file.
     * @return The number of blocks.
     */
    virtual int GetBankBlockCount(const String &file) = 0;

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
     * Vtable slot 23. Set the lag of the output, the value of the `lag_ms` entry of the `synth`
     * section.
     *
     * The base body is empty. The name is inferred.
     *
     * @param fLag The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00395fc8
     */
    virtual void SetLag(float fLag);

    /**
     * Vtable slot 24. Report the lag slot 23 set.
     *
     * The base body returns 0. The name is inferred.
     *
     * @return The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00395fd0
     */
    virtual float GetLag();

    /**
     * Vtable slot 25. Set the position of the sweep of a sweeping software filter.
     *
     * The base body is empty. SynthPS2 records the position for the filter types that sweep and
     * reports any other type. The name is inferred.
     *
     * @param fPosition The position, 1 for the top of the sweep.
     * @ghidraAddress NTSC-U/C: 0x00395fe0
     */
    virtual void SetSoftFxSweep(float fPosition);

    /**
     * Vtable slot 26. The base body is empty. The parameters are not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00395fe8
     */
    virtual void VirtualSlot26();

    /**
     * Vtable slot 27. Send the settings of the software effect.
     *
     * The base body is empty. The name is inferred.
     *
     * @param filter The settings.
     * @ghidraAddress NTSC-U/C: 0x00395ff0
     */
    virtual void SetSoftFxFilter(const SoftFxFilter &filter);

    /**
     * Vtable slot 28. Queue a job that turns the software effects on.
     *
     * The member is pure in this class. SynthPS2 returns 0. The name is inferred.
     *
     * @return 0.
     */
    virtual int EnableSoftFx() = 0;

    /**
     * Vtable slot 29. Queue a job that turns the software effects off.
     *
     * The member is pure in this class. SynthPS2 returns 0. The name is inferred.
     *
     * @return 0.
     */
    virtual int DisableSoftFx() = 0;

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

    /**
     * Vtable slot 31. Report the output level.
     *
     * The base body returns 1. The name is inferred.
     *
     * @return The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00396000
     */
    virtual float GetOutputLevel();

    /**
     * Vtable slot 32. The base body is empty, and the purpose is not yet recovered.
     *
     * SynthPS2 sends the value to the sound processor as command 0x6d. The metagame passes 1
     * before it plays the intro movie.
     *
     * @param nValue The value sent.
     * @ghidraAddress NTSC-U/C: 0x00396010
     */
    virtual void VirtualSlot32(int nValue);

    /**
     * Vtable slot 33. Set whether the effect buses feed the hardware effects.
     *
     * The base body is empty. SynthPS2 sends the value as command 0x6e. The name is inferred.
     *
     * @param nOn Whether the buses feed the hardware effects.
     * @ghidraAddress NTSC-U/C: 0x00396018
     */
    virtual void SetBusToCoreFx(int nOn);

    /**
     * Vtable slot 34. Set whether the effect buses feed the software effect.
     *
     * The base body is empty, and SynthPS2 does not override it with any work. The name is
     * inferred.
     *
     * @param nOn Whether the buses feed the software effect.
     * @ghidraAddress NTSC-U/C: 0x00396020
     */
    virtual void SetBusToSoftFx(int nOn);

    /**
     * Vtable slot 35. The base body is empty, and the purpose is not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00396028
     */
    virtual void VirtualSlot35();

    /**
     * Vtable slot 36. Queue a job that sends packed messages.
     *
     * The base body is empty. The name is inferred.
     *
     * @param messages The messages, each packed as SendPackedMessage() takes it.
     * @ghidraAddress NTSC-U/C: 0x00396030
     */
    virtual void SendMessages(const std::vector<unsigned int> &messages);

    /** Routine a queued callback job calls with its argument. */
    typedef void (*Callback)(int nArg);

    /**
     * Vtable slot 37. Queue a job that calls a routine once the jobs before it have run.
     *
     * The base body is empty. The name is inferred.
     *
     * @param pfnCallback The routine.
     * @param nArg The argument passed to the routine.
     * @ghidraAddress NTSC-U/C: 0x00396038
     */
    virtual void QueueCallback(Callback pfnCallback, int nArg);
};

/**
 * The sound output Synth::Create() built, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0a84
 */
extern Synth *TheSynth;
