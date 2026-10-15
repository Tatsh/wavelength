#include "met/localizedpanel.h"

#ifdef VIDEO_STANDARD_PAL

#include "os/string.h"
#include "os/system.h"

LocalizedPanel::LocalizedPanel(DataArray *pData, const char *pszDir) : UIPanel(pData, pszDir) {
    if (!SystemLanguageIsEnglish()) {
        mFile.Clear();
        // Yes, the binary passes the formatted path as the format.
        mFile.Printf(FormatString("%s/%s_%s.rnd", pszDir, mName, GetSystemLanguage()));
    }
}

#endif
