#include "met/localizedfreqpanel.h"

#ifdef VIDEO_STANDARD_PAL

#include "os/string.h"
#include "os/system.h"

LocalizedFreqPanel::LocalizedFreqPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir) {
    if (!SystemLanguageIsEnglish()) {
        mFile.Clear();
        // Yes, the binary passes the formatted path as the format.
        mFile.Printf(FormatString("%s/%s_%s.rnd", pszDir, mName, GetSystemLanguage()));
    }
}

#endif
