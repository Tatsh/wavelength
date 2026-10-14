#pragma once

#include <vector>

#include "os/binstream.h"
#include "os/string.h"

/**
 * Path of a file, kept relative to a shared root directory.
 *
 * The RTTI records the class as deriving from String, and the class adds no member. The vtable is
 * at `0x003cc948`.
 */
class FilePath : public String {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00357028
     * @ghidraAddress PAL: 0x003c4288
     */
    ~FilePath() override = default;

    /**
     * Set the root directory that relative paths resolve against.
     *
     * An absolute path replaces the root. A relative one is appended to the current root.
     *
     * @param pszRoot The directory.
     * @ghidraAddress NTSC-U/C: 0x002993b8
     * @ghidraAddress PAL: 0x002a2fc8
     */
    static void SetRoot(const char *pszRoot);

    /**
     * Set the path, resolving a relative path against the root directory.
     *
     * @param pszPath The path.
     * @ghidraAddress NTSC-U/C: 0x00299448
     * @ghidraAddress PAL: 0x002a3058
     */
    void Set(const char *pszPath);

    /**
     * Report the path relative to the root directory.
     *
     * The name is inferred.
     *
     * @return The path.
     * @ghidraAddress NTSC-U/C: 0x002994d0
     * @ghidraAddress PAL: 0x002a30e0
     */
    const char *RelativePath() const;

    /**
     * The root directory that relative paths resolve against, `.` until SetRoot() runs.
     *
     * One routine at 0x002994f8 constructs and destroys it, called by the static construction and
     * destruction entries at 0x00299548 and 0x00299568.
     *
     * @ghidraAddress NTSC-U/C: 0x00515318
     */
    static String sRoot;
};

/**
 * Write a vector of paths as its size followed by each path relative to the root directory.
 *
 * @param stream The stream to write to.
 * @param paths The paths.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0038eda8
 * @ghidraAddress PAL: 0x003fd4b0
 */
inline BinStream &operator<<(BinStream &stream, const std::vector<FilePath> &paths) {
    const int nSize = static_cast<int>(paths.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const FilePath &path : paths) {
        stream.WriteString(path.RelativePath());
    }
    return stream;
}
