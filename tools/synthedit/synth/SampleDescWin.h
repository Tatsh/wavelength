#pragma once

#include "synth/common/SampleDescription.h"
#include "utl/Str.h"

/**
 * Editor view of one sample description of an instrument.
 *
 * The class has no RTTI, and its name comes from the assert text of its users. The object is 0x38
 * bytes. Its routines are in BankManager.cpp.
 */
class SampleDescWin {
public:
    /**
     * Create a description of a sample, named after its identifier.
     *
     * @param sampleID The identifier of the SampleWin it plays.
     * @ghidraAddress 0x1001c020
     */
    explicit SampleDescWin(int sampleID);

    /**
     * Release the description.
     *
     * @ghidraAddress 0x1001c0c4
     */
    ~SampleDescWin();

    /**
     * Set the key range and base key.
     *
     * @param iLow The lowest note, below 128.
     * @param iBase The base key, below 128.
     * @param iHigh The highest note, below 128.
     * @ghidraAddress 0x1001c0da
     */
    void SetKeymap(unsigned char iLow, unsigned char iBase, unsigned char iHigh);

    /**
     * Report the key range and base key.
     *
     * @param oLow Receives the lowest note.
     * @param oBase Receives the base key.
     * @param oHigh Receives the highest note.
     * @ghidraAddress 0x1001c196
     */
    void GetKeymap(unsigned char *oLow, unsigned char *oBase, unsigned char *oHigh);

    /**
     * Report the identifier.
     *
     * @return The identifier.
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
     * Rename the description, unless the new name is empty.
     *
     * @param name The new name.
     */
    void SetName(const String &name) {
        if (name.size() > 0) {
            mName = name;
        }
    }

    /**
     * Report the identifier of the sample the description plays.
     *
     * @return The identifier of the SampleWin.
     * @ghidraAddress 0x10020430
     */
    int GetSampleID() const {
        return mSampleID;
    }

    /**
     * Report the playback settings.
     *
     * @return The settings.
     */
    SampleDescription &GetSampleDescription() {
        return mSampleDescription;
    }

private:
    int mID;                              /*!< Identifier, unique among every editor object. */
    String mName;                         /*!< Name shown in the editor. */
    int mSampleID;                        /*!< Identifier of the SampleWin it plays. */
    SampleDescription mSampleDescription; /*!< Playback settings. */
};
