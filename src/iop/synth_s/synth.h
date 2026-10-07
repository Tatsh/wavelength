#pragma once

#include <libsd.h>

#include "synth_s/banksampledesc.h"
#include "synth_s/channel.h"
#include "synth_s/midievent.h"
#include "synth_s/voice.h"

/**
 * The MIDI synthesiser that drives the SPU2 voices.
 *
 * The class has no RTTI, and its name is inferred from the module's name. Every member is static,
 * and the module has one synthesiser. The EE sends MIDI over RPC to play at once or stamped with a
 * delay. The tick thread runs every six milliseconds and plays delayed MIDI when it falls due.
 * Notes key on and off in batches. A pass over incoming MIDI collects the key masks, and
 * CommitKeys() writes them together.
 */
class Synth {
public:
    static constexpr int kNumChannels = 16;      /*!< MIDI channels. */
    static constexpr int kNumCores = 2;          /*!< SPU2 cores. */
    static constexpr int kVoicesPerCore = 24;    /*!< Voices of each core. */
    static constexpr int kMaxRetiredVoices = 50; /*!< Retired voice records. */

    /**
     * The tick thread. It sleeps until the timer wakes it, then advances the voices and the
     * queued MIDI and posts pending notifications to the EE.
     *
     * @ghidraAddress NTSC-U/C: 0x00000704
     * @ghidraAddress PAL: 0x00000704
     */
    static void TickThread();

    /**
     * Act on one MIDI channel message. System messages are ignored.
     *
     * @param status Status byte.
     * @param data1 First data byte.
     * @param data2 Second data byte.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000a10
     * @ghidraAddress PAL: 0x00000a10
     */
    static int ProcessMidiMessage(unsigned char status, unsigned char data1, unsigned char data2);

    /**
     * Act on every queued MIDI message that is due, as one batch of key changes.
     *
     * @ghidraAddress NTSC-U/C: 0x00000fd8
     * @ghidraAddress PAL: 0x00000fd8
     */
    static void ProcessDueEvents();

    /**
     * Queue MIDI messages stamped with delays from now.
     *
     * @param events The messages, whose fourth byte is the delay in milliseconds.
     * @param count Number of messages.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x000010dc
     * @ghidraAddress PAL: 0x000010dc
     */
    static int QueueTimedEvents(const MidiEvent *events, unsigned int count);

    /**
     * Act on a MIDI byte stream with running status. The stream ends at its size or at a system
     * message. A note-on of velocity zero is treated as a note-off.
     *
     * @param data The stream.
     * @param size Its size in bytes.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00001268
     * @ghidraAddress PAL: 0x00001268
     */
    static int ProcessMidiStream(const unsigned char *data, unsigned int size);

    /**
     * Select mono output. Mono output centres every later note.
     *
     * @param mono True for mono.
     * @ghidraAddress NTSC-U/C: 0x00002514
     * @ghidraAddress PAL: 0x00002514
     */
    static void SetMono(bool mono);

    /**
     * Allow or forbid the surround inversion of the descriptors that request it.
     *
     * @param enabled True to allow it.
     * @ghidraAddress NTSC-U/C: 0x00002520
     * @ghidraAddress PAL: 0x00002520
     */
    static void SetSurround(bool enabled);

    /**
     * Restore a channel's controllers to their defaults, except its monophonic switch.
     *
     * @param channel The channel.
     * @ghidraAddress NTSC-U/C: 0x00002808
     * @ghidraAddress PAL: 0x00002808
     */
    static void ResetAllControllers(unsigned char channel);

    /**
     * Reset the banks, channels, and voices, then the SPU2.
     *
     * @ghidraAddress NTSC-U/C: 0x000028dc
     * @ghidraAddress PAL: 0x000028dc
     */
    static void Init();

    /**
     * Initialise the SPU2: the mixer, the reverb areas and settings, and the voice mix.
     *
     * @ghidraAddress NTSC-U/C: 0x00002a3c
     * @ghidraAddress PAL: 0x00002a3c
     */
    static void ResetSpu();

    /**
     * Combine three pan offsets around the centre, clamped to zero to 127.
     *
     * @param channelPan Channel pan.
     * @param programPan Program pan.
     * @param samplePan Descriptor pan.
     * @return The combined pan.
     * @ghidraAddress NTSC-U/C: 0x00003070
     * @ghidraAddress PAL: 0x00003070
     */
    static int
    CombinePan(unsigned char channelPan, unsigned char programPan, unsigned char samplePan);

