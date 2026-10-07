#pragma once

#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/duellogic.h"
#include "game/gamedb.h"
#include "game/playnoteevent.h"
#include "game/rotateevent.h"
#include "game/stickevent.h"
#include "game/world.h"

/**
 * World of the duel rule set.
 *
 * The RTTI records the class as deriving from World. The object is 0x50 bytes. The input events
 * the duel acts on are recorded as serializable input commands, and DuelLogic acts on them when
 * the commands run.
 */
class Duel : public World {
public:
    /**
     * Construct a duel world.
     *
     * @ghidraAddress NTSC-U/C: 0x00104fb0
     * @ghidraAddress PAL: 0x00106698
     */
    Duel();

    /**
     * Release the world.
     *
     * @ghidraAddress NTSC-U/C: 0x00104ff0
     * @ghidraAddress PAL: 0x001066d8
     */
    ~Duel() override;

    /**
     * Report the duel rule set.
     *
     * @return GameDb::kRuleSetDuel.
     * @ghidraAddress NTSC-U/C: 0x00333f28
     * @ghidraAddress PAL: 0x003a14d8
     */
    int GetRuleSet() const override {
        return GameDb::kRuleSetDuel;
    }

    /**
     * Report whether the duel ended without a player abandoning it.
     *
     * @return Whether the duel ended.
     * @ghidraAddress NTSC-U/C: 0x001052b8
     * @ghidraAddress PAL: 0x001069f0
     */
    bool IsFinished() override;

    /**
     * Record a rotation.
     *
     * @param event The rotation.
     * @ghidraAddress NTSC-U/C: 0x00105940
     * @ghidraAddress PAL: 0x00107078
     */
    void Handle(const RotateEvent &event) override;

    /**
     * Record a gem button.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00105968
     * @ghidraAddress PAL: 0x001070a0
     */
    void Handle(const PlayNoteEvent &event) override;

    /**
     * Ignore a button event of command 8. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00333f30
     * @ghidraAddress PAL: 0x003a14e0
     */
    void Handle([[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Record a stick event of command 2.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x00105990
     * @ghidraAddress PAL: 0x001070c8
     */
    void Handle(const StickEvent<2> &event) override;

    /**
     * Record a stick event of command 6.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x001059b8
     * @ghidraAddress PAL: 0x001070f0
     */
    void Handle(const StickEvent<6> &event) override;

    /**
     * Record a button event of command 3.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x001059e0
     * @ghidraAddress PAL: 0x00107118
     */
    void Handle(const BtnEvent<3> &event) override;

    /**
     * Record a button event of command 4.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00105a08
     * @ghidraAddress PAL: 0x00107140
     */
    void Handle(const BtnEvent<4> &event) override;

    /**
     * Record a button event of command 5.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00105a30
     * @ghidraAddress PAL: 0x00107168
     */
    void Handle(const BtnEvent<5> &event) override;

    /**
     * Ignore a section change. The body is empty.
     *
     * @param event The section change.
     * @ghidraAddress NTSC-U/C: 0x00333f38
     * @ghidraAddress PAL: 0x003a14e8
     */
    void Handle([[maybe_unused]] const ChangeSectionEvent &event) override {
    }

    /**
     * Ignore a button event of command 9. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00333f40
     * @ghidraAddress PAL: 0x003a14f0
     */
    void Handle([[maybe_unused]] const BtnEvent<9> &event) override {
    }

    /**
     * Ignore a button event of command 10. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00333f48
     * @ghidraAddress PAL: 0x003a14f8
     */
    void Handle([[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Load the effect bank of the duel into its synthesiser slot.
     *
     * The "game" section of the configuration supplies the bank file as "remix_fx_bank_file" and
     * the slot as "game_fx_bank_slot". The slot is emptied first.
     *
     * @ghidraAddress NTSC-U/C: 0x00105120
     */
    void BeginAssetLoad() override;

    /**
     * Advance the effect bank load. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00333f58
     * @ghidraAddress PAL: 0x003a1508
     */
    void PollAssetLoad() override {
    }

    /**
     * Report whether the effect bank is loaded.
     *
     * @return Whether the bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x00105210
     * @ghidraAddress PAL: 0x00106948
     */
    bool IsAssetLoadDone() override;

    /**
     * Start the song's track loader.
     *
     * @ghidraAddress NTSC-U/C: 0x001050d0
     * @ghidraAddress PAL: 0x001067b8
     */
    void BeginSongLoad() override;

    /**
     * Advance the song load. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00333f50
     * @ghidraAddress PAL: 0x003a1500
     */
    void PollSongLoad() override {
    }

    /**
     * Advance the song's track loader and report whether it finished.
     *
     * @return Whether the song is loaded.
     * @ghidraAddress NTSC-U/C: 0x001050f8
     * @ghidraAddress PAL: 0x001067e0
     */
    bool IsSongLoadDone() override;

    /**
     * Stop the song's track loader and start it again.
     *
     * @ghidraAddress NTSC-U/C: 0x00105248
     * @ghidraAddress PAL: 0x00106980
     */
    void BeginEnding() override;

    /**
     * Advance the sequence that follows the end of the duel. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00105280
     * @ghidraAddress PAL: 0x001069b8
     */
    void PollEnding() override;

    /**
     * Advance the song's track loader and report whether it finished.
     *
     * @return Whether the loader finished.
     * @ghidraAddress NTSC-U/C: 0x00105288
     * @ghidraAddress PAL: 0x001069c0
     */
    bool IsEndingDone() override;

    /**
     * Build a DuelLogic for the song and install it.
     *
     * @ghidraAddress NTSC-U/C: 0x00105048
     * @ghidraAddress PAL: 0x00106730
     */
    void CreateLogic() override;

    /**
     * Finish starting the song. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x001052b0
     * @ghidraAddress PAL: 0x001069e8
     */
    void OnStart() override;

    /**
     * Build the display of the two players' tracks.
     *
     * @param nStartTick The tick the song starts at.
     * @ghidraAddress NTSC-U/C: 0x00105328
     * @ghidraAddress PAL: 0x00106a60
     */
    void BuildTracks(int nStartTick) override;

    /**
     * Install the logic in the world and keep it in mLogic.
     *
     * @param pLogic The logic to install.
     * @ghidraAddress NTSC-U/C: 0x00105098
     * @ghidraAddress PAL: 0x00106780
     */
    void SetLogic(DuelLogic *pLogic);

    DuelLogic *mLogic; /*!< The logic CreateLogic() installed, or null. */
    int mFxBankSlot;   /*!< The synthesiser slot of the effect bank, -1 until it is read. */
};
