#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that reads the news of the lobby server into LobbyRT::mAnnouncement.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d53c8`. The
 * step always succeeds.
 */
class GetAnnouncementsTask : public NetTask {
public:
    /**
     * Take the news and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response, whose text is edited in place.
     * @ghidraAddress NTSC-U/C: 0x00264618
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, rtMediusAnnouncementResponse *pResponse);

    /**
     * Replace each `<br>` of a text by a line break, in place.
     *
     * The name is inferred.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x002673a8
     * @ghidraAddress PAL: 0x00271080
     */
    static void ReplaceBreaks(char *pszText);

protected:
    /**
     * Clear the news and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00264560
     */
    void OnStart() override;
};
