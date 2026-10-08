#pragma once

#include "os/Debug.h"
#include "synth/AudioIO.h"
#include "synth/common/Sample.h"
#include "utl/Str.h"

/**
 * Editor view of one sample of a bank, tied to the WAV file it was made from.
 *
 * The class has no RTTI, and its name comes from the assert text of its users. The object is 0x2c
 * bytes. Its routines are in BankManager.cpp.
 */
class SampleWin {
public:
    /**
     * Create a sample from a WAV file, naming it after the file.
     *
     * @param filename The path of the WAV file.
     * @ghidraAddress 0x1001be58
     */
    explicit SampleWin(const String &filename);

    /**
     * Release the reader of the WAV file.
     *
     * @ghidraAddress 0x1001bf56
     */
    ~SampleWin();

    /**
     * Report whether the WAV file is unreadable or is not 16-bit mono.
     *
     * @return Whether the sample is unusable.
     * @ghidraAddress 0x1001bfc6
     */
    bool Fail();

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
     * Rename the sample, unless the new name is empty.
     *
     * @param name The new name.
     */
    void SetName(const String &name) {
        if (name.size() > 0) {
            mName = name;
        }
    }

    /**
     * Report the reader of the WAV file.
     *
     * @return The reader.
     * @ghidraAddress 0x10006df0
     */
    AudioIO &GetAudioIO() {
        ASSERT(mAudioIO);
        return *mAudioIO;
    }

    /**
     * Report the playback parameters.
     *
     * @return The parameters.
     */
    Sample &GetSample() {
        return mSample;
    }

private:
    int mID;           /*!< Identifier, unique among every editor object. */
    String mName;      /*!< Name shown in the editor. */
    AudioIO *mAudioIO; /*!< Reader of the WAV file. */
    Sample mSample;    /*!< Playback parameters. */
};
