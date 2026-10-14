#include "netflow/setuplocalplayertask.h"

#include "netflow/lobbyplayer.h"

void SetupLocalPlayerTask::OnStart() {
    if (mSource->mRating != 0) {
        mTarget->mRating = mSource->mRating;
        mTarget->mGames = mSource->mGames;
        mTarget->mRecord = mSource->mRecord;
    }
    Finish(true, 0);
}