    /**
     * Report whether a channel has a program it can play.
     *
     * @param channel The channel.
     * @return True when the channel's program slot has a program.
     * @ghidraAddress NTSC-U/C: 0x000034c4
     * @ghidraAddress PAL: 0x000034c4
     */
    static bool ChannelHasProgram(unsigned char channel);

    /**
     * Start a note on every descriptor of the channel's program whose key range covers it.
     *
     * @param note Note number.
     * @param velocity Velocity; zero stops the note instead.
     * @param channel The channel.
     * @ghidraAddress NTSC-U/C: 0x00003574
     * @ghidraAddress PAL: 0x00003574
     */
    static void NoteOn(unsigned char note, unsigned char velocity, unsigned char channel);

    /**
     * Stop the oldest sounding instance of a note.
     *
     * @param note Note number.
     * @param channel The channel.
     * @ghidraAddress NTSC-U/C: 0x00003d38
     * @ghidraAddress PAL: 0x00003d38
     */
    static void NoteOff(unsigned char note, unsigned char channel);

    /**
     * Point a channel at the program of its bank with a program number.
     *
     * @param channel The channel.
     * @param program Program number; no match clears the channel's program.
     * @ghidraAddress NTSC-U/C: 0x00004050
     * @ghidraAddress PAL: 0x00004050
     */
    static void SelectProgram(unsigned short channel, unsigned short program);

    /**
     * Act on a program change unless the channel already plays that program number.
     *
     * @param channel The channel.
     * @param program Program number.
     * @ghidraAddress NTSC-U/C: 0x00004148
     * @ghidraAddress PAL: 0x00004148
     */
    static void ProgramChange(unsigned char channel, unsigned short program);

    /**
     * Point a channel at a loaded bank, retaining its program number.
     *
     * @param channel The channel.
     * @param bank Bank identifier.
     * @ghidraAddress NTSC-U/C: 0x000041ec
     * @ghidraAddress PAL: 0x000041ec
     */
    static void BankSelect(unsigned char channel, unsigned short bank);

    /**
     * Rewrite the volume registers of a channel's sounding voices.
     *
     * @param channel The channel.
     * @ghidraAddress NTSC-U/C: 0x00004418
     * @ghidraAddress PAL: 0x00004418
     */
    static void UpdateChannelVolume(unsigned char channel);

    /**
     * Set a channel's pan.
     *
     * @param channel The channel.
     * @param pan Pan, where 64 is the centre.
     * @ghidraAddress NTSC-U/C: 0x00004554
     * @ghidraAddress PAL: 0x00004554
     */
    static void SetPan(unsigned char channel, unsigned char pan);

    /**
     * Set a channel's volume.
     *
     * @param channel The channel.
     * @param volume Volume.
     * @ghidraAddress NTSC-U/C: 0x000045c0
     * @ghidraAddress PAL: 0x000045c0
     */
    static void SetVolume(unsigned char channel, unsigned char volume);

    /**
     * Set a channel's expression.
     *
     * @param channel The channel.
     * @param expression Expression.
     * @ghidraAddress NTSC-U/C: 0x0000462c
     * @ghidraAddress PAL: 0x0000462c
     */
    static void SetExpression(unsigned char channel, unsigned char expression);

    /**
     * Set a channel's pitch bend.
     *
     * @param channel The channel.
     * @param semitones Whole semitones.
     * @param cents Remaining cents.
     * @ghidraAddress NTSC-U/C: 0x00004798
     * @ghidraAddress PAL: 0x00004798
     */
    static void SetPitchBend(unsigned char channel, signed char semitones, signed char cents);

    /**
     * Load a bank's header.
     *
     * @param data The record's size word followed by the record.
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x000048f8
     * @ghidraAddress PAL: 0x000048f8
     */
    static int LoadBankHeader(const unsigned char *data, unsigned short bank);

    /**
     * Load a bank's programs, each record preceded by a size word.
     *
     * @param data The records.
     * @param size Bytes of records.
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004954
     * @ghidraAddress PAL: 0x00004954
     */
    static int LoadPrograms(const unsigned char *data, short size, unsigned short bank);

