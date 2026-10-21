#pragma once

#include <list>
#include <memory>
#include <stddef.h>
#include <vector>

#include "os/bufstream.h"
#include "os/mem.h"
#include "os/string.h"
#include "synth/lfo.h"
#include "synth/onepolefilter.h"
#include "synth/queuedjob.h"
#include "synth/softfxfilter.h"
#include "synth/synth.h"

class BankFileIO;

/**
 * Sound output of the console, a client of the `synth` and `softfx` IOP modules.
 *
 * The RTTI includes the class name and records Synth as the base. The object is 0x20180 bytes and
 * is 64-byte aligned. Each frame, Poll() runs the queued jobs, advances the bank load, and sends
 * the commands collected in the two command buffers to the modules, each buffer as one
 * concatenated remote call. The synthesiser module plays the MIDI messages and the banks, and the
 * software effect module runs the software filters and plays the stream of a StreamPlayer.
 */
class SynthPS2 : public Synth {
public:
    /** The number of bank slots of the synthesiser module. */
    static constexpr int kNumBankSlots = 7;

    /** The number of MIDI channels, each with a smoothed volume. */
    static constexpr int kNumChannels = 16;

    /** The size of each command buffer in bytes. */
    static constexpr int kCommandBufferSize = 0x10000;

    /**
     * Allocate the object from the pool heaps under the tag "SynthPs2".
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "SynthPs2", 0);
    }

    /**
     * Return the object to the pool heaps.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Build the output with every bank slot empty and the command buffers ready.
     *
     * The smoothing of the channel volumes is the `volume_smoothing` entry of the `synth`
     * section.
     *
     * @ghidraAddress NTSC-U/C: 0x0024bdc0
     * @ghidraAddress PAL: 0x00254800
     */
    SynthPS2();

    /**
     * Release the command buffers. The queued jobs are not deleted.
     *
     * @ghidraAddress NTSC-U/C: 0x0024ce18
     * @ghidraAddress PAL: 0x00255858
     */
    ~SynthPS2() override;

    /**
     * Read the lag, bind to the IOP modules, and start them.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024d2e0
     * @ghidraAddress PAL: 0x00255d10
     */
    int Init() override;

    /**
     * Stop the synthesiser module.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024d1b0
     * @ghidraAddress PAL: 0x00255bf0
     */
    int Terminate() override;

    /**
     * Queue a reset of the sound processor.
     *
     * @ghidraAddress NTSC-U/C: 0x00250ea8
     * @ghidraAddress PAL: 0x00259858
     */
    void ResetSpu() override;

    /**
     * Send one packed message.
     *
     * @param nPacked The four message bytes, lowest first.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024ecf8
     * @ghidraAddress PAL: 0x00257708
     */
    int SendPackedMessage(unsigned int nPacked) override;

    /**
     * Send one message, or act on it at once.
     *
     * Control changes 12, 15, and 17 set the sweep of the software filter, the output level, and
     * the target volume of the channel. Every other message joins the MIDI queue with its delay
     * clamped to a byte.
     *
     * @param nByte0 The status byte.
     * @param nByte1 The first data byte.
     * @param nByte2 The second data byte.
     * @param nByte3 The delay in milliseconds.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024ea68
     * @ghidraAddress PAL: 0x00257478
     */
    int SendMessage(unsigned char nByte0,
                    unsigned char nByte1,
                    unsigned char nByte2,
                    int nByte3) override;

    /**
     * Read the system clock and shorten the delays of the queued messages by the time passed.
     *
     * @ghidraAddress NTSC-U/C: 0x0024ed58
     * @ghidraAddress PAL: 0x00257768
     */
    void UpdateTime() override;

    /**
     * Run the frame's work and send the command buffers to the IOP modules.
     *
     * A module still busy with the previous frame's call is skipped.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024ee10
     * @ghidraAddress PAL: 0x00257820
     */
    int Poll() override;

