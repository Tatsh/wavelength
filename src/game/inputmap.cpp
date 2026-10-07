#include "game/inputmap.h"

#include <algorithm>

#include "os/system.h"

InputMap::InputMap() : mButtons(kButtonCount, 0), mSticks(kStickCount, 0) {
}

InputMap::InputMap(const InputMap &other) : mButtons(), mSticks() {
    mButtons.resize(other.mButtons.size(), 0);
    for (unsigned int i = 0; i < mButtons.size(); ++i) {
        mButtons[i] = other.mButtons[i];
    }
    mSticks.resize(other.mSticks.size(), 0);
    for (unsigned int i = 0; i < mSticks.size(); ++i) {
        mSticks[i] = other.mSticks[i];
    }
}

InputMap &InputMap::operator=(const InputMap &other) {
    if (this != &other) {
        // The sizes of this map bound both copies.
        for (unsigned int i = 0; i < mButtons.size(); ++i) {
            mButtons[i] = other.mButtons[i];
        }
        for (unsigned int i = 0; i < mSticks.size(); ++i) {
            mSticks[i] = other.mSticks[i];
        }
    }
    return *this;
}

int InputMap::GetButtonAction(int nButton) {
    return mButtons[nButton];
}

int InputMap::FindStick(int nAction) {
    return std::find(mSticks.begin(), mSticks.end(), nAction) - mSticks.begin();
}

void InputMap::Load(const DataArray *pConfig) {
    const DataArray *pButtons = pConfig->FindArray("buttons", false);
    for (int i = 1; i < pButtons->Size(); ++i) {
        const DataArray *pEntry = pButtons->Array(i);
        mButtons[pEntry->Int(0)] = pEntry->Int(1);
    }

    const DataArray *pSticks = pConfig->FindArray("sticks", false);
    for (int i = 1; i < pSticks->Size(); ++i) {
        const DataArray *pEntry = pSticks->Array(i);
        mSticks[pEntry->Int(0)] = pEntry->Int(1);
    }
}

void InputMap::LoadDefaults() {
    Load(SystemConfig()->FindArray("db", false)->FindArray("input_map", false));
}
