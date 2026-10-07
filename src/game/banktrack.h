#pragma once

/**
 * The sound bank changes of a song, from its track named "BANK".
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the track name. Only the
 * members GameLogic uses are declared.
 */
class BankTrack {
public:
    /**
     * Enter the playing state and refresh the current bank.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a028
     * @ghidraAddress PAL: 0x0014b9e8
     */
    void Start();
};
