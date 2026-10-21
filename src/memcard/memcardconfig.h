#pragma once

#include <list>
#include <memory>
#include <vector>

#include <libmc.h>

#include "os/string.h"
#include "script/dataarray.h"

/** The size of the file named after its directory that every save directory receives. */
constexpr int kMemcardDirFileSize = 2;

/**
 * The `mc` section of the `metagame` system configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x003af870
 * @ghidraAddress PAL: 0x0042e70c
 */
extern DataArray *g_pMemcardConfig;

/**
 * The buffer a save is serialised into and a load is read into, of `buff_size` bytes.
 *
 * @ghidraAddress NTSC-U/C: 0x003af858
 * @ghidraAddress PAL: 0x0042e6f0
 */
extern std::unique_ptr<char[]> g_pMemcardBuffer;

/**
 * The size of g_pMemcardBuffer, the `buff_size` of the configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x003af85c
 * @ghidraAddress PAL: 0x0042e6f4
 */
extern int g_nMemcardBufferSize;

#ifdef VIDEO_STANDARD_PAL
/**
 * The result of the last memory card read, the bytes it read or an error code.
 *
 * @ghidraAddress PAL: 0x0042e6f8
 */
extern int g_nMemcardReadResult;

/**
 * Report whether the last memory card read read the given number of bytes.
 *
 * The name is inferred.
 *
 * @param nBytes The number of bytes a whole file has.
 * @return Whether g_nMemcardReadResult is nBytes.
 * @ghidraAddress PAL: 0x0015eab8
 */
bool MemcardReadSizeIs(int nBytes);
#endif

/**
 * The icon file every save directory receives, read from the `icon_file` of the disc's `mc`
 * directory.
 *
 * @ghidraAddress NTSC-U/C: 0x003af860
 * @ghidraAddress PAL: 0x0042e6fc
 */
extern std::unique_ptr<char[]> g_pMemcardIcon;

/**
 * The size of g_pMemcardIcon.
 *
 * @ghidraAddress NTSC-U/C: 0x003af864
 * @ghidraAddress PAL: 0x0042e700
 */
extern int g_nMemcardIconSize;

/**
 * Non-zero when the directory the next save goes into exists already.
 *
 * @ghidraAddress NTSC-U/C: 0x003af868
 * @ghidraAddress PAL: 0x0042e704
 */
extern int g_bMemcardDirExists;

/**
 * The kilobytes a new save directory needs besides the save itself.
 *
 * @ghidraAddress NTSC-U/C: 0x003af86c
 * @ghidraAddress PAL: 0x0042e708
 */
extern int g_nMemcardDirKilobytes;

/**
 * The remixes one remix directory may include, `max_remixes_perdir`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af874
 * @ghidraAddress PAL: 0x0042e710
 */
extern int g_nMaxRemixesPerDir;

/**
 * The remix directories a card may include, `max_remix_dirs`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af878
 * @ghidraAddress PAL: 0x0042e714
 */
extern int g_nMaxRemixDirs;

/**
 * The remixes a card may include, `max_remixes`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af87c
 * @ghidraAddress PAL: 0x0042e718
 */
extern int g_nMaxRemixes;

/**
 * The extension of a remix file, `remix_ext`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af880
 * @ghidraAddress PAL: 0x0042e71c
 */
extern const char *g_pszRemixExt;

/**
 * The extension of a Freq file, `freq_ext`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af884
 * @ghidraAddress PAL: 0x0042e720
 */
extern const char *g_pszFreqExt;

/**
 * The prefix of every save directory, `base_dir`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af888
 * @ghidraAddress PAL: 0x0042e724
 */
extern const char *g_pszMemcardBaseDir;

/**
 * The remix file the last search found, as a path below the card root.
 *
 * @ghidraAddress NTSC-U/C: 0x00436340
 * @ghidraAddress PAL: 0x004d30d8
 */
extern String g_RemixPath;

#ifdef VIDEO_STANDARD_PAL
/**
 * The file MCOpenTask opened last, as a path below the card root.
 *
 * The name is inferred. A load that finds the file corrupt deletes it by this path.
 *
 * @ghidraAddress PAL: 0x004d30f0
 */
extern String g_MemcardOpenPath;
#endif

/**
 * The save directory the current work addresses.
 *
 * @ghidraAddress NTSC-U/C: 0x00436358
 * @ghidraAddress PAL: 0x004d3108
 */
extern String g_MemcardPath;

/**
 * The remix files the last listing found, as paths below the card root.
 *
 * @ghidraAddress NTSC-U/C: 0x00436370
 * @ghidraAddress PAL: 0x004d3120
 */
#ifdef VIDEO_STANDARD_PAL
extern std::vector<String> g_RemixNames;
#else
extern std::list<String> g_RemixNames;
#endif

/**
 * The `icon.sys` every save directory receives. LoadMemcardConfig() fills every field but the
 * title. MCCreateSaveDirTask::Set() writes the title.
 *
 * @ghidraAddress NTSC-U/C: 0x00436378
 */
extern sceMcIconSys g_MemcardIconSys;

/**
 * Read the memory card configuration, the icon file, and the icon description.
 *
 * @ghidraAddress NTSC-U/C: 0x0015d2c0
 * @ghidraAddress PAL: 0x0015ead8
 */
void LoadMemcardConfig();

/**
 * Release the buffers LoadMemcardConfig() allocated.
 *
 * @ghidraAddress NTSC-U/C: 0x0015d758
 * @ghidraAddress PAL: 0x0015ef70
 */
void FreeMemcardConfig();

/**
 * Report the name of a remix directory.
 *
 * @param nIndex The directory.
 * @return The name, in the shared FormatString() buffer.
 * @ghidraAddress NTSC-U/C: 0x0015d258
 * @ghidraAddress PAL: 0x0015ea48
 */
const char *GetRemixDirName(int nIndex);

/**
 * Report the kilobytes a number of bytes occupies.
 *
 * @param nBytes The number of bytes.
 * @return The kilobytes, rounded up.
 * @ghidraAddress NTSC-U/C: 0x0015d2b0
 * @ghidraAddress PAL: 0x0015eaa0
 */
int BytesToKilobytes(int nBytes);
