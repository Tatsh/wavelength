#pragma once

#include <vector>

#include "utl/EmbeddedFile.h"
#include "utl/Str.h"

/**
 * The files compiled into the program.
 *
 * The RTTI records the class. The object is 0x18 bytes. The control never adds a file, so only
 * the teardown is recovered, and the members of an entry other than its name are placeholders.
 */
class EmbeddedFileTable {
public:
    /**
     * Create an empty table.
     *
     * @ghidraAddress 0x10015070
     */
    EmbeddedFileTable();

    /**
     * Destroy the entries. The open files are not deleted.
     *
     * @ghidraAddress 0x10015090
     */
    ~EmbeddedFileTable();

    /**
     * Report the table, creating it at the first call.
     *
     * @return The table.
     * @ghidraAddress 0x10015010
     */
    static EmbeddedFileTable &Instance();

    /**
     * Delete the open files of the table.
     *
     * @ghidraAddress 0x10015060
     */
    static void ClearAll();

    /**
     * Delete the open files.
     *
     * @ghidraAddress 0x10015180
     */
    void Clear();

private:
    /** A file the table can open. The object is 0x28 bytes. */
    struct Entry {
        String mName;           /*!< The path. */
        int mReserved14;        // +0x14, not yet identified.
        int mReserved18;        // +0x18, not yet identified.
        std::vector<int> mData; /*!< The file's words; the element type is inferred. */
    };

    std::vector<EmbeddedFile *> mFiles; /*!< The open files. */
    std::vector<Entry> mEntries;        /*!< The files that can be opened. */
};
