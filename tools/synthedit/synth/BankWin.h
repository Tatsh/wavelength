#pragma once

#include <map>
#include <vector>

#include "synth/InstrumentWin.h"
#include "synth/SampleWin.h"
#include "utl/Str.h"
#include "synth/common/Bank.h"

/**
 * Editor view of one bank: its instruments, its samples, and the files it is stored in.
 *
 * The class has no RTTI, and its name comes from the assert text of its users. The object is 0x50
 * bytes. It deletes its instruments and samples. Its routines are in BankManager.cpp.
 */
class BankWin {
public:
    /**
     * Create a bank, loading it when a file is given.
     *
     * @param filename The path of the bank file, or an empty string for a new bank.
     * @param bankID The bank identifier.
     * @param promptForMissingFiles Whether loading offers to find a replacement for a missing WAV
     * file.
     * @ghidraAddress 0x1001c2da
     */
    BankWin(const String &filename, unsigned short bankID, bool promptForMissingFiles);

    /**
     * Delete the instruments and samples.
     *
     * @ghidraAddress 0x1001c3c7
     */
    ~BankWin();

    /**
     * Report whether two instruments share a program number.
     *
     * @return Whether a program number is used twice.
     * @ghidraAddress 0x1001c4ec
     */
    bool HasDuplicateProgramIds();

    /**
     * Write the bank file, and the audio data file when the samples changed or it is requested.
     * The write is refused, with a notice, when the bank breaks a limit of the console.
     *
     * @param filename The path to write, or an empty string for the path the bank was loaded from.
     * @param forceAudioData Whether to write the audio data file even when no sample changed.
     * @return Zero, or -1 when the write was refused.
     * @ghidraAddress 0x1001c5f0
     */
    int Save(const String &filename, bool forceAudioData);

    /**
     * Replace the contents with those of the bank file.
     *
     * @return The identifier of the bank.
     * @ghidraAddress 0x1001daf0
     */
    int Load();

    /**
     * Add an instrument. A program number already in use, or below 1, becomes the one after the
     * highest in use.
     *
     * @param program The program number.
     * @return The identifier of the new InstrumentWin.
     * @ghidraAddress 0x1001f3e2
     */
    int NewInstrument(unsigned short program);

    /**
     * Delete an instrument.
     *
     * @param instrumentID The identifier of the InstrumentWin.
     * @param sampleIDs Receives samples the instrument played. Only a sample found at the start
     * of the list is added.
     * @return Zero.
     * @ghidraAddress 0x1001f5a0
     */
    int DeleteInstrument(int instrumentID, std::vector<int> &sampleIDs);

    /**
     * Describe a sample: its name, its file, its format, and the instruments that play it.
     *
     * @param sampleID The identifier of the SampleWin.
     * @return The description.
     * @ghidraAddress 0x1001f66b
     */
    String GetSampleInfo(unsigned short sampleID);

    /**
     * Add a sample made from a WAV file, unless one is already made from it.
     *
     * @param filename The path of the WAV file.
     * @param alreadyExists Receives whether a sample already used the file.
     * @return The identifier of the SampleWin, or -1 when the file was unusable.
     * @ghidraAddress 0x1001f86d
     */
    int NewSample(const String &filename, bool *alreadyExists);

    /**
     * Delete a sample and every sample description that plays it.
     *
     * @param sampleID The identifier of the SampleWin.
     * @param removed Receives the identifier of the instrument under the identifier of each
     * removed SampleDescWin.
     * @return Zero.
     * @ghidraAddress 0x1001fa8e
     */
    int DeleteSample(int sampleID, std::map<int, int> &removed);

    /**
     * List the sample descriptions that play a sample.
     *
     * @param names Receives the name of each SampleDescWin.
     * @param sampleDescIDs Receives the identifier of each SampleDescWin.
     * @param sampleID The identifier of the SampleWin.
     * @ghidraAddress 0x1001fd98
     */
    void GetSampleUsers(std::vector<String> &names, std::vector<int> &sampleDescIDs, int sampleID);

    /**
     * Report the identifier.
     *
     * @return The identifier.
     * @ghidraAddress 0x10020430
     */
    int GetID() const {
        return mID;
    }

    /**
     * Report the name.
     *
     * @return The name.
     */
    String &GetName() {
        return mName;
    }

    /**
     * Rename the bank, unless the new name is empty.
     *
     * @param name The new name.
     */
    void SetName(const String &name) {
        if (name.size() > 0) {
            mName = name;
        }
    }

    /**
     * Report the path of the bank file.
     *
     * @return The path, empty for a bank never saved.
     */
    String &GetFilename() {
        return mFilename;
    }

    /**
     * Report the header.
     *
     * @return The header.
     */
    Bank &GetBank() {
        return mBank;
    }

    /**
     * Report the instruments.
     *
     * @return The instruments by identifier.
     */
    std::map<int, InstrumentWin *> &GetInstruments() {
        return mInstruments;
    }

    /**
     * Report the samples.
     *
     * @return The samples by identifier.
     */
    std::map<int, SampleWin *> &GetSamples() {
        return mSamples;
    }

private:
    friend class CSyntheditCtrl;

    bool mAudioDataDirty;                        /*!< Whether a sample changed since a save. */
    String mName;                                /*!< Name shown in the editor. */
    int mID;                                     /*!< Identifier, unique among editor objects. */
    String mFilename;                            /*!< Path of the bank file. */
    bool mPromptForMissingFiles;                 /*!< Whether a load offers replacement files. */
    Bank mBank;                                  /*!< The header. */
    std::map<int, InstrumentWin *> mInstruments; /*!< Instruments by identifier. */
    std::map<int, SampleWin *> mSamples;         /*!< Samples by identifier. */
};
