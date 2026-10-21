#include "netflow/setlocparamstask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "os/system.h"

namespace {

// The language keys GetSystemLanguage() reports, with the language each tells the server.
struct LanguageCode {
    const char *pszLanguage;
    int nCode;
};

constexpr LanguageCode kLanguageCodes[] = {
    {"french", RT_MEDIUS_LANGUAGE_FRENCH},
    {"spanish", RT_MEDIUS_LANGUAGE_SPANISH},
    {"italian", RT_MEDIUS_LANGUAGE_ITALIAN},
    {"german", RT_MEDIUS_LANGUAGE_GERMAN},
};

} // namespace

void SetLocParamsTask::OnStart() {
    rtMediusSetLocalizationParamsRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.characterEncoding = RT_MEDIUS_CHARACTER_ENCODING_UTF8;
    const char *pszLanguage = GetSystemLanguage();
    request.language = RT_MEDIUS_LANGUAGE_US_ENGLISH;
    for (const auto &code : kLanguageCodes) {
        if (strcmp(pszLanguage, code.pszLanguage) == 0) {
            request.language = code.nCode;
            break;
        }
    }
    const int nError = rtMediusSetLocalizationParams(&request, OnResponse);
    LobbyRT::ReportMediusError(894, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void SetLocParamsTask::OnResponse(int nConnection,
                                  int nType,
                                  int nSize,
                                  const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask != nullptr) {
        pTask->Finish(true, 0);
    }
}
