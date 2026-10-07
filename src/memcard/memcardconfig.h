#pragma once

#include <list>

#include <libmc.h>

#include "os/string.h"
#include "script/dataarray.h"

/** The size of the file named after its directory that every save directory receives. */
constexpr int kMemcardDirFileSize = 2;

/**
 * The `mc` section of the `metagame` system configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x003af870
 */
extern DataArray *g_pMemcardConfig;

/**
 * The buffer a save is serialised into and a load is read into, of `buff_size` bytes.
 *
 * @ghidraAddress NTSC-U/C: 0x003af858
 */
extern char *g_pMemcardBuffer;

/**
 * The size of g_pMemcardBuffer, the `buff_size` of the configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x003af85c
 */
extern int g_nMemcardBufferSize;

/**
 * The icon file every save directory receives, read from the `icon_file` of the disc's `mc`
 * directory.
 *
 * @ghidraAddress NTSC-U/C: 0x003af860
 */
extern char *g_pMemcardIcon;

/**
 * The size of g_pMemcardIcon.
 *
 * @ghidraAddress NTSC-U/C: 0x003af864
 */
extern int g_nMemcardIconSize;

/**
 * Non-zero when the directory the next save goes into exists already.
 *
 * @ghidraAddress NTSC-U/C: 0x003af868
 */
extern int g_bMemcardDirExists;

/**
 * The kilobytes a new save directory needs besides the save itself.
 *
 * @ghidraAddress NTSC-U/C: 0x003af86c
 */
extern int g_nMemcardDirKilobytes;

/**
 * The remixes one remix directory may include, `max_remixes_perdir`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af874
 */
extern int g_nMaxRemixesPerDir;

/**
 * The remix directories a card may include, `max_remix_dirs`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af878
 */
extern int g_nMaxRemixDirs;

/**
 * The remixes a card may include, `max_remixes`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af87c
 */
extern int g_nMaxRemixes;

/**
 * The extension of a remix file, `remix_ext`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af880
 */
extern const char *g_pszRemixExt;

/**
 * The extension of a Freq file, `freq_ext`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af884
 */
extern const char *g_pszFreqExt;

/**
 * The prefix of every save directory, `base_dir`.
 *
 * @ghidraAddress NTSC-U/C: 0x003af888
 */
extern const char *g_pszMemcardBaseDir;

/**
 * The remix file the last search found, as a path below the card root.
 *
 * @ghidraAddress NTSC-U/C: 0x00436340
 */
extern String g_RemixPath;

/**
 * The save directory the current work addresses.
 *
 * @ghidraAddress NTSC-U/C: 0x00436358
 */
extern String g_MemcardPath;

/**
 * The remix files the last listing found, as paths below the card root.
 *
 * @ghidraAddress NTSC-U/C: 0x00436370
 */
extern std::list<String> g_RemixNames;

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
