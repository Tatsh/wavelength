#pragma once

/**
 * Function numbers of the synthesiser's RPC server. The names follow kSynthCommandConcatenation,
 * the one name the module records.
 */
enum SynthCommand {
    kSynthCommandInit = 100,            /*!< Start the synthesiser; a SynthInitCommand. */
    kSynthCommandTerminate = 101,       /*!< Stop the tick thread and free the pools. */
    kSynthCommandSetBankLayout = 102,   /*!< Seven size words, one per bank slot. */
    kSynthCommandSetBankLoaded = 103,   /*!< A bank slot as a halfword. */
    kSynthCommandUnloadBank = 104,      /*!< A bank slot as a halfword. */
    kSynthCommandTransferSamples = 105, /*!< A BankDataCommand followed by waveform data. */
    kSynthCommandLoadBankHeader = 106,  /*!< A BankDataCommand followed by the header. */
    kSynthCommandLoadPrograms = 107,    /*!< A BankDataCommand followed by programs. */
    kSynthCommandLoadSampleDescs = 108, /*!< A BankDataCommand followed by descriptors. */
    kSynthCommandLoadSamples = 109,     /*!< A BankDataCommand followed by samples. */
    kSynthCommandQueueEvents = 110,     /*!< MidiEvent messages stamped with delays. */
    kSynthCommandMidi = 112,            /*!< A MIDI byte stream to act on at once. */
    kSynthCommandConcatenation = 113,   /*!< Commands, each after a SynthCommandHeader. */
    kSynthCommandMono = 114,            /*!< A word; one selects mono output. */
    kSynthCommandSurround = 115,        /*!< A word; one allows the surround inversion. */
    kSynthCommandResetSpu = 116,        /*!< Initialise the SPU2 again. */
};

/** The argument of #kSynthCommandInit. */
struct SynthInitCommand {
    void *mNotifyAddress;       /*!< EE address told when a waveform transfer ends. */
    void *mEffectNotifyAddress; /*!< EE address told when a reverb update ends, or null. */
    unsigned char mFlag;        /*!< Recorded and never read. */
};

/** The header of every bank loading command. */
struct BankDataCommand {
    unsigned short mBank; /*!< Bank slot. */
    unsigned int mOffset; /*!< Destination offset in the bank's SPU2 area, for waveform data. */
};

/**
 * The header of each command inside #kSynthCommandConcatenation. Commands follow one another
 * with no padding. A header may sit at any alignment.
 */
struct __attribute__((packed)) SynthCommandHeader {
    int mCommand; /*!< A #SynthCommand. */
    int mSize;    /*!< Bytes of argument after the header. */
};