    /**
     * Report the stream SetStream() set.
     *
     * @return The stream, or null.
     * @ghidraAddress NTSC-U/C: 0x00396a18
     * @ghidraAddress PAL: 0x00405100
     */
    StreamPlayer *GetStream() override {
        return mStream;
    }

    /**
     * Set the stream the output plays, and end the stream the software effect played.
     *
     * @param pStream The stream, or null.
     * @ghidraAddress NTSC-U/C: 0x0024faa0
     * @ghidraAddress PAL: 0x00258490
     */
    void SetStream(StreamPlayer *pStream) override;

    /**
     * Queue a run of stream data for the software effect and clear the room it reported.
     *
     * @param pData The data.
     * @param nSize The size of the data in bytes. 0 ends the stream.
     * @ghidraAddress NTSC-U/C: 0x0024fae0
     * @ghidraAddress PAL: 0x002584d0
     */
    void SendStreamData(const void *pData, int nSize) override;

    /**
     * Report the room the software effect has for stream data.
     *
     * @return The room in bytes.
     * @ghidraAddress NTSC-U/C: 0x00250d98
     * @ghidraAddress PAL: 0x00259748
     */
    int GetStreamStatus() override;

    /**
     * Report the format of the stream data.
     *
     * @param pnRate Receives 28351.
     * @param pnBlock Receives 64.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024f968
     * @ghidraAddress PAL: 0x00258358
     */
    int GetStreamFormat(int *pnRate, int *pnBlock) override;

    /**
     * Queue the division of the sound memory among the bank slots.
     *
     * @param blocks The size of each bank slot in 64-byte blocks.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024f9b8
     * @ghidraAddress PAL: 0x002583a8
     */
    int Partition(const std::vector<int> &blocks) override;

    /**
     * Report the size of the audio data file of a bank in 64-byte blocks, rounded up.
     *
     * @param file The bank file.
     * @return The number of blocks.
     * @ghidraAddress NTSC-U/C: 0x0024e4f8
     * @ghidraAddress PAL: 0x00256f28
     */
    int GetBankBlockCount(const String &file) override;

    /**
     * Report whether a slot has a bank and no load into it is waiting.
     *
     * @param nSlot The bank slot.
     * @return Whether the bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x0024cfd0
     * @ghidraAddress PAL: 0x00255a10
     */
    bool IsBankLoaded(unsigned short nSlot) override;

    /**
     * Report whether a slot has a bank or a load into it is waiting.
     *
     * @param nSlot The bank slot.
     * @return 1 when the slot has a bank or a pending load, otherwise 0.
     * @ghidraAddress NTSC-U/C: 0x0024d0b8
     * @ghidraAddress PAL: 0x00255af8
     */
    int IsBankPresent(unsigned short nSlot) override;

    /**
     * Queue the load of a bank file into a slot.
     *
     * @param nSlot The bank slot.
     * @param file The bank file.
     * @param bSkipPending Do nothing when a load into the slot is already waiting.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e750
     * @ghidraAddress PAL: 0x00257180
     */
    int LoadBank(unsigned short nSlot, const String &file, bool bSkipPending) override;

    /**
     * Empty a slot, or cancel the load into it.
     *
     * A load that is under way is aborted and the slot emptied after it. A load still waiting is
     * taken off the queue, and the job is not deleted.
     *
     * @param nSlot The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e0b0
     * @ghidraAddress PAL: 0x00256ae0
     */
    int UnloadBank(unsigned short nSlot) override;

    /**
     * Choose whether the synthesiser may invert a channel for surround decoders.
     *
     * @param bSurround Whether the inversion is allowed.
     * @ghidraAddress NTSC-U/C: 0x00250db8
     * @ghidraAddress PAL: 0x00259768
     */
    void SetSurround(bool bSurround) override;

    /**
     * Choose mono output for the synthesiser and the stream.
     *
     * @param bMono Whether the output is mono.
     * @ghidraAddress NTSC-U/C: 0x00250df0
     * @ghidraAddress PAL: 0x002597a0
     */
    void SetMono(bool bMono) override;

