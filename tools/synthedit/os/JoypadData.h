#pragma once

/**
 * State of one controller.
 *
 * The object is 0x80 bytes. The name is inferred. This build does not poll the controllers, so
 * only the player and the threshold are written after construction, and the held buttons stay
 * clear.
 */
class JoypadData {
public:
    /**
     * Clear the state and mark the controller as without a player.
     *
     * @ghidraAddress 0x1000ed20
     */
    JoypadData();

    int mButtons;        /*!< The held buttons, one bit each. */
    int mReserved04[4];  // +0x04, cleared by the constructor and not yet identified.
    int mReserved14[24]; // +0x14, cleared by the constructor and not yet identified.
    int mPlayerNum;      /*!< The player of the controller, or -1. */
    bool mReserved78;    // +0x78, cleared by the constructor and not yet identified.
    float mThreshold;    /*!< Press threshold of the analog buttons, from the configuration. */
};
