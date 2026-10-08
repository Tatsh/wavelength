#include "game/worldtrack.h"

#include <algorithm>

WorldTrack::WorldTrack() : mEvents(kListCount, std::vector<int>()) {
}

WorldTrack::~WorldTrack() {
}

void WorldTrack::Insert(int nTick, char cLetter) {
    std::vector<int> &events = mEvents[cLetter - kFirstLetter];
    if (events.empty() || events.back() < nTick) {
        events.push_back(nTick);
    } else {
        events.insert(std::upper_bound(events.begin(), events.end(), nTick), nTick);
    }
}

std::vector<int> *WorldTrack::GetEvents(char cLetter) {
    return &mEvents[cLetter - kFirstLetter];
}
