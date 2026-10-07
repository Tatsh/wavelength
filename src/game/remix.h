#pragma once

#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/gamedb.h"
#include "game/playnoteevent.h"
#include "game/remixlogic.h"
#include "game/rotateevent.h"
#include "game/stickevent.h"
#include "game/world.h"

/**
 * World of the remix rule set.
 *
 * The RTTI records the class as deriving from World. The object is 0x54 bytes. The input events
 * the remix acts on are recorded as serializable input commands, and RemixLogic acts on them when
 * the commands run.
 */
class Remix : public World {
public:
    /**
     * Construct a remix world.
     *
     * @ghidraAddress NTSC-U/C: 0x0012eff8
     * @ghidraAddress PAL: 0x001307d0
     */
    Remix();

    /**
     * Unload the effect bank and release the world.
     *
     * @ghidraAddress NTSC-U/C: 0x0012f040
     * @ghidraAddress PAL: 0x00130818
     */
    ~Remix() override;

    /**
     * Report the remix rule set.
     *
     * @return GameDb::kRuleSetRemix.
     * @ghidraAddress NTSC-U/C: 0x0033e0a0
     */
    int GetRuleSet() const override {
        return GameDb::kRuleSetRemix;
    }

    /**
     * Report whether the remix ended without a player abandoning it.
     *
     * @return Whether the remix ended.
     * @ghidraAddress NTSC-U/C: 0x0012f0f0
     * @ghidraAddress PAL: 0x001308c8
     */
    bool IsFinished() override;

    /**
     * Record a rotation.
     *
     * @param event The rotation.
     * @ghidraAddress NTSC-U/C: 0x0012f160
     * @ghidraAddress PAL: 0x00130938
     */
    void Handle(const RotateEvent &event) override;

    /**
     * Record a gem button.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0012f188
     * @ghidraAddress PAL: 0x00130960
     */
    void Handle(const PlayNoteEvent &event) override;

    /**
     * Record a button event of command 8.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0012f1b0
     * @ghidraAddress PAL: 0x00130988
     */
    void Handle(const BtnEvent<8> &event) override;

    /**
     * Record a stick event of command 2.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x0012f1d8
     * @ghidraAddress PAL: 0x001309b0
     */
    void Handle(const StickEvent<2> &event) override;

    /**
     * Ignore a stick event of command 6. The body is empty.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x0012f200
     */
    void Handle(const StickEvent<6> &event) override;

    /**
     * Record a button event of command 3.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0012f208
     * @ghidraAddress PAL: 0x001309e0
     */
    void Handle(const BtnEvent<3> &event) override;

    /**
     * Ignore a button event of command 4. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0033e0a8
     */
    void Handle([[maybe_unused]] const BtnEvent<4> &event) override {
    }

    /**
     * Ignore a button event of command 5. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0033e0b0
     */
    void Handle([[maybe_unused]] const BtnEvent<5> &event) override {
    }

    /**
     * Record a section change.
     *
     * @param event The section change.
     * @ghidraAddress NTSC-U/C: 0x0012f230
     * @ghidraAddress PAL: 0x00130a08
     */
    void Handle(const ChangeSectionEvent &event) override;

    /**
     * Record a button event of command 9.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0012f258
     * @ghidraAddress PAL: 0x00130a30
     */
    void Handle(const BtnEvent<9> &event) override;

    /**
     * Record a button event of command 10.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0012f280
     * @ghidraAddress PAL: 0x00130a58
     */
    void Handle(const BtnEvent<10> &event) override;

    /**
     * Load the effect bank of the remix into its synthesiser slot.
     *
     * The "game" section of the configuration supplies the bank file as "remix_fx_bank_file" and
     * the slot as "game_fx_bank_slot". The slot is emptied first.
     *
     * @ghidraAddress NTSC-U/C: 0x0012f2f8
     */
    void BeginAssetLoad() override;

    /**
     * Advance the effect bank load. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0033e0c0
     */
    void PollAssetLoad() override {
    }

    /**
     * Report whether the effect bank is loaded.
     *
     * @return Whether the bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x0012f3e8
     * @ghidraAddress PAL: 0x00130c10
     */
    bool IsAssetLoadDone() override;

    /**
     * Start the song's track loader.
     *
     * @ghidraAddress NTSC-U/C: 0x0012f2a8
     * @ghidraAddress PAL: 0x00130a80
     */
    void BeginSongLoad() override;

    /**
     * Advance the song load. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0033e0b8
     */
    void PollSongLoad() override {
    }

    /**
     * Advance the song's track loader and report whether it finished.
     *
     * @return Whether the song is loaded.
     * @ghidraAddress NTSC-U/C: 0x0012f2d0
     * @ghidraAddress PAL: 0x00130aa8
     */
    bool IsSongLoadDone() override;

    /**
     * Build a RemixLogic for the song and install it.
     *
     * @ghidraAddress NTSC-U/C: 0x0012f0a0
     * @ghidraAddress PAL: 0x00130878
     */
    void CreateLogic() override;

    /**
     * Build the display of the song's tracks.
     *
     * Each track of kind 2 or 4 adds its instrument to the first list and its kind to the second.
     * The last track of kind 1 or 3 supplies the option.
     *
     * @param nStartTick The tick the song starts at.
     * @ghidraAddress NTSC-U/C: 0x0012f468
     * @ghidraAddress PAL: 0x00130c90
     */
    void BuildTracks(int nStartTick) override;

    /**
     * Empty the synthesiser slot of the effect bank.
     *
     * @ghidraAddress NTSC-U/C: 0x0012f420
     * @ghidraAddress PAL: 0x00130c48
     */
    void UnloadFxBank();

    int mReserved48;    // +0x48, set to 2 and not read by any routine here.
    RemixLogic *mLogic; /*!< The logic CreateLogic() installed, or null. */
    int mFxBankSlot;    /*!< The synthesiser slot of the effect bank, -1 until it is read. */
};
