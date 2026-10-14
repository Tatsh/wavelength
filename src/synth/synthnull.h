#pragma once

#include <vector>

#include "os/string.h"
#include "synth/synth.h"

/**
 * Sound output that does nothing, built when the configuration asks for no synthesiser.
 *
 * The RTTI includes the class name and records Synth as the base. The object is the vptr alone,
 * and its vtable is at `0x003d3f10`. Every member that the base leaves pure has an empty body here,
 * and every body is expanded in the header.
 */
class SynthNull : public Synth {
public:
    /**
     * Do nothing.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x003960c0
     * @ghidraAddress PAL: 0x00404710
     */
    int Init() override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x003960c8
     */
    int Terminate() override {
        return 0;
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x003960d0
     * @ghidraAddress PAL: 0x00404720
     */
    ~SynthNull() override {
    }

    /**
     * Discard a message.
     *
     * @param nPacked The message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396100
     */
    int SendPackedMessage([[maybe_unused]] unsigned int nPacked) override {
        return 0;
    }

    /**
     * Discard a message.
     *
     * @param nByte0 The first message byte.
     * @param nByte1 The second message byte.
     * @param nByte2 The third message byte.
     * @param nByte3 The delay of the message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396108
     */
    int SendMessage([[maybe_unused]] unsigned char nByte0,
                    [[maybe_unused]] unsigned char nByte1,
                    [[maybe_unused]] unsigned char nByte2,
                    [[maybe_unused]] int nByte3) override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x00396110
     */
    void UpdateTime() override {
    }

    /**
     * Do nothing.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396118
     */
    int Poll() override {
        return 0;
    }

    /**
     * Report nothing.
     *
     * @param pnRate Unchanged.
     * @param pnBlock Unchanged.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396120
     */
    int GetStreamFormat([[maybe_unused]] int *pnRate, [[maybe_unused]] int *pnBlock) override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @param blocks Unused.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396128
     */
    int Partition([[maybe_unused]] const std::vector<int> &blocks) override {
        return 0;
    }

    /**
     * Report an empty bank.
     *
     * @param file Unused.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396130
     */
    int GetBankBlockCount([[maybe_unused]] const String &file) override {
        return 0;
    }

    /**
     * Report every bank loaded.
     *
     * @param nSlot Unused.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x00396138
     */
    bool IsBankLoaded([[maybe_unused]] unsigned short nSlot) override {
        return true;
    }

    /**
     * Do nothing.
     *
     * @param nSlot Unused.
     * @param file Unused.
     * @param bSkipPending Unused.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396140
     */
    int LoadBank([[maybe_unused]] unsigned short nSlot,
                 [[maybe_unused]] const String &file,
                 [[maybe_unused]] bool bSkipPending) override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @param nSlot Unused.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396148
     */
    int UnloadBank([[maybe_unused]] unsigned short nSlot) override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396150
     */
    int EnableSoftFx() override {
        return 0;
    }

    /**
     * Do nothing.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00396158
     */
    int DisableSoftFx() override {
        return 0;
    }
};
