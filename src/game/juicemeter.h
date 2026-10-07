#pragma once

/**
 * Energy of a solo player, which falls while the player misses phrases and ends the song at 0.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Only the members its callers
 * here use are declared.
 */
class JuiceMeter {
public:
    /**
     * Construct a meter at a value.
     *
     * The maximum is the initial juice of the skill level times GameConfig::mJuiceMeterMax.
     *
     * @param fValue The value.
     * @ghidraAddress NTSC-U/C: 0x00151660
     * @ghidraAddress PAL: 0x00152ea8
     */
    explicit JuiceMeter(float fValue);

    /**
     * Set the value, limited to the maximum, and record it in the session log.
     *
     * @param fValue The value.
     * @ghidraAddress NTSC-U/C: 0x001516d0
     * @ghidraAddress PAL: 0x00152f18
     */
    void Set(float fValue);

    /**
     * Add to the value.
     *
     * @param fAmount The amount, negative to subtract.
     * @ghidraAddress NTSC-U/C: 0x00151730
     * @ghidraAddress PAL: 0x00152f78
     */
    void Add(float fAmount);

    /**
     * Report the value.
     *
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x00151778
     * @ghidraAddress PAL: 0x00152fc0
     */
    float Get() const;

    /**
     * Report the maximum.
     *
     * @return The maximum.
     * @ghidraAddress NTSC-U/C: 0x00151780
     * @ghidraAddress PAL: 0x00152fc8
     */
    float GetMax() const;

private:
    /**
     * Show the value on the energy meter of the first player.
     *
     * The display also learns whether the juice has run out.
     *
     * @ghidraAddress NTSC-U/C: 0x00151788
     * @ghidraAddress PAL: 0x00152fd0
     */
    void UpdateDisplay();

    /**
     * Warn once that the juice is low when a change brings it below a sixth of the maximum.
     *
     * A value above a third of the maximum allows the warning again.
     *
     * @param fOld The value before the change. The body does not read it.
     * @param fNew The value after the change.
     * @ghidraAddress NTSC-U/C: 0x00151800
     * @ghidraAddress PAL: 0x00153048
     */
    void CheckLow(float fOld, float fNew);

    float mValue; /*!< The value. */
    float mMax;   /*!< The maximum. */
    int mWarned;  /*!< Whether the low juice warning played since the value was last high. */
};