    /**
     * Load a bank's sample descriptors, each record preceded by a size word. The samples must be
     * loaded first.
     *
     * @param data The records.
     * @param size Bytes of records.
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004a18
     * @ghidraAddress PAL: 0x00004a18
     */
    static int LoadSampleDescs(const unsigned char *data, short size, unsigned short bank);

    /**
     * Load a bank's samples, each record preceded by a size word.
     *
     * @param data The records.
     * @param size Bytes of records.
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004adc
     * @ghidraAddress PAL: 0x00004adc
     */
    static int LoadSamples(const unsigned char *data, short size, unsigned short bank);

    /**
     * Notify the EE at the next tick that a waveform transfer ended.
     *
     * @param channel Transfer channel.
     * @param data Unused.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004ba8
     * @ghidraAddress PAL: 0x00004ba8
     */
    static int TransferDone(int channel, void *data);

    /**
     * Notify the EE at the next tick that a reverb update ended, when the EE requested it.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004be8
     * @ghidraAddress PAL: 0x00004be8
     */
    static int EffectsApplied();

    /**
     * Start a waveform transfer into a bank's SPU2 area.
     *
     * @param data The waveform data.
     * @param size Bytes to transfer.
     * @param offset Destination offset in the bank's area.
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004c18
     * @ghidraAddress PAL: 0x00004c18
     */
    static int
    TransferSampleData(unsigned char *data, int size, unsigned int offset, unsigned short bank);

    /**
     * Place the banks' SPU2 areas one after another.
     *
     * @param data One size word per bank slot, in 64-byte units, or negative to retain the slot's
     * size.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004d4c
     * @ghidraAddress PAL: 0x00004d4c
     */
    static int SetBankSpuLayout(const unsigned char *data);

    /**
     * Mark a bank loaded. Bank selects find only loaded banks.
     *
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004e98
     * @ghidraAddress PAL: 0x00004e98
     */
    static int SetBankLoaded(unsigned short bank);

    /**
     * Silence a bank's voices, detach its channels, and empty it.
     *
     * @param bank Bank slot.
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00004ecc
     * @ghidraAddress PAL: 0x00004ecc
     */
    static int UnloadBank(unsigned short bank);

    /**
     * Set a channel's voice priority.
     *
     * @param channel The channel.
     * @param priority Priority.
     * @ghidraAddress NTSC-U/C: 0x000050a0
     * @ghidraAddress PAL: 0x000050a0
     */
    static void SetPriority(unsigned char channel, unsigned char priority);

    /**
     * Set a channel's detune switch.
     *
     * @param channel The channel.
     * @param detune Nonzero to detune later notes with vibrato.
     * @ghidraAddress NTSC-U/C: 0x000050c4
     * @ghidraAddress PAL: 0x000050c4
     */
    static void SetDetune(unsigned char channel, unsigned char detune);

    /**
     * Set a channel's stereo switch.
     *
     * @param channel The channel.
     * @param stereo Nonzero to play later notes as hard-panned pairs.
     * @ghidraAddress NTSC-U/C: 0x000050ec
     * @ghidraAddress PAL: 0x000050ec
     */
    static void SetStereo(unsigned char channel, unsigned char stereo);

    /**
     * Set a channel's monophonic switch.
     *
     * @param channel The channel.
     * @param monophonic Nonzero to silence the channel before each later note.
     * @ghidraAddress NTSC-U/C: 0x00005114
     * @ghidraAddress PAL: 0x00005114
     */
    static void SetMonophonic(unsigned char channel, unsigned char monophonic);

    /**
     * Switch the live update of one controller on or off for a channel.
     *
     * @param channel The channel.
     * @param controller 10, 224 (pitch bend), 7, 17, or 91 (transposition).
     * @param value At least 64 to switch the update on.
     * @ghidraAddress NTSC-U/C: 0x0000513c
     * @ghidraAddress PAL: 0x0000513c
     */
    static void
    SetControllerEnable(unsigned char channel, unsigned char controller, unsigned char value);

    /**
     * Set a channel's transposition.
     *
     * @param channel The channel.
     * @param semitones Transposition in semitones.
     * @ghidraAddress NTSC-U/C: 0x0000521c
     * @ghidraAddress PAL: 0x0000521c
     */
    static void SetTranspose(unsigned char channel, signed char semitones);

