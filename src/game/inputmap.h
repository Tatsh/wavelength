#pragma once

#include <vector>

#include "script/dataarray.h"

/**
 * The actions a player's controller buttons and analogue sticks are bound to.
 *
 * The class is not polymorphic and has no RTTI. The name comes from the "input_map" configuration
 * entry LoadDefaults() reads. Each player profile has one, and InputMgr reads the bindings of
 * every pad.
 */
class InputMap {
public:
    /** The number of controller buttons the map binds. */
    static constexpr int kButtonCount = 24;

    /** The number of analogue sticks the map binds. */
    static constexpr int kStickCount = 2;

    /**
     * Build a map with every button and stick binding cleared.
     *
     * @ghidraAddress NTSC-U/C: 0x0027d5c0
     * @ghidraAddress PAL: 0x00286ed8
     */
    InputMap();

    /**
     * Construct a copy of another map.
     *
     * @param other The map to copy.
     * @ghidraAddress NTSC-U/C: 0x0027d700
     * @ghidraAddress PAL: 0x00287018
     */
    InputMap(const InputMap &other);

    /**
     * Copy the button and stick bindings of another map.
     *
     * @param other The map to copy.
     * @return The map.
     * @ghidraAddress NTSC-U/C: 0x0027d9f0
     * @ghidraAddress PAL: 0x00287308
     */
    InputMap &operator=(const InputMap &other);

    /**
     * Report the action a controller button is bound to.
     *
     * @param nButton The button.
     * @return The action.
     * @ghidraAddress NTSC-U/C: 0x0027dad0
     * @ghidraAddress PAL: 0x002873e8
     */
    int GetButtonAction(int nButton);

    /**
     * Bind a controller button to an action.
     *
     * @param nButton The button.
     * @param nAction The action, or 0 to clear the binding.
     * @ghidraAddress NTSC-U/C: 0x0027dab8
     * @ghidraAddress PAL: 0x002873d0
     */
    void SetButtonAction(int nButton, int nAction);

    /**
     * Bind an analogue stick to an action.
     *
     * @param nStick The stick.
     * @param nAction The action.
     * @ghidraAddress NTSC-U/C: 0x0027dae8
     * @ghidraAddress PAL: 0x00287400
     */
    void SetStickAction(int nStick, int nAction);

    /**
     * Report the action an analogue stick is bound to.
     *
     * @param nStick The stick.
     * @return The action.
     * @ghidraAddress NTSC-U/C: 0x0027db00
     * @ghidraAddress PAL: 0x00287418
     */
    int GetStickAction(int nStick);

    /**
     * Find the analogue stick bound to an action.
     *
     * @param nAction The action.
     * @return The stick, or the number of sticks when no stick is bound to the action.
     * @ghidraAddress NTSC-U/C: 0x0027db18
     * @ghidraAddress PAL: 0x00287430
     */
    int FindStick(int nAction);

    /**
     * Load the bindings a configuration array lists.
     *
     * Each node after the tag of the "buttons" and "sticks" child arrays is a pair of a button or
     * stick and the action bound to it. The title is inferred.
     *
     * @param pConfig The configuration array.
     * @ghidraAddress NTSC-U/C: 0x0027db58
     * @ghidraAddress PAL: 0x00287470
     */
    void Load(const DataArray *pConfig);

    /**
     * Load the bindings the "input_map" entry of the "db" configuration section lists.
     *
     * @ghidraAddress NTSC-U/C: 0x0027dc78
     * @ghidraAddress PAL: 0x00287590
     */
    void LoadDefaults();

private:
    std::vector<int> mButtons; // The action of each button, indexed by button.
    std::vector<int> mSticks;  // The action of each stick, indexed by stick.
};
