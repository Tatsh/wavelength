#include "utl/OptionProcessor.h"

#include <cstdlib>
#include <cstring>

#include "os/Debug.h"
#include "utl/BoolOption.h"
#include "utl/MemMgr.h"
#include "utl/StringOption.h"

namespace {

// ParseCommandLine() keeps at most this many arguments, in a vector in front of its copy of the
// command line.
const int kMaxArgs = 20;
const int kArgVectorBytes = kMaxArgs * sizeof(char *);

} // namespace

OptionProcessor::~OptionProcessor() {
    std::list<Option *>::iterator it;
    for (it = mOptions.begin(); it != mOptions.end(); ++it) {
        delete *it;
    }
}

void OptionProcessor::AddBoolOption(const char *name, bool *value, bool setValue) {
    mOptions.push_back(new BoolOption(name, value, setValue));
}

void OptionProcessor::AddStringOption(const char *name, String *value) {
    mOptions.push_back(new StringOption(name, value));
}

void OptionProcessor::Process(int argc, char **argv) {
    ASSERT(argv);
    bool showOptions = false;
    // Option::Matches() skips the dash of the argument. The option is given as `--options`.
    AddBoolOption("-options", &showOptions, true);
    mArgc = argc;
    mArgv = argv;
    mCurArg = 1;
    gOptionArgv = argv;
    std::list<Option *>::iterator it;
    while (mCurArg < mArgc && argv[mCurArg][0] == '-') {
        for (it = mOptions.begin(); it != mOptions.end(); ++it) {
            if ((*it)->Matches(mCurArg)) {
                (*it)->Set(mCurArg);
                break;
            }
        }
        if (it == mOptions.end()) {
            DebugPrint("OptionProcessor: Didn't recognize option %s\n", argv[mCurArg]);
            ++mCurArg;
        }
    }
    if (showOptions) {
        for (it = mOptions.begin(); it != mOptions.end(); ++it) {
            (*it)->PrintUsage();
        }
        exit(0);
    }
}

char **OptionProcessor::ParseCommandLine(const char *commandLine, int *argc) {
    ASSERT(commandLine);
    char **argv = static_cast<char **>(
        MemAlloc(static_cast<int>(strlen(commandLine) + 1 + kArgVectorBytes), "char**", 0));
    char *c = reinterpret_cast<char *>(argv) + kArgVectorBytes;
    strcpy(c, commandLine);
    bool inQuote = false;
    bool newArg = true;
    *argc = 0;
    while (*c) {
        if (!inQuote && *c == ' ') {
            *c++ = '\0';
            newArg = true;
        } else if (*c == '"') {
            *c++ = '\0';
            inQuote = !inQuote;
            if (inQuote) {
                newArg = false;
                argv[(*argc)++] = c;
            } else {
                newArg = true;
            }
        } else {
            if (newArg) {
                newArg = false;
                argv[(*argc)++] = c;
            }
            ++c;
        }
        if (*argc == kMaxArgs) {
            TheDebug.Notify("Option: max args reached");
            break;
        }
    }
    return argv;
}