    /**
     * Set the lag of the output.
     *
     * @param fLag The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0024d2d0
     * @ghidraAddress PAL: 0x00255d00
     */
    void SetLag(float fLag) override;

    /**
     * Report the lag of the output.
     *
     * @return The lag in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0024d2d8
     * @ghidraAddress PAL: 0x00255d08
     */
    float GetLag() override;

    /**
     * Set the position of the sweep of a sweeping software filter.
     *
     * Another filter type is reported and the position is not changed.
     *
     * @param fPosition The position, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x0024fbe8
     * @ghidraAddress PAL: 0x002585c8
     */
    void SetSoftFxSweep(float fPosition) override;

    /**
     * Set the second control of the software effect.
     *
     * @param fValue The value, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x0024fbd8
     * @ghidraAddress PAL: 0x002585b8
     */
    void SetSoftFxModulation(float fValue) override;

    /**
     * Select the software effect and record its settings.
     *
     * The output level returns to full. A sweeping filter starts its sweep at the ratio of its
     * initial frequency to its ceiling, and a chorus restarts the oscillators of its taps.
     *
     * @param filter The settings.
     * @ghidraAddress NTSC-U/C: 0x0024fc58
     * @ghidraAddress PAL: 0x00258638
     */
    void SetSoftFxFilter(const SoftFxFilter &filter) override;

    /**
     * Queue a job that turns the software effects on.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024f630
     * @ghidraAddress PAL: 0x00258020
     */
    int EnableSoftFx() override;

    /**
     * Queue a job that turns the software effects off.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024f8a0
     * @ghidraAddress PAL: 0x00258290
     */
    int DisableSoftFx() override;

    /**
     * Set the volume of the input the software effect captures.
     *
     * @param fLevel The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00250c10
     * @ghidraAddress PAL: 0x002595c0
     */
    void SetOutputLevel(float fLevel) override;

    /**
     * Report the level SetOutputLevel() set.
     *
     * @return The level, 1 for full.
     * @ghidraAddress NTSC-U/C: 0x00250cb0
     * @ghidraAddress PAL: 0x00259660
     */
    float GetOutputLevel() override;

    /**
     * Choose the mixer input of the sound processor that the software effect captures.
     *
     * @param nInput The input pair, 0 to 2.
     * @ghidraAddress NTSC-U/C: 0x002508f0
     * @ghidraAddress PAL: 0x002592a0
     */
    void SetSoftFxInput(int nInput) override;

    /**
     * Set whether the voices feed the hardware effects.
     *
     * @param nOn Whether the voices feed the hardware effects.
     * @ghidraAddress NTSC-U/C: 0x00250970
     * @ghidraAddress PAL: 0x00259320
     */
    void SetBusToCoreFx(int nOn) override;

    /**
     * Do nothing.
     *
     * @param nOn Unused.
     * @ghidraAddress NTSC-U/C: 0x002509f0
     * @ghidraAddress PAL: 0x002593a0
     */
    void SetBusToSoftFx(int nOn) override;

    /**
     * Queue a job that sends packed messages to set up the hardware effects.
     *
     * @param messages The messages.
     * @ghidraAddress NTSC-U/C: 0x002509f8
     * @ghidraAddress PAL: 0x002593a8
     */
    void SendMessages(const std::vector<unsigned int> &messages) override;

