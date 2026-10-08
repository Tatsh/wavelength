#pragma once

#include "synth/BankWin.h"
#include "utl/Str.h"

/**
 * Bank loaded from a file to report which notes it plays. No routine creates one.
 *
 * The class has no RTTI, and its name is inferred. The object is 4 bytes. Its routines are in
 * BankManager.cpp.
 */
class BankLookup {
public:
    /**
     * Load a bank without prompting for missing files.
     *
     * @param filename The path of the bank file.
     * @ghidraAddress 0x100201ec
     */
    explicit BankLookup(const String &filename);

    /**
     * Delete the bank.
     *
     * @ghidraAddress 0x10020271
     */
    ~BankLookup();

    /**
     * Report whether a note falls in the key range of a sample description.
     *
     * @param key The note.
     * @param program The program number of the instrument to search, or -1 for every instrument.
     * @return Whether a sample description covers the note.
     * @ghidraAddress 0x10020075
     */
    bool HasKey(unsigned char key, int program);

private:
    BankWin *mBankWin; /*!< The bank. */
};
