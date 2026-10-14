#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class String;

/**
 * Step that passes a text through the word filter of the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5698` and the
 * object is 0x18 bytes. The step fails with CreateAccountResultMsg::kResultVulgarName when the
 * request cannot be sent.
 */
class TextFilterTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bb50
     */
    ~TextFilterTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pText The text.
     * @param pFiltered Receives the filtered text.
     * @param nReplaceStars Non-zero to write `#` for each `*` of the filtered text.
     */
    TextFilterTask(String *pText, String *pFiltered, int nReplaceStars)
        : mText(pText), mFiltered(pFiltered), mReplaceStars(nReplaceStars) {
    }

    /**
     * Take the filtered text and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263828
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusTextFilterResponse *pResponse);

    String *mText;     /*!< The text. */
    String *mFiltered; /*!< Receives the filtered text. */
    int mReplaceStars; /*!< Non-zero to write `#` for each `*` of the filtered text. */

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00263770
     */
    void OnStart() override;
};