    /**
     * Set a channel's dry and reverb mix for later notes.
     *
     * @param channel The channel.
     * @param busMode A mix mode, or Channel::kBusModeFromSampleDesc.
     * @ghidraAddress NTSC-U/C: 0x0000533c
     * @ghidraAddress PAL: 0x0000533c
     */
    static void SetBusMode(unsigned char channel, unsigned char busMode);

    /**
     * Set a channel's preferred core for later notes.
     *
     * @param channel The channel.
     * @param bus Zero for either core, one for core 0, two for core 1.
     * @ghidraAddress NTSC-U/C: 0x00005360
     * @ghidraAddress PAL: 0x00005360
     */
    static void SetBus(unsigned char channel, unsigned char bus);

    /**
     * Release a channel's notes and forget every retired voice.
     *
     * @param channel The channel.
     * @ghidraAddress NTSC-U/C: 0x00005388
     * @ghidraAddress PAL: 0x00005388
     */
    static void AllNotesOff(unsigned char channel);

    /**
     * The reverb thread. It sleeps until CommitEffects() wakes it, then applies the reverb
     * settings.
     *
     * @ghidraAddress NTSC-U/C: 0x000059b8
     * @ghidraAddress PAL: 0x000059b8
     */
    static void EffectThread();

    /**
     * Set one reverb setting of a core from a controller.
     *
     * @param channel 1 for core 0 or 2 for core 1; other channels are ignored.
     * @param value Controller value.
     * @param param 0 for the mode, 1 and 2 for the left and right depth, 3 for the delay, and 4
     * for the feedback.
     * @ghidraAddress NTSC-U/C: 0x00005a10
     * @ghidraAddress PAL: 0x00005a10
     */
    static void SetEffectParam(unsigned char channel, unsigned char value, unsigned char param);

    /**
     * Key off and then key on every voice the current batch collected.
     *
     * A core whose key-on and key-off masks overlap drops its whole key-off mask.
     *
     * @ghidraAddress NTSC-U/C: 0x00003420
     * @ghidraAddress PAL: 0x00003420
     */
    static void CommitKeys();

    /**
     * Start a batch of key changes.
     *
     * @ghidraAddress NTSC-U/C: 0x00003314
     * @ghidraAddress PAL: 0x00003314
     */
    static void ClearKeyMasks();

    static void *sNotifyAddress;       /*!< EE address a transfer notification goes to. */
    static int sNotifyPending;         /*!< Nonzero while a transfer notification waits. */
    static void *sEffectNotifyAddress; /*!< EE address a reverb notification goes to. */
    static int sEffectNotifyPending;   /*!< Nonzero while a reverb notification waits. */
    static int sEffectThreadId;        /*!< The reverb thread. */
    static unsigned char sInitFlag;    /*!< Flag the EE sends at initialisation; unused. */

    static Channel sChannels[kNumChannels]; /*!< The MIDI channels. */

private:
    /**
     * Copy a voice whose waveform ended into the retired list. A later note-off finds the note
     * there.
     *
     * @param voice The voice.
     * @ghidraAddress NTSC-U/C: 0x00002c40
     * @ghidraAddress PAL: 0x00002c40
     */
    static void RetireVoice(const Voice *voice);

    /**
     * Apply vibrato to every sounding voice that has it and advance the vibrato clock.
     *
     * @ghidraAddress NTSC-U/C: 0x00002d28
     * @ghidraAddress PAL: 0x00002d28
     */
    static void UpdateVibrato();

    /**
     * Free voices whose one-shot waveform ended or whose release faded out.
     *
     * @ghidraAddress NTSC-U/C: 0x00002e74
     * @ghidraAddress PAL: 0x00002e74
     */
    static void UpdateVoiceStates();

    /**
     * Assign a voice to a note, stealing one when none is free.
     *
     * @param core Receives the voice's core.
     * @param voice Receives the voice's index in its core.
     * @param bus -1 for the core with fewer voices in use, otherwise the core to use.
     * @return The voice, or null when neither core has a voice to steal.
     * @ghidraAddress NTSC-U/C: 0x000030b0
     * @ghidraAddress PAL: 0x000030b0
     */
    static Voice *AllocVoice(int *core, int *voice, signed char bus);

