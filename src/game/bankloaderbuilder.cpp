#include "game/bankloaderbuilder.h"

#include <cctype>

#include "os/fileutil.h"

namespace {

constexpr char kTextOnlyError[] = "The bank track may only contain text messages";
constexpr char kNotBankFileFormat[] = "\"%s\" is not a bank file name (should end with %s)";
constexpr char kPermNotFirstError[] =
    "Permanent bank messages should be at the first tick in the BANK track";
constexpr char kNotOnBarFormat[] = "Bank %s is not on a bar boundary";
constexpr char kTwiceFormat[] = "Bank %s appears twice in a row";
constexpr char kBadNameFormat[] = "Incorrect bank file name \"%s\"; should end with \"_p\" or "
                                  "\"_s_#\"";
constexpr char kPermSuffix[] = "_p";
constexpr char kSwapSuffix[] = "_s";
constexpr char kPathSeparator[] = "/";

// The meta event type of a plain text event.
constexpr unsigned char kTextEvent = 1;

// The tick every permanent bank must be at.
constexpr int kFirstTick = 0;

// The characters of kSwapSuffix.
constexpr int kSwapSuffixLength = 2;

// The unit's static initialiser at NTSC-U/C: 0x0014a800, PAL: 0x0014c1c0, and its global
// constructor at NTSC-U/C: 0x0014a870, PAL: 0x0014c230, construct and destroy the suffixes.
// NTSC-U/C: 0x00436258
String g_strBankFileSuffix(".bnk");

// NTSC-U/C: 0x00436270
String g_strNoiseFileSuffix(".nse");

// Report whether a string ends with another string.
// NTSC-U/C: 0x0014a300, PAL: 0x0014bcc0
bool EndsWith(const String &text, const String &suffix) {
    const String tail = text.Substring(text.mLength - suffix.mLength);
    return tail == suffix;
}

} // namespace

bool BankLoaderBuilder::IsPermBankName(const String &name) {
    return EndsWith(name, String(kPermSuffix));
}

bool BankLoaderBuilder::IsSwapBankName(const String &name) {
    int nPos = name.mLength - 1;
    while (isdigit(name[nPos])) {
        if (nPos < 0) {
            break;
        }
        --nPos;
    }
    return name.Substring(nPos - kSwapSuffixLength, kSwapSuffixLength) == kSwapSuffix;
}

BankLoaderBuilder::BankLoaderBuilder(int nTrack,
                                     bool bValidate,
                                     ErrorHandler pfnError,
                                     int nIntroTicks,
                                     int nTicksPerBar,
                                     const String *pDirectory,
                                     BankLoader *pLoader)
    : TrackBuilder(nTrack, bValidate, pfnError), mDirectory(pDirectory), mIntroTicks(nIntroTicks),
      mTicksPerBar(nTicksPerBar), mLoader(pLoader) {
}

BankLoaderBuilder::~BankLoaderBuilder() {
}

void BankLoaderBuilder::OnMidi(int nTick,
                               [[maybe_unused]] unsigned char nStatus,
                               [[maybe_unused]] unsigned char nData1,
                               [[maybe_unused]] unsigned char nData2) {
    Error(nTick, kTextOnlyError);
}

void BankLoaderBuilder::OnText(int nTick, const char *pszText, unsigned char nType) {
    if (nType == kTextEvent) {
        AddBank(nTick, pszText);
    }
}

void BankLoaderBuilder::AddBank(int nTick, const char *pszFile) {
    if (!EndsWith(String(pszFile), g_strBankFileSuffix)) {
        Error(nTick, FormatString(kNotBankFileFormat, pszFile, g_strBankFileSuffix.c_str()));
        return;
    }

    String name(FileGetBaseName(pszFile));
    if (IsPermBankName(name)) {
        if (nTick != kFirstTick) {
            Error(nTick, kPermNotFirstError);
        }
        const String path = (*mDirectory + kPathSeparator) + pszFile;
        mLoader->AddPermBank(path.c_str());
    } else if (IsSwapBankName(name)) {
        if ((nTick - mIntroTicks) % mTicksPerBar != 0) {
            Error(nTick, FormatString(kNotOnBarFormat, name.c_str()));
        }
        if (name == mLastSwapBank) {
            Error(nTick, FormatString(kTwiceFormat, name.c_str()));
        }
        const String path = (*mDirectory + kPathSeparator) + pszFile;
        mLoader->AddSwapBank(path.c_str(), (nTick - mIntroTicks) / mTicksPerBar);
        mLastSwapBank = name;
    } else {
        Error(nTick, FormatString(kBadNameFormat, pszFile));
    }
}
