#pragma once

#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/gamedb.h"
#include "game/gamelogic.h"
#include "game/playnoteevent.h"
#include "game/rotateevent.h"
#include "game/stickevent.h"
#include "game/world.h"
#include "script/dataarray.h"

/**
 * World of the main game rule set.
 *
 * The RTTI records the class as deriving from World. The object is 0x5c bytes. The world builds a
 * TutorialGameLogic, a SoloGameLogic, or a MultiGameLogic, and while it exists the script commands
 * `play_all_gems`, `practice_mode`, and `no_capture` toggle the matching settings.
 */
class Game : public World {
public:
    /**
     * Construct a game world.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d340
     * @ghidraAddress PAL: 0x0010ea78
     */
    Game();

    /**
     * Turn the software effects off, empty the bank slots, and remove the script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d3a0
     * @ghidraAddress PAL: 0x0010ead8
     */
    ~Game() override;

    /**
     * Report the main game rule set.
     *
     * @return GameDb::kRuleSetGame.
     * @ghidraAddress NTSC-U/C: 0x00336208
     */
    int GetRuleSet() const override {
        return GameDb::kRuleSetGame;
    }

    /**
     * Report whether the game ended without a player abandoning it.
     *
     * @return Whether the game ended.
     * @ghidraAddress NTSC-U/C: 0x0010d5a0
     * @ghidraAddress PAL: 0x0010ecd8
     */
    bool IsFinished() override;

    /**
     * Record a rotation.
     *
     * @param event The rotation.
     * @ghidraAddress NTSC-U/C: 0x0010d610
     * @ghidraAddress PAL: 0x0010ed48
     */
    void Handle(const RotateEvent &event) override;

    /**
     * Record a gem button.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0010d638
     * @ghidraAddress PAL: 0x0010ed70
     */
    void Handle(const PlayNoteEvent &event) override;

