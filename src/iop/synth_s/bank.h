#pragma once

#include "synth_s/bankheader.h"
#include "synth_s/bankprogram.h"
#include "synth_s/banksampledesc.h"
#include "synth_s/poolallocator.h"
#include "synth_s/sample.h"

/**
 * An instrument bank: its header, programs, sample descriptors, and samples, and where its
 * waveforms sit in SPU2 memory.
 *
 * The class has no RTTI, and its name is inferred from its role. The EE loads a bank piece by piece
 * over RPC into one of #kMaxBanks slots. Programs, descriptors, and samples come from pools every
 * bank shares.
 */
class Bank {
public:
    static constexpr int kMaxBanks = 7;         /*!< Bank slots. */
    static constexpr int kMaxPrograms = 100;    /*!< Programs per bank, and in the shared pool. */
    static constexpr int kMaxSampleDescs = 700; /*!< Descriptors per bank and in the pool. */
    static constexpr int kMaxSamples = 700;     /*!< Samples per bank and in the pool. */

    /** Construct an empty, unloaded bank. */
    Bank() : mLoaded(0), mSpuAddress(0), mSpuSize(0) {
        mHeader.Init();
        for (int i = kMaxPrograms - 1; i >= 0; --i) {
            mPrograms[i] = nullptr;
        }
        for (int i = kMaxSampleDescs - 1; i >= 0; --i) {
            mSampleDescs[i] = nullptr;
        }
        for (int i = kMaxSamples - 1; i >= 0; --i) {
            mSamples[i] = nullptr;
        }
    }

    /**
     * Reset the header, forget every program, descriptor, and sample, and mark the bank unloaded.
     *
     * The SPU2 area is retained.
     *
     * @ghidraAddress NTSC-U/C: 0x00001efc
     * @ghidraAddress PAL: 0x00001efc
     */
    void Init();

    /**
     * Store a program and point it at its run of descriptors.
     *
     * The run starts after the descriptors of every program before @p index. Those programs must
     * already be stored.
     *
     * @param program The program.
     * @param index Its slot.
     * @ghidraAddress NTSC-U/C: 0x00001f94
     * @ghidraAddress PAL: 0x00001f94
     */
    void SetProgram(BankProgram *program, int index);

    /**
     * Store a descriptor and link it to its sample. The sample must already be stored.
     *
     * @param sampleDesc The descriptor.
     * @param index Its slot.
     * @ghidraAddress NTSC-U/C: 0x00001ff0
     * @ghidraAddress PAL: 0x00001ff0
     */
    void SetSampleDesc(BankSampleDesc *sampleDesc, int index);

    /**
     * Store a sample.
     *
     * @param sample The sample.
     * @param index Its slot.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x0000201c
     * @ghidraAddress PAL: 0x0000201c
     */
    bool SetSample(Sample *sample, int index);

    /**
     * Look up a program by slot.
     *
     * @param index The slot.
     * @return The program, or null when @p index is not below the header's program count.
     * @ghidraAddress NTSC-U/C: 0x00002030
     * @ghidraAddress PAL: 0x00002030
     */
    BankProgram *GetProgram(int index);

    /**
     * Look up one descriptor of a program's run.
     *
     * @param program The program.
     * @param index Position in the program's run.
     * @return The descriptor.
     * @ghidraAddress NTSC-U/C: 0x000020a0
     * @ghidraAddress PAL: 0x000020a0
     */
    BankSampleDesc *GetSampleDesc(BankProgram *program, int index);

    /**
     * Return every program, descriptor, and sample to the pools, then Init().
     *
     * @ghidraAddress NTSC-U/C: 0x00002530
     * @ghidraAddress PAL: 0x00002530
     */
    void Clear();

    /**
     * Take a program from the pool and Init() it.
     *
     * @param program Receives the program.
     * @ghidraAddress NTSC-U/C: 0x00002698
     * @ghidraAddress PAL: 0x00002698
     */
    static void NewProgram(BankProgram **program);

    /**
     * Take a descriptor from the pool and Init() it.
     *
     * @param sampleDesc Receives the descriptor.
     * @ghidraAddress NTSC-U/C: 0x000026d0
     * @ghidraAddress PAL: 0x000026d0
     */
    static void NewSampleDesc(BankSampleDesc **sampleDesc);

    /**
     * Take a sample from the pool and Init() it.
     *
     * @param sample Receives the sample.
     * @ghidraAddress NTSC-U/C: 0x00002708
     * @ghidraAddress PAL: 0x00002708
     */
    static void NewSample(Sample **sample);

    /**
     * Find the loaded bank with an identifier.
     *
     * @param id The identifier.
     * @return The bank's slot, or -1 when no loaded bank has it.
     * @ghidraAddress NTSC-U/C: 0x00002740
     * @ghidraAddress PAL: 0x00002740
     */
    static int Find(unsigned short id);

    /**
     * Allocate the shared pools.
     *
     * @ghidraAddress NTSC-U/C: 0x000027b4
     * @ghidraAddress PAL: 0x000027b4
     */
    static void InitPools();

    /**
     * Release the shared pools.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00002c00
     * @ghidraAddress PAL: 0x00002c00
     */
    static int ReleasePools();

    unsigned char mLoaded;                         /*!< One once the EE marks the bank loaded. */
    BankHeader mHeader;                            /*!< The bank's identity and mix settings. */
    BankProgram *mPrograms[kMaxPrograms];          /*!< Programs by slot. */
    BankSampleDesc *mSampleDescs[kMaxSampleDescs]; /*!< Descriptors, in program order. */
    Sample *mSamples[kMaxSamples];                 /*!< Samples by index. */
    unsigned int mSpuAddress;                      /*!< Start of the bank's SPU2 area. */
    unsigned int mSpuSize;                         /*!< Size of the bank's SPU2 area in bytes. */

    static Bank sBanks[kMaxBanks]; /*!< The bank slots. */

private:
    static PoolAllocator sProgramPool;    /*!< Programs. */
    static PoolAllocator sSampleDescPool; /*!< Descriptors. */
    static PoolAllocator sSamplePool;     /*!< Samples. */
};
