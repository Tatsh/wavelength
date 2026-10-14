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

InputMap::~InputMap() {
}

void InputMap::SetButtonAction(int nButton, int nAction) {
    mButtons[nButton] = nAction;
}

int InputMap::GetButtonAction(int nButton) {
    return mButtons[nButton];
}

void InputMap::SetStickAction(int nStick, int nAction) {
    mSticks[nStick] = nAction;
}

int InputMap::GetStickAction(int nStick) {
    return mSticks[nStick];
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

void InputMap::Save(BinStream &stream) const {
    const unsigned int nButtons = mButtons.size();
    stream.WriteEndian(&nButtons, sizeof(nButtons));
    for (unsigned int i = 0; i < mButtons.size(); ++i) {
        const int nAction = mButtons[i];
        stream.WriteEndian(&nAction, sizeof(nAction));
    }
    const unsigned int nSticks = mSticks.size();
    stream.WriteEndian(&nSticks, sizeof(nSticks));
    for (unsigned int i = 0; i < mSticks.size(); ++i) {
        const int nAction = mSticks[i];
        stream.WriteEndian(&nAction, sizeof(nAction));
    }
}

void InputMap::Load(BinStream &stream) {
    unsigned int nCount;
    stream.ReadEndian(&nCount, sizeof(nCount));
    mButtons.resize(nCount, 0);
    for (unsigned int i = 0; i < mButtons.size(); ++i) {
        int nAction;
        stream.ReadEndian(&nAction, sizeof(nAction));
        mButtons[i] = nAction;
    }
    stream.ReadEndian(&nCount, sizeof(nCount));
    mSticks.resize(nCount, 0);
    for (unsigned int i = 0; i < mSticks.size(); ++i) {
        const int nAction = mSticks[i];
        stream.WriteEndian(&nAction, sizeof(nAction)); // Yes, the binary writes the sticks here.
    }
}

BinStream &operator<<(BinStream &stream, const InputMap &map) {
    map.Save(stream);
    return stream;
}

BinStream &operator>>(BinStream &stream, InputMap &map) {
    map.Load(stream);
    return stream;
}
