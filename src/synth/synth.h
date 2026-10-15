#pragma once

#include <vector>

#include "os/string.h"
#include "os/system.h"
#include "synth/softfxfilter.h"
#include "synth/streamplayer.h"

#ifdef VIDEO_STANDARD_PAL
/**
 * Report the path of a sound bank of the console's language, in SoundFX for English and in the
 * language's directory of SoundFX otherwise. Every caller inlines the path.
 *
 * @param pszFile The bank file, relative to its directory.
 * @return The path, in the buffer of FormatString().
 */
inline const char *LanguageSoundFxPath(const char *pszFile) {
    if (SystemLanguageIsEnglish()) {
        return FormatString("SoundFX/%s", pszFile);
    }
    return FormatString("SoundFX/%s/%s", GetSystemLanguage(), pszFile);
}
#endif

/**
 * Abstract base of the game's sound output.
 *
 * The RTTI includes the class name and records no base. The vptr sits at offset 0. Two classes
 * implement it. SynthPS2 drives the sound hardware through the `synth` and `softfx` IOP modules,
 * and SynthNull supplies empty bodies. Create() builds one of the two as TheSynth.
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

    /**
     * Vtable slot 1. Bring up the sound output.
     *
     * @return The status of the start-up, 0 when it succeeded.
     */
    virtual int Init() = 0;

    /**
     * Vtable slot 2. Shut down the sound output.
     *
     * @return 0.
     */
    virtual int Terminate() = 0;

    /**
     * Vtable slot 3. Initialise the sound processor again.
     *
     * The base body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00395f58
     * @ghidraAddress PAL: 0x00404600
     */
    virtual void ResetSpu();

    /**
     * Vtable slot 4. Release the sound output.
     *
     * @ghidraAddress NTSC-U/C: 0x00395f60
     * @ghidraAddress PAL: 0x004047f0
     */
    virtual ~Synth();

    /**
     * Vtable slot 5. Send one packed message.
     *
     * SynthPS2 splits the word into its four bytes, lowest first, and passes them to
     * SendMessage().
     *
     * @param nPacked The four message bytes.
     * @return 0.
     */
    virtual int SendPackedMessage(unsigned int nPacked) = 0;

    /**
     * Vtable slot 6. Send one message.
     *
     * @param nByte0 The first message byte.
     * @param nByte1 The second message byte.
     * @param nByte2 The third message byte.
     * @param nByte3 The fourth message value, the delay of the message in milliseconds.
     * @return 0.
     */
    virtual int
    SendMessage(unsigned char nByte0, unsigned char nByte1, unsigned char nByte2, int nByte3) = 0;

    /**
     * Vtable slot 7. Report an unsupported request.
     *
     * The base body passes "talk to denny" to the empty diagnostic routine at `0x00333d50`. The
     * PAL body only returns.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024bd28
     * @ghidraAddress PAL: 0x00404608
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
     *
     * @return 0.
     */
    virtual int Poll() = 0;

    /**
     * Vtable slot 10. Report the stream SetStream() set.
     *
     * The base body returns null. The name is inferred.
     *
     * @return The stream, or null.
     * @ghidraAddress NTSC-U/C: 0x00395f90
     * @ghidraAddress PAL: 0x00404610
     */
    virtual StreamPlayer *GetStream();

    /**
     * Vtable slot 11. Set the stream the output plays, or none.
     *
     * The base body is empty. The name is inferred.
     *
     * @param pStream The stream, or null.
     * @ghidraAddress NTSC-U/C: 0x00395f98
     * @ghidraAddress PAL: 0x00404618
     */
    virtual void SetStream(StreamPlayer *pStream);

    /**
     * Vtable slot 12. Append a run of stream data to the stream the software effect plays.
     *
     * The base body is empty. An empty run ends the stream. The name is inferred.
     *
     * @param pData The data.
     * @param nSize The size of the data in bytes.
     * @ghidraAddress NTSC-U/C: 0x00395fa0
     * @ghidraAddress PAL: 0x00404620
     */
    virtual void SendStreamData(const void *pData, int nSize);

    /**
     * Vtable slot 13. Report the room the software effect has for stream data.
     *
     * The base body returns 0. The name is inferred.
     *
     * @return The room in bytes.
     * @ghidraAddress NTSC-U/C: 0x00395fa8
     * @ghidraAddress PAL: 0x00404628
     */
    virtual int GetStreamStatus();

    /**
     * Vtable slot 14, pure in this class. Report the format of the stream data.
     *
     * SynthPS2 reports 28351 and 64. The name is inferred.
     *
     * @param pnRate Receives the first value.
     * @param pnBlock Receives the second value.
     * @return 0.
     */
    virtual int GetStreamFormat(int *pnRate, int *pnBlock) = 0;

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
     * Vtable slot 18. Report whether a slot has a bank or a bank waiting to load.
     *
     * The base body returns 1. The name is inferred.
     *
     * @param nSlot The bank slot.
     * @return 1 when the slot has a bank or a pending load, otherwise 0.
     * @ghidraAddress NTSC-U/C: 0x00395fb0
     * @ghidraAddress PAL: 0x00404630
     */
    virtual int IsBankPresent(unsigned short nSlot);

    /**
     * Vtable slot 19. Start loading a sample bank file into a slot.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nSlot The bank slot.
     * @param file The bank file.
     * @param bSkipPending Do nothing when a load into the slot is already waiting.
     * @return 0.
     */
    virtual int LoadBank(unsigned short nSlot, const String &file, bool bSkipPending) = 0;

    /**
     * Vtable slot 20. Empty a sample bank slot.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nSlot The bank slot.
     * @return 0.
     */
    virtual int UnloadBank(unsigned short nSlot) = 0;

    /**
     * Vtable slot 21. Choose whether the output may invert a channel for surround decoders.
     *
     * The base body is empty. The name is inferred.
     *
     * @param bSurround Whether the inversion is allowed.
     * @ghidraAddress NTSC-U/C: 0x00395fb8
     * @ghidraAddress PAL: 0x00404638
     */
    virtual void SetSurround(bool bSurround);

    /**
     * Vtable slot 22. Choose mono output.
     *
     * The base body is empty. The name is inferred.
     *
     * @param bMono Whether the output is mono.
     * @ghidraAddress NTSC-U/C: 0x00395fc0
     * @ghidraAddress PAL: 0x00404640
     */
    virtual void SetMono(bool bMono);

    /**
     * Vtable slot 23. Set the lag of the output, the value of the `lag_ms` entry of the `synth`
     * section.
     *
     * The base body is empty. The name is inferred.
     *
     * @param fLag The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00395fc8
     * @ghidraAddress PAL: 0x00404648
     */
    virtual void SetLag(float fLag);

    /**
     * Vtable slot 24. Report the lag slot 23 set.
     *
     * The base body returns 0. The name is inferred.
     *
     * @return The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00395fd0
     * @ghidraAddress PAL: 0x00404650
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
     * @ghidraAddress PAL: 0x00404660
     */
    virtual void SetSoftFxSweep(float fPosition);

    /**
     * Vtable slot 26. Set the second control of the software effect.
     *
     * The base body is empty. AxeVoice passes the vertical position of the stick. The name is
     * inferred.
     *
     * @param fValue The value, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x00395fe8
     * @ghidraAddress PAL: 0x00404668
     */
    virtual void SetSoftFxModulation(float fValue);

    /**
     * Vtable slot 27. Send the settings of the software effect.
     *
     * The base body is empty. The name is inferred.
     *
     * @param filter The settings.
     * @ghidraAddress NTSC-U/C: 0x00395ff0
     * @ghidraAddress PAL: 0x00404670
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
     * The base body is empty. SynthPS2 scales the level by 16383 and sends it to the software
     * effect. The name is inferred.
     *
     * @param fLevel The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00395ff8
     * @ghidraAddress PAL: 0x00404678
     */
    virtual void SetOutputLevel(float fLevel);

    /**
     * Vtable slot 31. Report the output level.
     *
     * The base body returns 1. The name is inferred.
     *
     * @return The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00396000
     * @ghidraAddress PAL: 0x00404680
     */
    virtual float GetOutputLevel();

    /**
     * Vtable slot 32. Choose the mixer input of the sound processor that the software effect
     * captures.
     *
     * The base body is empty. The metagame passes 1 before it plays the intro movie. The name is
     * inferred.
     *
     * @param nInput The input pair, 0 to 2.
     * @ghidraAddress NTSC-U/C: 0x00396010
     * @ghidraAddress PAL: 0x00404690
     */
    virtual void SetSoftFxInput(int nInput);

    /**
     * Vtable slot 33. Set whether the effect buses feed the hardware effects.
     *
     * The base body is empty. The name is inferred.
     *
     * @param nOn Whether the buses feed the hardware effects.
     * @ghidraAddress NTSC-U/C: 0x00396018
     * @ghidraAddress PAL: 0x00404698
     */
    virtual void SetBusToCoreFx(int nOn);

    /**
     * Vtable slot 34. Set whether the effect buses feed the software effect.
     *
     * The base body is empty, and SynthPS2 overrides it with an empty body. The name is
     * inferred.
     *
     * @param nOn Whether the buses feed the software effect.
     * @ghidraAddress NTSC-U/C: 0x00396020
     * @ghidraAddress PAL: 0x004046a0
     */
    virtual void SetBusToSoftFx(int nOn);

    /**
     * Vtable slot 35. The base body is empty, and the purpose is not yet recovered.
     *
     * @ghidraAddress NTSC-U/C: 0x00396028
     * @ghidraAddress PAL: 0x004046a8
     */
    virtual void VirtualSlot35();

    /**
     * Vtable slot 36. Queue a job that sends packed messages.
     *
     * The base body is empty. The name is inferred.
     *
     * @param messages The messages, each packed as SendPackedMessage() takes it.
     * @ghidraAddress NTSC-U/C: 0x00396030
     * @ghidraAddress PAL: 0x004046b0
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
     * @ghidraAddress PAL: 0x004046b8
     */
    virtual void QueueCallback(Callback pfnCallback, int nArg);
};

/**
 * The sound output Synth::Create() built, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0a84
 * @ghidraAddress PAL: 0x0042f91c
 */
extern Synth *TheSynth;