    /**
     * Queue a job that calls a routine once the jobs before it have run.
     *
     * @param pfnCallback The routine.
     * @param nArg The argument passed to the routine.
     * @ghidraAddress NTSC-U/C: 0x0024f700
     * @ghidraAddress PAL: 0x002580f0
     */
    void QueueCallback(Callback pfnCallback, int nArg) override;

private:
    /** Function numbers of the synthesiser module's remote calls. */
    enum Command {
        kCommandInit = 100,            /*!< Start the module. */
        kCommandTerminate = 101,       /*!< Stop the module. */
        kCommandSetBankLayout = 102,   /*!< Divide the sound memory among the slots. */
        kCommandSetBankLoaded = 103,   /*!< Mark a slot loaded. */
        kCommandUnloadBank = 104,      /*!< Empty a slot. */
        kCommandTransferSamples = 105, /*!< Waveform data of a bank. */
        kCommandLoadBankHeader = 106,  /*!< The `BANK` chunk of a bank. */
        kCommandLoadPrograms = 107,    /*!< The `INST` chunk of a bank. */
        kCommandLoadSampleDescs = 108, /*!< The `SDES` chunk of a bank. */
        kCommandLoadSamples = 109,     /*!< The `SAMP` chunk of a bank. */
        kCommandQueueEvents = 110,     /*!< MIDI messages stamped with delays. */
        kCommandConcatenation = 113,   /*!< The commands of a command buffer. */
        kCommandMono = 114,            /*!< Choose mono output. */
        kCommandSurround = 115,        /*!< Allow the surround inversion. */
        kCommandResetSpu = 116,        /*!< Initialise the sound processor again. */
    };

    /** Function numbers of the software effect module's remote calls. */
    enum SoftFxCommand {
        kSoftFxCommandInit = 100,          /*!< Start the module. */
        kSoftFxCommandStart = 102,         /*!< Start capturing. */
        kSoftFxCommandStop = 103,          /*!< Stop capturing. */
        kSoftFxCommandStreamData = 105,    /*!< Append stream data. */
        kSoftFxCommandSetEffect = 106,     /*!< Select an effect. */
        kSoftFxCommandSetParams = 107,     /*!< The parameters of both channels. */
        kSoftFxCommandConcatenation = 108, /*!< The commands of a command buffer. */
        kSoftFxCommandRouteInput = 109,    /*!< Select the mixer input. */
        kSoftFxCommandRouteVoices = 110,   /*!< Route the voices to the hardware effects. */
        kSoftFxCommandSetVolume = 111,     /*!< Set the captured input volume. */
        kSoftFxCommandSetMonoStream = 112, /*!< Mix the stream to mono. */
    };

    /** The states of the bank load PollBankLoad() advances, the values of mLoadState. */
    enum LoadState {
        kLoadIdle = 0,          /*!< No load is under way. */
        kLoadBankHeader = 1,    /*!< The `BANK` chunk is read and sent. */
        kFindPrograms = 2,      /*!< The search for the `INST` chunk starts. */
        kLoadPrograms = 3,      /*!< The `INST` chunk is read and sent. */
        kFindSamples = 4,       /*!< The search for the `SAMP` chunk starts. */
        kLoadSampleDescs = 5,   /*!< The `SDES` chunk is read and sent. */
        kOpenAudioData = 6,     /*!< The audio data file opens. */
        kLoadSamples = 7,       /*!< The `SAMP` chunk is read and sent. */
        kFindSampleDescs = 8,   /*!< The search for the `SDES` chunk starts. */
        kTransferAudioData = 9, /*!< The audio data is read and sent. */
        kMarkLoaded = 10,       /*!< The slot is marked loaded. */
        kFinishLoad = 11,       /*!< The load completes. */
        kAbortLoad = 12,        /*!< The load is abandoned. */
        kNumLoadStates = 13,    /*!< The number of states. */
    };

    /** The header of every bank loading command, as the module reads it. */
    struct BankDataCommand {
        int mBank;   /*!< The bank slot. */
        int mOffset; /*!< The destination offset of waveform data. */
    };

    /** A command buffer, whose first bytes SendBankData() also overwrites with its header. */
    union CommandBuffer {
        BankDataCommand mLastHeader;     /*!< The header SendBankData() last built. */
        char mBytes[kCommandBufferSize]; /*!< The commands. */
    };

    /** The number of delay taps of the software echo. */
    static constexpr int kNumTaps = SoftFxFilter::kNumTaps;

    /** One tap of the software echo, as the module reads it. */
    struct SoftFxTap {
        int mDelay; /*!< The delay in samples. */
        int mGain;  /*!< The gain in Q13. */
    };