    /**
     * Key on voices of both cores.
     *
     * @param core0Mask Voices of core 0, one bit each.
     * @param core1Mask Voices of core 1, one bit each.
     * @ghidraAddress NTSC-U/C: 0x00003338
     * @ghidraAddress PAL: 0x00003338
     */
    static void KeyOn(unsigned int core0Mask, unsigned int core1Mask);

    /**
     * Key off voices of both cores.
     *
     * @param core0Mask Voices of core 0, one bit each.
     * @param core1Mask Voices of core 1, one bit each.
     * @ghidraAddress NTSC-U/C: 0x000033e4
     * @ghidraAddress PAL: 0x000033e4
     */
    static void KeyOff(unsigned int core0Mask, unsigned int core1Mask);

    /**
     * Route a voice to the dry and reverb mixes.
     *
     * @param core The voice's core.
     * @param voice The voice's index in its core.
     * @param mode 0 dry, 1 reverb, 2 both, or 3 neither. Mode 0 applies while reverb settings
     * are being applied.
     * @ghidraAddress NTSC-U/C: 0x00005564
     * @ghidraAddress PAL: 0x00005564
     */
    static void SetVoiceMix(int core, int voice, int mode);

    /**
     * Write the voice mix switches when they changed.
     *
     * @ghidraAddress NTSC-U/C: 0x00005730
     * @ghidraAddress PAL: 0x00005730
     */
    static void CommitVoiceMix();

    /**
     * Wake the reverb thread when the reverb settings changed.
     *
     * @ghidraAddress NTSC-U/C: 0x00005828
     * @ghidraAddress PAL: 0x00005828
     */
    static void CommitEffects();

    /**
     * Apply the reverb settings of both cores, waiting for the hardware.
     *
     * @ghidraAddress NTSC-U/C: 0x00005880
     * @ghidraAddress PAL: 0x00005880
     */
    static void ApplyEffects();

    /**
     * Route a new voice by its channel's mix, or its descriptor's when the channel defers.
     *
     * @param core The voice's core.
     * @param voice The voice's index in its core.
     * @param sampleDesc The voice's descriptor.
     * @param channel The voice's channel.
     * @ghidraAddress NTSC-U/C: 0x000059d8
     * @ghidraAddress PAL: 0x000059d8
     */
    static void
    SetVoiceBus(int core, int voice, const BankSampleDesc *sampleDesc, const Channel *channel);

    static int sTickCount; /*!< Ticks since the voice states were last updated. */
    static Voice sVoices[kNumCores][kVoicesPerCore]; /*!< The voices of each core. */
    static int sRetiredVoiceCount;                   /*!< Retired voice records in use. */
    static Voice sRetiredVoices[kMaxRetiredVoices];  /*!< Retired voice records. */

    static unsigned char sVoiceMixDirty;        /*!< Nonzero when the mix switches changed. */
    static unsigned char sEffectsDirty;         /*!< Nonzero when the reverb settings changed. */
    static unsigned char sMono;                 /*!< Nonzero for mono output. */
    static unsigned char sSurroundDisabled;     /*!< Nonzero to forbid the surround inversion. */
    static unsigned char sEffectsApplying;      /*!< Nonzero while the reverb thread works. */
    static int sVibratoTick;                    /*!< Vibrato clock, zero to 199. */
    static unsigned int sNoteSerial;            /*!< Age the next note-on receives. */
    static int sTransferBusy;                   /*!< Nonzero while the SPU2 transfer is in use. */
    static int sTransferCancel;                 /*!< Nonzero to stop waiting for the handler. */
    static int sVoicesInUse[kNumCores];         /*!< Voices assigned on each core. */
    static unsigned int sKeyOnMask[kNumCores];  /*!< Voices to key on in this batch. */
    static unsigned int sKeyOffMask[kNumCores]; /*!< Voices to key off in this batch. */
    static unsigned int sVoiceMixDryLeft[kNumCores];  /*!< Left dry mix switches. */
    static unsigned int sVoiceMixDryRight[kNumCores]; /*!< Right dry mix switches. */
    static unsigned int sVoiceMixWetLeft[kNumCores];  /*!< Left reverb mix switches. */
    static unsigned int sVoiceMixWetRight[kNumCores]; /*!< Right reverb mix switches. */
    static sceSdEffectAttr sEffectAttr[kNumCores];    /*!< Reverb settings of each core. */
};
