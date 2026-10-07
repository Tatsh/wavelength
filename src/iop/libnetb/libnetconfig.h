#pragma once

/**
 * Settings from the module arguments.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class LibnetConfig {
public:
    /**
     * Set the defaults.
     *
     * @ghidraAddress NTSC-U/C: 0x000011e4
     * @ghidraAddress PAL: 0x000011c4
     */
    void Init();

    /**
     * Print the version, then set the delay between sends from the settings.
     *
     * @ghidraAddress NTSC-U/C: 0x00001154
     * @ghidraAddress PAL: 0x00001134
     */
    void Apply();

    int mSendDelay; /*!< Milliseconds between sends, from `-send_delay=`. Limited to 1 to 10. */
    int mVerbose;   /*!< Set by `-verbose`. Never read. */
};
