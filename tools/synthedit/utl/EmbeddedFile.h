#pragma once

#include "os/File.h"
#include "utl/Str.h"

/**
 * A file compiled into the program.
 *
 * The RTTI records the class. The control never creates one, so only the destructor is recovered;
 * the type of the object it owns is inferred.
 */
class EmbeddedFile {
public:
    /**
     * Delete the file's stream.
     *
     * @ghidraAddress 0x100151e0
     */
    ~EmbeddedFile();

    String mName; /*!< The path the file is opened by. */
    File *mFile;  /*!< The open stream of the file, which this object deletes. */
};
