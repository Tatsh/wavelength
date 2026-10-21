#include "netflow/setuplocalplayertask.h"

#include "netflow/lobbyplayer.h"

void SetupLocalPlayerTask::OnStart() {
    if (mSource->GetRating() != 0) {
        mTarget->SetRating(mSource->GetRating());
        mTarget->SetGames(mSource->GetGames());
        mTarget->SetRecord(mSource->GetRecord());
    }
    Finish(true, 0);
}
