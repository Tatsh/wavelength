#include "game/scripttrackbuilder.h"

#include <cstring>

#include "os/bufstream.h"
#include "os/string.h"

namespace {

constexpr char kCommandNameFormat[] = "SCRIPT track, tick:%d";

} // namespace

ScriptTrackBuilder::ScriptTrackBuilder(const char *pszName,
                                       bool bValidate,
                                       ErrorHandler pfnError,
                                       int nReserved,
                                       ScriptTrackData *pData)
    : TrackBuilder(pszName, bValidate, pfnError), mReserved10(nReserved), mData(pData) {
}

void ScriptTrackBuilder::OnText(int nTick,
                                const char *pszText,
                                [[maybe_unused]] unsigned char nType) {
    BufStream stream(
        const_cast<char *>(pszText), static_cast<int>(strlen(pszText)), true); // Read only.
    DataArray *pCommand = DataArray::Read(FormatString(kCommandNameFormat, nTick), &stream);
    mData->AddCommand(nTick, pCommand);
}