    /**
     * Ignore a button event of command 8. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00336210
     */
    void Handle([[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Record a stick event of command 2 while the freestyle track is active.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x0010d660
     * @ghidraAddress PAL: 0x0010ed98
     */
    void Handle(const StickEvent<2> &event) override;

    /**
     * Record a stick event of command 6 while the freestyle track is active.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x0010d6b0
     * @ghidraAddress PAL: 0x0010ede8
     */
    void Handle(const StickEvent<6> &event) override;

    /**
     * Record a button event of command 3.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0010d700
     * @ghidraAddress PAL: 0x0010ee38
     */
    void Handle(const BtnEvent<3> &event) override;

    /**
     * Record a button event of command 4.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0010d728
     * @ghidraAddress PAL: 0x0010ee60
     */
    void Handle(const BtnEvent<4> &event) override;

    /**
     * Record a button event of command 5.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0010d750
     * @ghidraAddress PAL: 0x0010ee88
     */
    void Handle(const BtnEvent<5> &event) override;

    /**
     * Ignore a section change. The body is empty.
     *
     * @param event The section change.
     * @ghidraAddress NTSC-U/C: 0x00336218
     */
    void Handle([[maybe_unused]] const ChangeSectionEvent &event) override {
    }

    /**
     * Ignore a button event of command 9. The body is empty.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00336220
     */
    void Handle([[maybe_unused]] const BtnEvent<9> &event) override {
    }

    /**
     * Record a button event of command 10.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x0010d778
     * @ghidraAddress PAL: 0x0010eeb0
     */
    void Handle(const BtnEvent<10> &event) override;

    /**
     * Load the effect bank and the cheer bank into their synthesiser slots.
     *
     * The "game" section of the configuration supplies the effect bank of the community, the cheer
     * bank, and the effect bank slot. The cheer bank takes the first slot after the song's banks,
     * and the sound memory is divided again to fit it.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d7a0
     * @ghidraAddress PAL: 0x0010eed8
     */
    void BeginAssetLoad() override;

    /**
     * Advance the bank loads. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00336230
     */
    void PollAssetLoad() override {
    }

    /**
     * Report whether the effect bank is loaded.
     *
     * @return Whether the bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x0010dab0
     * @ghidraAddress PAL: 0x0010f248
     */
    bool IsAssetLoadDone() override;

    /**
     * Start the song's track loader.
     *
     * @ghidraAddress NTSC-U/C: 0x0010dae8
     * @ghidraAddress PAL: 0x0010f280
     */
    void BeginSongLoad() override;

    /**
     * Advance the song load. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00336228
     */
    void PollSongLoad() override {
    }

    /**
     * Advance the song's track loader and report whether it finished.
     *
     * @return Whether the song is loaded.
     * @ghidraAddress NTSC-U/C: 0x0010db10
     * @ghidraAddress PAL: 0x0010f2a8
     */
    bool IsSongLoadDone() override;

    /**
     * Stop the song's track loader and start it again.
     *
     * @ghidraAddress NTSC-U/C: 0x0010dba8
     * @ghidraAddress PAL: 0x0010f340
     */
    void BeginEnding() override;

    /**
     * Advance the sequence that follows the end of the game. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0010dbe0
     */
    void PollEnding() override;

    /**
     * Advance the song's track loader and report whether it finished.
     *
     * @return Whether the loader finished.
     * @ghidraAddress NTSC-U/C: 0x0010dbe8
     * @ghidraAddress PAL: 0x0010f380
     */
    bool IsEndingDone() override;

    /**
     * Build the logic of the tutorial, the solo game, or the multiplayer game and install it.
     *
     * A demo or the `zero_rand_seed` setting clears the seed first.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d4a0
     * @ghidraAddress PAL: 0x0010ebd8
     */
    void CreateLogic() override;

    /**
     * Register the script commands of the game.
     *
     * @ghidraAddress NTSC-U/C: 0x0010d440
     * @ghidraAddress PAL: 0x0010eb78
     */
    void OnStart() override;

    /**
     * Build the display of the song's catch tracks.
     *
     * Each catch track adds its instrument and its type to the two lists, and a freestyle track
     * sets the option. A solo game outside the tutorial passes the songs of the group the player
     * cleared as the flags, unless the song is of type 1. A demo also shows the demo text.
     *
     * @param nStartTick The tick the song starts at.
     * @ghidraAddress NTSC-U/C: 0x0010dc10
     * @ghidraAddress PAL: 0x0010f3a8
     */
    void BuildTracks(int nStartTick) override;

    /**
     * Empty the bank slots BeginAssetLoad() filled and forget them.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010db38
     * @ghidraAddress PAL: 0x0010f2d0
     */
    void ReleaseBanks();

    /**
     * Toggle GameConfig::mPlayAllGems, the `play_all_gems` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x0010ef20
     * @ghidraAddress PAL: 0x001106b8
     */
    static void TogglePlayAllGems(DataArray *pCommand, void *pUserData);

    /**
     * Toggle the practice mode of GameDb, the `practice_mode` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x0010ef70
     * @ghidraAddress PAL: 0x00110708
     */
    static void TogglePracticeMode(DataArray *pCommand, void *pUserData);

    /**
     * Toggle GameConfig::mNoCapture, the `no_capture` script command.
     *
     * @param pCommand The command.
     * @param pUserData The value given to ScriptFunction::Register(), null.
     * @ghidraAddress NTSC-U/C: 0x0010efd0
     * @ghidraAddress PAL: 0x00110768
     */
    static void ToggleNoCapture(DataArray *pCommand, void *pUserData);

    GameLogic *mLogic;  /*!< The logic CreateLogic() installed, or null. */
    int mFxBankSlot;    /*!< The synthesiser slot of the effect bank, or -1. */
    int mCheerBankSlot; /*!< The synthesiser slot of the cheer bank, or -1. */
    int mDemo;          /*!< Whether a demo was played when the world was built. */
    int mReserved58;    // +0x58, cleared by the constructor. The purpose is not yet recovered.
};