    /** The parameters of the software effect for one channel, as the module reads them. */
    struct SoftFxParams {
        int mMode;                 /*!< The ladder filter mode. */
        int mCutoff;               /*!< The ladder filter cutoff in Q13. */
        int mLevel;                /*!< The filter frequency, echo level, or stutter rate. */
        int mFeedback;             /*!< The filter damping or echo feedback in Q13. */
        SoftFxTap mTaps[kNumTaps]; /*!< The taps of the echo. */
    };

    /**
     * Read the room the software effect reported for stream data.
     *
     * @return The room in bytes.
     * @ghidraAddress NTSC-U/C: 0x0024bd50
     * @ghidraAddress PAL: 0x00254790
     */
    int ReadStreamStatus();

    /**
     * Clear the room the software effect reported for stream data.
     *
     * @ghidraAddress NTSC-U/C: 0x0024bd90
     * @ghidraAddress PAL: 0x002547d0
     */
    static void ClearStreamStatus();

    /**
     * Append a command to the synthesiser's command buffer.
     *
     * @param nCommand The function number, one of Command.
     * @param pData The argument.
     * @param nSize The size of the argument in bytes.
     * @ghidraAddress NTSC-U/C: 0x0024d200
     * @ghidraAddress PAL: 0x00255c40
     */
    void SendCommand(int nCommand, const void *pData, int nSize);

    /**
     * Bind the clients of both modules, waiting for each module to appear, and start them.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024d360
     * @ghidraAddress PAL: 0x00255d90
     */
    int InitIop();

    /**
     * Move the volume of each channel a step toward its target, and queue a volume change for
     * each channel that moved by more than half a step.
     *
     * @ghidraAddress NTSC-U/C: 0x0024d5c8
     * @ghidraAddress PAL: 0x00255ff8
     */
    void UpdateVolumes();

    /**
     * Report whether the last call to the synthesiser module is still in flight.
     *
     * @return Whether the module is busy.
     * @ghidraAddress NTSC-U/C: 0x0024d7f8
     * @ghidraAddress PAL: 0x00256228
     */
    bool IsRpcBusy();

    /**
     * Report whether the last call to the software effect module is still in flight.
     *
     * @return Whether the module is busy.
     * @ghidraAddress NTSC-U/C: 0x0024d840
     * @ghidraAddress PAL: 0x00256270
     */
    bool IsSoftFxRpcBusy();

    /**
     * Report whether the synthesiser module finished the last waveform transfer.
     *
     * @return Whether the transfer is done.
     * @ghidraAddress NTSC-U/C: 0x0024d888
     * @ghidraAddress PAL: 0x002562b8
     */
    bool IsTransferDone();

    /**
     * Mark a waveform transfer started, for the synthesiser module to clear.
     *
     * @ghidraAddress NTSC-U/C: 0x0024d8d0
     * @ghidraAddress PAL: 0x00256300
     */
    void SetTransferPending();

    /**
     * Report whether the synthesiser module finished the last update of the hardware effects.
     *
     * @return Whether the update is done.
     * @ghidraAddress NTSC-U/C: 0x0024d900
     * @ghidraAddress PAL: 0x00256330
     */
    bool IsCoreFxDone();

    /**
     * Mark an update of the hardware effects started, for the synthesiser module to clear.
     *
     * @ghidraAddress NTSC-U/C: 0x0024d948
     * @ghidraAddress PAL: 0x00256378
     */
    void SetCoreFxPending();

    /**
     * Advance the bank load by one state, or more while each state completes at once.
     *
     * @return 0 when no load is under way, otherwise 1.
     * @ghidraAddress NTSC-U/C: 0x0024d978
     * @ghidraAddress PAL: 0x002563a8
     */
    int PollBankLoad();

    /**
     * Queue the command that marks a slot loaded.
     *
     * @param nSlot The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024dfa0
     * @ghidraAddress PAL: 0x002569d0
     */
    int SendBankLoaded(unsigned short nSlot);

