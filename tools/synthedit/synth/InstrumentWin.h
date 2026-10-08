#pragma once

#include <map>

#include "synth/SampleDescWin.h"
#include "synth/SampleWin.h"
#include "utl/Str.h"
#include "synth/common/Instrument.h"

/**
 * Editor view of one instrument of a bank and its sample descriptions.
 *
 * The class has no RTTI, and its name comes from the assert text of its users. The object is 0x2c
 * bytes. It deletes its sample descriptions. Its routines are in BankManager.cpp.
 */
class InstrumentWin {
public:
    /**
     * Create an instrument, named after its identifier.
     *
     * @param program The program number.
     * @ghidraAddress 0x1001c1c4
     */
    explicit InstrumentWin(unsigned short program);

    /**
     * Delete the sample descriptions.
     *
     * @ghidraAddress 0x1001c279
     */
    ~InstrumentWin();

    /**
     * Add a sample description that plays a sample, named after the sample, on the key after
     * the highest base key in use.
     *
     * @param iSampleWin The sample.
     * @return The identifier of the new SampleDescWin.
     * @ghidraAddress 0x1001fb6c
     */
    int AttachSample(SampleWin *iSampleWin);

    /**
     * Put a sample description on the key after the highest base key of the instrument, or on key
     * zero when none is in use or the next key is out of range.
     *
     * @param sdw The sample description.
     * @return Zero.
     * @ghidraAddress 0x1001fc9b
     */
    int SetDefaultKeymap(SampleDescWin *sdw);

    /**
     * Remove and delete a sample description.
     *
     * @param sampleDescID The identifier of the SampleDescWin.
     * @return Zero.
     * @ghidraAddress 0x1001ff03
     */
    int DeleteSampleDescription(int sampleDescID);

    /**
     * Remove and delete every sample description that plays a sample.
     *
     * @param sampleWin The sample.
     * @param removed Receives the identifier of the instrument under the identifier of each
     * removed SampleDescWin.
     * @return Zero.
     * @ghidraAddress 0x1001ff52
     */
    int RemoveSample(SampleWin *sampleWin, std::map<int, int> &removed);

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
     * Rename the instrument, unless the new name is empty.
     *
     * @param name The new name.
     */
    void SetName(const String &name) {
        if (name.size() > 0) {
            mName = name;
        }
    }

    /**
     * Report the program settings.
     *
     * @return The settings.
     */
    Instrument &GetInstrument() {
        return mInstrument;
    }

    /**
     * Report the sample descriptions.
     *
     * @return The sample descriptions by identifier.
     */
    std::map<int, SampleDescWin *> &GetSampleDescs() {
        return mSampleDescs;
    }

private:
    int mID;                                     /*!< Identifier, unique among editor objects. */
    String mName;                                /*!< Name shown in the editor. */
    Instrument mInstrument;                      /*!< Program settings. */
    std::map<int, SampleDescWin *> mSampleDescs; /*!< Sample descriptions by identifier. */
};
