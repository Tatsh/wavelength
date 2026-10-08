#include "memcard/memcardconfig.h"

#include <string.h>

#include "os/file.h"
#include "os/system.h"

namespace {

// The open mode File::New() reads a file with.
constexpr int kFileReadMode = 1;

// The colours of the four corners of the icon background, each red, green, and blue.
constexpr int kIconCornerCount = 4;
constexpr int kColorComponentCount = 3;

// The lights of the icon, each with a direction and a colour of three components.
constexpr int kIconLightCount = 3;
constexpr int kLightComponentCount = 3;

// The kilobytes a new directory occupies besides its files.
constexpr int kDirEntryKilobytes = 2;

// The bytes of a kilobyte, less one, for rounding up.
constexpr int kKilobyteRounding = 1023;
constexpr int kKilobyteShift = 10;

constexpr char kIconSysMagic[] = "PS2D";

} // namespace

DataArray *g_pMemcardConfig;
char *g_pMemcardBuffer;
int g_nMemcardBufferSize;
char *g_pMemcardIcon;
int g_nMemcardIconSize;
int g_bMemcardDirExists;
int g_nMemcardDirKilobytes;
int g_nMaxRemixesPerDir;
int g_nMaxRemixDirs;
int g_nMaxRemixes;
const char *g_pszRemixExt;
const char *g_pszFreqExt;
const char *g_pszMemcardBaseDir;
// The unit's static initialiser at NTSC-U/C: 0x001626d8, and its global constructor at
// NTSC-U/C: 0x001627c8, PAL: 0x00165540, construct and destroy the three.
String g_RemixPath;
String g_MemcardPath;
std::list<String> g_RemixNames;
sceMcIconSys g_MemcardIconSys;

const char *GetRemixDirName(int nIndex) {
    const char *pszDirExt;
    g_pMemcardConfig->FindSymbol("remix_dir_ext", &pszDirExt, true);
    return FormatString("%s%s%d", g_pszMemcardBaseDir, pszDirExt, nIndex);
}

int BytesToKilobytes(int nBytes) {
    return (nBytes + kKilobyteRounding) >> kKilobyteShift;
}

void LoadMemcardConfig() {
    g_pMemcardConfig = SystemConfig()->FindArray("metagame", true)->FindArray("mc", true);
    g_pMemcardConfig->FindInt("buff_size", &g_nMemcardBufferSize, true);
    g_pMemcardBuffer = new char[g_nMemcardBufferSize];

    const char *pszIcon;
    g_pMemcardConfig->FindSymbol("icon_file", &pszIcon, true);
    File *pFile = File::New(FormatString("mc/%s", pszIcon), kFileReadMode, 0);
    g_nMemcardIconSize = pFile->Size();
    g_pMemcardIcon = new char[g_nMemcardIconSize];
    pFile->Read(g_pMemcardIcon, g_nMemcardIconSize);
    delete pFile;

    memset(&g_MemcardIconSys, 0, sizeof(g_MemcardIconSys));
    // Yes, the binary copies the terminator of the magic into the reserved field after it.
    memcpy(&g_MemcardIconSys, kIconSysMagic, sizeof(kIconSysMagic));
    int nLineBreak;
    g_pMemcardConfig->FindInt("label_line_break", &nLineBreak, true);
    g_MemcardIconSys.OffsLF = nLineBreak * 2;
    int nTransparency = static_cast<int>(g_MemcardIconSys.TransRate);
    g_pMemcardConfig->FindInt("icon_bg_transparency", &nTransparency, false);
    g_MemcardIconSys.TransRate = nTransparency;

    DataArray *pColors = g_pMemcardConfig->FindArray("icon_bg_colors", true);
    for (int i = 0; i < kIconCornerCount; ++i) {
        for (int j = 0; j < kColorComponentCount; ++j) {
            g_MemcardIconSys.BgColor[i][j] = pColors->Array(i + 1)->Int(j);
        }
    }
    DataArray *pDirs = g_pMemcardConfig->FindArray("icon_lit_dirs", true);
    for (int i = 0; i < kIconLightCount; ++i) {
        for (int j = 0; j < kLightComponentCount; ++j) {
            g_MemcardIconSys.LightDir[i][j] = pDirs->Array(i + 1)->Float(j);
        }
    }
    DataArray *pLightColors = g_pMemcardConfig->FindArray("icon_lit_colors", true);
    for (int i = 0; i < kIconLightCount; ++i) {
        for (int j = 0; j < kLightComponentCount; ++j) {
            g_MemcardIconSys.LightColor[i][j] = pLightColors->Array(i + 1)->Float(j);
        }
    }
    DataArray *pAmbient = g_pMemcardConfig->FindArray("icon_ambient", true);
    for (int i = 0; i < kLightComponentCount; ++i) {
        g_MemcardIconSys.Ambient[i] = pAmbient->Float(i + 1);
    }

    const size_t nIconNameSize = strlen(pszIcon) + 1;
    memcpy(g_MemcardIconSys.FnameView, pszIcon, nIconNameSize);
    memcpy(g_MemcardIconSys.FnameCopy, pszIcon, nIconNameSize);
    memcpy(g_MemcardIconSys.FnameDel, pszIcon, nIconNameSize);

    g_nMemcardDirKilobytes = BytesToKilobytes(g_nMemcardIconSize);
    g_nMemcardDirKilobytes += BytesToKilobytes(kMemcardDirFileSize);
    g_nMemcardDirKilobytes += BytesToKilobytes(sizeof(sceMcIconSys)) + kDirEntryKilobytes;

    g_pMemcardConfig->FindInt("max_remixes_perdir", &g_nMaxRemixesPerDir, true);
    g_pMemcardConfig->FindInt("max_remix_dirs", &g_nMaxRemixDirs, true);
    g_pMemcardConfig->FindInt("max_remixes", &g_nMaxRemixes, true);
    g_pMemcardConfig->FindSymbol("remix_ext", &g_pszRemixExt, true);
    g_pMemcardConfig->FindSymbol("freq_ext", &g_pszFreqExt, true);
    g_pMemcardConfig->FindSymbol("base_dir", &g_pszMemcardBaseDir, true);
}

void FreeMemcardConfig() {
    // Yes, the binary releases both arrays with the scalar delete.
    delete g_pMemcardBuffer;
    delete g_pMemcardIcon;
}