    /**
     * Queue the command that empties a slot.
     *
     * @param nSlot The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e080
     * @ghidraAddress PAL: 0x00256ab0
     */
    int SendUnloadBank(unsigned short nSlot);

    /**
     * Open a bank file and start reading its `BANK` chunk.
     *
     * @param nSlot The bank slot.
     * @param file The bank file.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e5b0
     * @ghidraAddress PAL: 0x00256fe0
     */
    int StartBankLoad(unsigned short nSlot, const String &file);

    /**
     * Queue a bank loading command with its header and data.
     *
     * @param data The data.
     * @param nBank The bank slot.
     * @param nCommand The function number, one of Command.
     * @param nOffset The destination offset of waveform data.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e890
     * @ghidraAddress PAL: 0x002572c0
     */
    int SendBankData(const std::vector<unsigned char> &data, int nBank, int nCommand, int nOffset);

    /**
     * Queue the `BANK` chunk of a bank.
     *
     * @param data The chunk.
     * @param nBank The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e9c8
     * @ghidraAddress PAL: 0x002573d8
     */
    int SendBankHeader(const std::vector<unsigned char> &data, int nBank);

    /**
     * Queue the `INST` chunk of a bank.
     *
     * @param data The chunk.
     * @param nBank The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024e9f0
     * @ghidraAddress PAL: 0x00257400
     */
    int SendPrograms(const std::vector<unsigned char> &data, int nBank);

    /**
     * Queue the `SDES` chunk of a bank.
     *
     * @param data The chunk.
     * @param nBank The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024ea18
     * @ghidraAddress PAL: 0x00257428
     */
    int SendSampleDescs(const std::vector<unsigned char> &data, int nBank);

    /**
     * Queue the `SAMP` chunk of a bank.
     *
     * @param data The chunk.
     * @param nBank The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024ea40
     * @ghidraAddress PAL: 0x00257450
     */
    int SendSamples(const std::vector<unsigned char> &data, int nBank);

    /**
     * Mark a waveform transfer started and queue a run of the audio data of a bank.
     *
     * @param data The audio data.
     * @param nOffset The offset of the data in the bank's waveform area.
     * @param nBank The bank slot.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396f50
     * @ghidraAddress PAL: 0x00405638
     */
    int SendSampleData(const std::vector<unsigned char> &data, int nOffset, int nBank) {
        SetTransferPending();
        (void)SendBankData(data, nBank, kCommandTransferSamples, nOffset);
        return 0;
    }

    /**
     * Run or poll the job at the front of the queue, and finish it once it is done.
     *
     * @ghidraAddress NTSC-U/C: 0x0024f028
     * @ghidraAddress PAL: 0x00257a18
     */
    void ProcessJobs();

    /**
     * Add an amount to the delay of each queued message, clamping each delay to a byte.
     *
     * @param nBias The amount in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0024f490
     * @ghidraAddress PAL: 0x00257e80
     */
    void AdjustDelays(int nBias);

    /**
     * Move the MIDI queue into the synthesiser's command buffer, adding the lag less the time
     * since the last UpdateTime() to each delay.
     *
     * @return 1 when the queue was moved, otherwise 0.
     * @ghidraAddress NTSC-U/C: 0x0024f520
     * @ghidraAddress PAL: 0x00257f10
     */
    int FlushMidi();

    /**
     * Queue the command that starts or stops the software effects, and record the choice.
     *
     * @param nEnable 1 to start the software effects, 0 to stop them.
     * @ghidraAddress NTSC-U/C: 0x0024f7d8
     * @ghidraAddress PAL: 0x002581c8
     */
    void SendSoftFxEnable(int nEnable);

    /**
     * Queue the division of the sound memory among the bank slots.
     *
     * @param blocks The size of each bank slot in 64-byte blocks.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0024f980
     * @ghidraAddress PAL: 0x00258370
     */
    int SendPartition(const std::vector<int> &blocks);

    /**
     * Poll the stream SetStream() set, when there is one.
     *
     * @ghidraAddress NTSC-U/C: 0x0024fba8
     * @ghidraAddress PAL: 0x00258588
     */
    void PollStream();

