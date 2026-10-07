#include "game/netfaker.h"

#include "game/gamedb.h"
#include "game/player.h"
#include "os/scheduler.h"

void NetFaker::Start() {
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCommand.Get(), mCursor.GetTick(), false);
    }
}

void NetFaker::Stop() {
    TheSongScheduler.Cancel(mUpdateCommand.Get());
}

void NetFaker::SetCursor(const GemCursor &cursor) {
    TheSongScheduler.Cancel(mUpdateCommand.Get());
    mCursor = cursor;
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCommand.Get(), mCursor.GetTick(), false);
    }
}

void NetFaker::Update() {
    const int nBar = mCursor.GetTick() / mReactor->GetTicksPerBar();
    Player *pPlayer = mReactor->GetPlayer();
    if (pPlayer != nullptr && !TheGameDb->IsLocalPlayer(pPlayer->GetIndex()) &&
        !pPlayer->IsAborted() && mReactor->IsBarActive(nBar)) {
        if (nBar == mLastBar && mCountMisses && !pPlayer->GetCatching()) {
            mReactor->MissGem(mCursor.GetTick(), mCursor, true);
        } else {
            mReactor->HitGem(mCursor.GetTick(), mCursor, true);
        }
    }

    (void)mCursor.Next(); // Yes, the binary discards the copy.
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCommand.Get(), mCursor.GetTick(), false);
    }
    mLastBar = nBar;
}
