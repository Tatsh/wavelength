#pragma once

#include "game/bankloader.h"
#include "mid/trackbuilder.h"
#include "os/string.h"

/**
 * TrackBuilder that reads the bank files of the track named "BANK" into a BankLoader.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. Each text event of the
 * track gives a bank file. A file whose name ends in `_p` is a permanent bank, and one whose name
 * ends in `_s` and a number is a swapped bank that plays from the bar of the event.
 */
class BankLoaderBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nIntroTicks The length of the song intro.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pDirectory The directory of the bank files.
     * @param pLoader The loader to fill.
     * @ghidraAddress NTSC-U/C: 0x0014a458
     * @ghidraAddress PAL: 0x0014be18
     */
    BankLoaderBuilder(int nTrack,
                      bool bValidate,
                      ErrorHandler pfnError,
                      int nIntroTicks,
                      int nTicksPerBar,
                      const String *pDirectory,
                      BankLoader *pLoader);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a500
     * @ghidraAddress PAL: 0x0014bec0
     */
    ~BankLoaderBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x00349ba0
     * @ghidraAddress PAL: 0x003b6fd0
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Ignore the end of the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00349bc0
     * @ghidraAddress PAL: 0x003b6ff0
     */
    void OnEndTrack() override {
    }

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00349ba8
     * @ghidraAddress PAL: 0x003b6fd8
     */
    void OnAllDone() override {
    }

    /**
     * Report a channel message, which the track must not have.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0014a550
     * @ghidraAddress PAL: 0x0014bf10
     */
    void OnMidi(int nTick,
                [[maybe_unused]] unsigned char nStatus,
                [[maybe_unused]] unsigned char nData1,
                [[maybe_unused]] unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00349bb0
     * @ghidraAddress PAL: 0x003b6fe0
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Add the bank file of a text event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type. Only a plain text event gives a bank.
     * @ghidraAddress NTSC-U/C: 0x0014a570
     * @ghidraAddress PAL: 0x0014bf30
     */
    void OnText(int nTick, const char *pszText, unsigned char nType) override;

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x00349bb8
     * @ghidraAddress PAL: 0x003b6fe8
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    const String *mDirectory; /*!< The directory of the bank files. */
    int mIntroTicks;          /*!< The length of the song intro. */
    int mTicksPerBar;         /*!< The song ticks in one bar. */
    BankLoader *mLoader;      /*!< The loader to fill. */
    String mLastSwapBank;     /*!< The name of the last swapped bank. */

private:
    /**
     * Report whether a bank name ends in `_p`.
     *
     * @param name The bank file name without its directory and extension.
     * @return Whether the bank is permanent.
     * @ghidraAddress NTSC-U/C: 0x0014a358
     * @ghidraAddress PAL: 0x0014bd18
     */
    static bool IsPermBankName(const String &name);

    /**
     * Report whether a bank name ends in `_s` and a number.
     *
     * @param name The bank file name without its directory and extension.
     * @return Whether the bank is swapped.
     * @ghidraAddress NTSC-U/C: 0x0014a3a8
     * @ghidraAddress PAL: 0x0014bd68
     */
    static bool IsSwapBankName(const String &name);

    /**
     * Add a bank file to the loader as a permanent or a swapped bank.
     *
     * @param nTick The tick of the event.
     * @param pszFile The bank file.
     * @ghidraAddress NTSC-U/C: 0x0014a598
     * @ghidraAddress PAL: 0x0014bf58
     */
    void AddBank(int nTick, const char *pszFile);
};
