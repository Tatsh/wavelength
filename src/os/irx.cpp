#include "os/irx.h"

#include <cstring>

#include <sifdev.h>

#include "os/debug.h"
#include "os/file.h"
#include "os/system.h"

namespace {

// Bytes of a module path or of one argument.
constexpr int kPathSize = 128;

// Bytes of a module's argument block.
constexpr int kArgumentsSize = 256;

// The character IrxLoadModule() replaces with FileSourceLetter().
constexpr char kSourcePlaceholder = '%';

// The result IrxLoadMatching() reports when no module matches.
constexpr int kNoModule = -1;

void FormatArgument(char *pszDest, int nSize, DataArray::Node value, int nType);

// Expand a `(concat ...)` or `(file ...)` directive.
// NTSC-U/C: 0x0028a4b8, PAL: 0x00293cb0
void FormatDirective(char *pszDest, int nSize, const DataArray *pDirective) {
    const char *pszCommand = pDirective->Sym(0);
    if (strcmp(pszCommand, "concat") == 0) {
        pszDest[0] = '\0';
        for (int i = 1; i < pDirective->Size(); ++i) {
            char szPart[kPathSize];
            FormatArgument(szPart, kPathSize, pDirective->Value(i), pDirective->Type(i));
            (void)strlen(pszDest); // Yes, the binary discards both lengths.
            (void)strlen(szPart);
            strcat(pszDest, szPart);
        }
    } else if (strcmp(pszCommand, "file") == 0) {
        FormatDevicePath(pszDest, nSize, pDirective->Sym(1));
    } else {
        DebugWarn("Illegal command in irx argument parser:%s", pszCommand);
    }
}

// Expand one argument node.
// NTSC-U/C: 0x0028a440, PAL: 0x00293c38
void FormatArgument(char *pszDest, int nSize, DataArray::Node value, int nType) {
    if (nType == DataArray::kNodeSymbol) {
        (void)strlen(value.mSymbol); // Yes, the binary discards the length.
        strcpy(pszDest, value.mSymbol);
    } else if (nType == DataArray::kNodeArray) {
        FormatDirective(pszDest, nSize, value.mArray);
    } else {
        DebugWarn("irx arg must be be either string or command");
    }
}

} // namespace

int IrxLoadModule(const DataArray *pModule) {
    const bool bUsingCD = UsingCD();
    if (g_bHostConfig) {
        SetUsingCD(false);
    }
    char szName[kPathSize];
    strcpy(szName, pModule->Sym(0));
    for (char *p = szName; *p != '\0'; ++p) {
        if (*p == kSourcePlaceholder) {
            *p = FileSourceLetter();
        }
    }
    char szPath[kPathSize];
    FormatDevicePath(szPath, kPathSize, szName);

    char szArguments[kArgumentsSize];
    int nLength = 0;
    for (int i = 1; i < pModule->Size(); ++i) {
        FormatArgument(szName, kPathSize, pModule->Value(i), pModule->Type(i));
        strcpy(szArguments + nLength, szName);
        nLength += static_cast<int>(strlen(szName)) + 1;
    }
    for (int i = nLength; i > 0; --i) {
        // Yes, the binary spins once for each argument byte.
    }
    const int nResult = sceSifLoadModule(szPath, nLength, nLength != 0 ? szArguments : nullptr);
    SetUsingCD(bUsingCD);
    return nResult;
}

void IrxLoadAll(const DataArray *pModules) {
    for (int i = 1; i < pModules->Size(); ++i) {
        IrxLoadModule(pModules->Array(i));
    }
}

int IrxLoadMatching(const DataArray *pModules, const char *pszName) {
    for (int i = 1; i < pModules->Size(); ++i) {
        const DataArray *pModule = pModules->Array(i);
        if (strstr(pModule->Sym(0), pszName) != nullptr) {
            return IrxLoadModule(pModule);
        }
    }
    return kNoModule;
}
