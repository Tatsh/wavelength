#pragma once

/**
 * The sound bank changes of a song, from its track named "BANK".
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the track name. The
 * first word records the state, 1 while the banks load, 2 once they are loaded, and 3 while the
 * song plays. Only the members its callers here use are declared.
 */
class BankTrack {
public:
    /**
     * Start loading the sound banks.
     *
     * @ghidraAddress NTSC-U/C: 0x00149e90
     * @ghidraAddress PAL: 0x0014b850
     */
    void Load();

    /**
     * Advance the load and report whether it finished.
     *
     * @return Whether every bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x00149f88
     * @ghidraAddress PAL: 0x0014b948
     */
    bool PollLoad();

    /**
     * Enter the playing state and refresh the current bank.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a028
     * @ghidraAddress PAL: 0x0014b9e8
     */
    void Start();

    /**
     * Stop a load that is under way.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a058
     * @ghidraAddress PAL: 0x0014ba18
     */
    void Stop();

    /**
     * Report the first synthesiser bank slot after the slots the load fills.
     *
     * The name is inferred.
     *
     * @return The slot.
     * @ghidraAddress NTSC-U/C: 0x0014a0e8
     * @ghidraAddress PAL: 0x0014baa8
     */
    unsigned short GetEndBankSlot();
};
