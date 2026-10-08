#include "gfx/gfxconfig.h"

#include "os/debug.h"

namespace {

// Look in the section without reporting a miss, then in the defaults with bFail.
template <typename T>
inline bool FindConfigValue(DataArray *pConfig,
                            DataArray *pDefaults,
                            const char *pszName,
                            T *pValue,
                            bool bFail,
                            bool (DataArray::*pfnFind)(const char *, T *, bool) const) {
    if (pConfig != nullptr && (pConfig->*pfnFind)(pszName, pValue, false)) {
        return true;
    }
    if (pDefaults != nullptr) {
        return (pDefaults->*pfnFind)(pszName, pValue, bFail);
    }
    if (bFail) {
        DebugNotify("could not find %s", pszName);
    }
    return false;
}

} // namespace

bool FindConfigArray(DataArray *pConfig,
                     DataArray *pDefaults,
                     const char *pszName,
                     DataArray **ppValue,
                     bool bFail) {
    if (pConfig != nullptr) {
        *ppValue = pConfig->FindArray(pszName, false);
        if (*ppValue != nullptr) {
            return true;
        }
    }
    if (pDefaults != nullptr) {
        *ppValue = pDefaults->FindArray(pszName, bFail);
        return *ppValue != nullptr;
    }
    if (bFail) {
        DebugNotify("could not find %s", pszName);
    }
    return false;
}

bool FindConfigInt(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, int *pnValue, bool bFail) {
    return FindConfigValue(pConfig, pDefaults, pszName, pnValue, bFail, &DataArray::FindInt);
}

bool FindConfigFloat(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, float *pfValue, bool bFail) {
    return FindConfigValue(pConfig, pDefaults, pszName, pfValue, bFail, &DataArray::FindFloat);
}

bool FindConfigBool(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, bool *pbValue, bool bFail) {
    return FindConfigValue<bool>(pConfig, pDefaults, pszName, pbValue, bFail, &DataArray::FindBool);
}

bool FindConfigSymbol(DataArray *pConfig,
                      DataArray *pDefaults,
                      const char *pszName,
                      const char **ppszValue,
                      bool bFail) {
    return FindConfigValue(pConfig, pDefaults, pszName, ppszValue, bFail, &DataArray::FindSymbol);
}

bool FindConfigVector2(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Vector2 *pValue, bool bFail) {
    return FindConfigValue<Vector2>(
        pConfig, pDefaults, pszName, pValue, bFail, &DataArray::FindVector);
}

bool FindConfigVector3(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Vector3 *pValue, bool bFail) {
    return FindConfigValue<Vector3>(
        pConfig, pDefaults, pszName, pValue, bFail, &DataArray::FindVector);
}

bool FindConfigColor(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Color *pValue, bool bFail) {
    return FindConfigValue(pConfig, pDefaults, pszName, pValue, bFail, &DataArray::FindColor);
}
