#include "os/optionprocessor.h"

#include <stdlib.h>

#include "os/booloption.h"
#include "os/debug.h"
#include "os/stringoption.h"

namespace {

// The first argument after the program.
constexpr int kFirstArgument = 1;

} // namespace

void OptionProcessor::AddBool(const char *pszName, int *pnTarget, int nValue) {
    mOptions.push_back(std::make_unique<BoolOption>(pszName, pnTarget, nValue));
}

void OptionProcessor::AddString(const char *pszName, String *pTarget) {
    mOptions.push_back(std::make_unique<StringOption>(pszName, pTarget));
}

void OptionProcessor::Process(int argc, char **argv) {
    int bShowOptions = 0;
    // The name has its own `-`, so the command line writes the option as `--options`.
    AddBool("-options", &bShowOptions, 1);
    mArgc = argc;
    mIndex = kFirstArgument;
    mArgv = argv;
    g_pCommandLineArgv = argv;
    if (kFirstArgument < argc && argv[kFirstArgument][0] == '-') {
        do {
            auto it = mOptions.begin();
            for (; it != mOptions.end(); ++it) {
                if ((*it)->Match(&mIndex)) {
                    (*it)->Parse(&mIndex);
                    break;
                }
            }
            if (it == mOptions.end()) {
#ifdef VIDEO_STANDARD_PAL
                DebugPrint("OptionProcessor: Didn't recognize option %s\n", argv[mIndex]);
#else
                DebugError("OptionProcessor: Didn't recognize option %s\n", argv[mIndex]);
#endif
                ++mIndex;
            }
        } while (mIndex < mArgc && argv[mIndex][0] == '-');
    }
    if (bShowOptions != 0) {
        for (const auto &pOption : mOptions) {
            pOption->PrintUsage();
        }
        exit(0);
    }
}

OptionProcessor::~OptionProcessor() = default;