    /**
     * Send the parameters of the software effect for this frame.
     *
     * Nothing is sent while the software effects are off.
     *
     * @ghidraAddress NTSC-U/C: 0x0024ff40
     * @ghidraAddress PAL: 0x00258920
     */
    void UpdateSoftFx();

    /**
     * Send each of a list of packed messages.
     *
     * @param messages The messages.
     * @ghidraAddress NTSC-U/C: 0x00250ae0
     * @ghidraAddress PAL: 0x00259490
     */
    void SendMessageList(const std::vector<unsigned int> &messages);

    /**
     * Set the sweep of the software filter from a controller value.
     *
     * @param nValue The controller value, 0 to 127.
     * @ghidraAddress NTSC-U/C: 0x00250b70
     * @ghidraAddress PAL: 0x00259520
     */
    void SetSweepFromController(unsigned char nValue);

    /**
     * Set the output level from a controller value.
     *
     * @param nValue The controller value, 0 to 127.
     * @ghidraAddress NTSC-U/C: 0x00250bc8
     * @ghidraAddress PAL: 0x00259578
     */
    void SetLevelFromController(unsigned char nValue);

    /**
     * Report whether a load into a slot is waiting in the queue.
     *
     * @param nSlot The bank slot.
     * @return Whether a BankLoadQJ for the slot is queued.
     * @ghidraAddress NTSC-U/C: 0x00250cc0
     * @ghidraAddress PAL: 0x00259670
     */
    bool HasPendingLoad(unsigned short nSlot);

    std::vector<bool> mBankLoaded;               /*!< Whether each slot has a bank. */
    std::list<std::unique_ptr<QueuedJob>> mJobs; /*!< The queued jobs, in order. */
    int mLoadSlot;                               /*!< The slot of the load under way, or -1. */
    std::vector<unsigned char> mChunk;           /*!< The chunk of the bank being loaded. */
    std::unique_ptr<BankFileIO> mBankFile;       /*!< The bank file being loaded, or null. */
    int mLoadState;                              /*!< One of LoadState. */
    std::vector<unsigned int> mMidiQueue;        /*!< The packed messages waiting to be sent. */
    int mInitialized;                            /*!< 1 once Init() ran. */
    float mLagMs;                                /*!< The lag of the output in milliseconds. */
    float mNowMs;                                /*!< The system time of the last UpdateTime(). */
    float mLastMs;                               /*!< The system time of the UpdateTime() before. */
    int mTimeUpdated;                     /*!< 1 when UpdateTime() ran since the last Poll(). */
    std::unique_ptr<BufStream> mCommands; /*!< The writer of mCommandBuffer. */
    std::unique_ptr<BufStream> mSoftFxCommands; /*!< The writer of mSoftFxBuffer. */
    alignas(64) CommandBuffer mCommandBuffer;   /*!< The commands for the synthesiser. */
    CommandBuffer mSoftFxBuffer;                /*!< The commands for the software effect. */
    StreamPlayer *mStream;   /*!< The stream that plays, or null. Belongs to its StreamTask. */
    int mSoftFxEnabled;      /*!< 1 while the software effects are on. */
    float mSoftFxSweep;      /*!< The sweep of the software filter. */
    float mSoftFxModulation; /*!< The second control of the software effect. */
    float mOutputLevel;      /*!< The level SetOutputLevel() set. */
    SoftFxFilter mFilter;    /*!< The settings of the software effect. */
    Lfo mTapOscillators[kNumTaps];              /*!< The oscillators of the echo taps. */
    std::vector<OnePoleFilter> mChannelVolumes; /*!< The smoothed volume of each channel. */
    int mGotAudioData;                     /*!< Whether the last read of audio data completed. */
    int mMoreAudioData;                    /*!< Whether more audio data is to come. */
    int mAudioDataOffset;                  /*!< The offset of the audio data last read. */
    std::vector<unsigned char> mAudioData; /*!< The audio data last read. */
};
