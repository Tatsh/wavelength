#pragma once

/**
 * Open file of the host or the disc.
 *
 * The class is polymorphic. Each kind of storage implements it. Only the members its callers here
 * use are declared.
 */
class File {
public:
    /** Close the file. */
    virtual ~File();
};
