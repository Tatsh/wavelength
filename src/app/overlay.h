#pragma once

#include <vector>

#include "app/hudcommon.h"
#include "app/msgsink.h"
#include "app/ovyallplayer.h"
#include "app/ovylocalplayer.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "msg/message.h"
#include "rnd/cam.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Head-up display of a game, drawn over the tunnel.
 *
 * The RTTI records the class as deriving from MsgSink. The vtable is at `0x003d19d8`, and the
 * object is 0x50 bytes. The display keeps the parts of the whole game in one HudCommon, the parts
 * of each player on this console in an OvyLocalPlayer, and the score and the avatar of every player
 * in an OvyAllPlayer. A duel orders its players by slot, and any other game puts the local
 * players first.
 */
class Overlay : public MsgSink {
public:
    /**
     * Build the display of the game about to start.
     *
     * The layout is `_hud<letter>.view`, which joins `hud.view`. Every child of the layout but
     * `hud.cam` and `hud.env` starts hidden, and the camera draws into the depth range sHudZ.
     *
     * @param pConfig The configuration section of the game mode.
     * @param pDefaults The section of defaults.
     * @ghidraAddress NTSC-U/C: 0x001c39c8
     * @ghidraAddress PAL: 0x001cc768
     */
    Overlay(DataArray *pConfig, DataArray *pDefaults);

    /**
     * Destroy the display and turn the screen blur off.
     *
     * @ghidraAddress NTSC-U/C: 0x001c4458
     * @ghidraAddress PAL: 0x001cd1f8
     */
    ~Overlay() override;

    /**
     * Ignore a message.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001c60a0
     * @ghidraAddress PAL: 0x001cee40
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Build the picture of the controller and the panel of text a tutorial uses.
     *
     * @ghidraAddress NTSC-U/C: 0x001c45a0
     * @ghidraAddress PAL: 0x001cd340
     */
    void CreateTutorialParts();

    /**
     * Find the local parts of a player.
     *
     * @param nPlayer The player.
     * @return The parts, or null for a player not on this console.
     * @ghidraAddress NTSC-U/C: 0x001c4698
     * @ghidraAddress PAL: 0x001cd438
     */
    OvyLocalPlayer *FindLocal(int nPlayer);

    /**
     * Find the parts every player has.
     *
     * @param nPlayer The player.
     * @return The parts, or null.
     * @ghidraAddress NTSC-U/C: 0x001c46d8
     * @ghidraAddress PAL: 0x001cd478
     */
    OvyAllPlayer *FindAll(int nPlayer);

    /**
     * Add a checkpoint to the song position bar.
     *
     * @param fPos The place on the bar, from 0 to 1.
     * @param pszLabel The label, or null.
     * @ghidraAddress NTSC-U/C: 0x001c4718
     * @ghidraAddress PAL: 0x001cd4b8
     */
    void AddCheckpoint(float fPos, const char *pszLabel);

    /**
     * Remove the checkpoints of the song position bar.
     *
     * @ghidraAddress NTSC-U/C: 0x001c4748
     * @ghidraAddress PAL: 0x001cd4e8
     */
    void ClearCheckpoints();

    /**
     * Set the level of the energy bar.
     *
     * @param fLevel The level, from 0 to 1.
     * @param nColor The colour of the bar.
     * @ghidraAddress NTSC-U/C: 0x001c4778
     * @ghidraAddress PAL: 0x001cd518
     */
    void SetEnergy(float fLevel, int nColor);

    /**
     * Start or stop the warning of the energy bar.
     *
     * @param bWarn Whether the warning shows.
     * @ghidraAddress NTSC-U/C: 0x001c4798
     * @ghidraAddress PAL: 0x001cd538
     */
    void SetEnergyWarning(bool bWarn);

    /**
     * Show the score of a player, or the letters of a duel player.
     *
     * @param nPlayer The player.
     * @param nScore The score, or the number of letters.
     * @ghidraAddress NTSC-U/C: 0x001c47b8
     * @ghidraAddress PAL: 0x001cd558
     */
    void SetScore(int nPlayer, int nScore);

    /**
     * Show the multiplier of a player's pending points.
     *
     * @param nPlayer The player.
     * @param nMultiplier The multiplier.
     * @param bHot Whether the points take the hot colour.
     * @param pszText The text of the multiplier.
     * @ghidraAddress NTSC-U/C: 0x001c4810
     * @ghidraAddress PAL: 0x001cd5b0
     */
    void SetMultiplier(int nPlayer, int nMultiplier, int bHot, const char *pszText);

    /**
     * Show a player's pending points, or reveal letters in a duel.
     *
     * @param nPlayer The player.
     * @param nPoints The points.
     * @param pszText The text of the points.
     * @ghidraAddress NTSC-U/C: 0x001c4870
     * @ghidraAddress PAL: 0x001cd610
     */
    void ShowPoints(int nPlayer, int nPoints, const char *pszText);

    /**
     * Hide the multiplier of a player's pending points.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001c48e0
     * @ghidraAddress PAL: 0x001cd680
     */
    void HideMultiplier(int nPlayer);

    /**
     * End a player's pending points, or fly the letters of a duel.
     *
     * @param nPlayer The player.
     * @param nResult One of GfxManager::PendingPointsResult.
     * @ghidraAddress NTSC-U/C: 0x001c4918
     * @ghidraAddress PAL: 0x001cd6b8
     */
    void EndPoints(int nPlayer, int nResult);

    /**
     * Show a player catching a phrase. The body is empty.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001c4980
     * @ghidraAddress PAL: 0x001cd720
     */
    void ShowCapture([[maybe_unused]] int nPlayer) {
    }

    /**
     * Mark a player as playing or not.
     *
     * @param nPlayer The player.
     * @param bPlaying Whether the player plays.
     * @ghidraAddress NTSC-U/C: 0x001c4988
     * @ghidraAddress PAL: 0x001cd728
     */
    void SetPlaying(int nPlayer, bool bPlaying);

    /**
     * Show the power-up a player holds.
     *
     * @param nPlayer The player.
     * @param nPowerup The power-up, one of GameLogic::Powerup.
     * @ghidraAddress NTSC-U/C: 0x001c49a8
     * @ghidraAddress PAL: 0x001cd748
     */
    void ShowPowerup(int nPlayer, int nPowerup);

    /**
     * Let a player's power-up icon slide into view or not.
     *
     * @param nPlayer The player.
     * @param nEnabled Non-zero to let the icon show.
     * @ghidraAddress NTSC-U/C: 0x001c49f8
     * @ghidraAddress PAL: 0x001cd798
     */
    void SetPowerupEnabled(int nPlayer, int nEnabled);

    /**
     * Fly a message of two lines across the display.
     *
     * @param pszText The large line.
     * @param pszSmallText The small line.
     * @param fDuration The time the message rests in view, in milliseconds.
     * @param fScale The scale of the lines.
     * @param nPlayer The player whose colour the lines take, or a negative value for the default.
     * @param fX The horizontal place of the message.
     * @param fZ The vertical place of the message.
     * @ghidraAddress NTSC-U/C: 0x001c4a48
     * @ghidraAddress PAL: 0x001cd7e8
     */
    void ShowTextMessage(const char *pszText,
                         const char *pszSmallText,
                         float fDuration,
                         float fScale,
                         int nPlayer,
                         float fX,
                         float fZ);

    /**
     * Show a line of text, or hide it for a null or empty text.
     *
     * @param pszText The line, or null.
     * @param bFirstLine Whether the line is `HUD genmsg.txt` rather than `HUD genmsg2.txt`.
     * @ghidraAddress NTSC-U/C: 0x001c4a68
     * @ghidraAddress PAL: 0x001cd808
     */
    void SetMessage(const char *pszText, bool bFirstLine);

    /**
     * Close or open the letterbox bars.
     *
     * @param bClosed Whether to close the bars.
     * @ghidraAddress NTSC-U/C: 0x001c4aa0
     * @ghidraAddress PAL: 0x001cd840
     */
    void SetLetterbox(bool bClosed);

    /**
     * Report how far the letterbox bars are closed.
     *
     * @return The part, from 0 (open) to 1 (closed).
     * @ghidraAddress NTSC-U/C: 0x001c4ad8
     * @ghidraAddress PAL: 0x001cd878
     */
    float GetLetterbox();

    /**
     * Show or hide the parts of the display the game shows during play.
     *
     * A remix that plays back without editing shows the song position and the avatars only. The
     * tutorial does not show the song position, the avatars, or the scores, and practice does not
     * show the energy bar.
     *
     * @param bShow Whether to show the parts.
     * @param nUnused Not read.
     * @ghidraAddress NTSC-U/C: 0x001c4af8
     * @ghidraAddress PAL: 0x001cd898
     */
    void SetPartsShown(bool bShow, int nUnused);

    /**
     * Slide the energy bar in or out.
     *
     * @param bShow Whether to show the bar.
     * @ghidraAddress NTSC-U/C: 0x001c4ef0
     * @ghidraAddress PAL: 0x001cdc90
     */
    void ShowEnergy(bool bShow);

    /**
     * Show or hide a player's score and pending points.
     *
     * @param nPlayer The player.
     * @param bShow Whether to show them.
     * @ghidraAddress NTSC-U/C: 0x001c4f10
     * @ghidraAddress PAL: 0x001cdcb0
     */
    void ShowScore(int nPlayer, bool bShow);

    /**
     * Fade the tutorial highlight in or out.
     *
     * @param bShow Whether to show the highlight.
     * @ghidraAddress NTSC-U/C: 0x001c4fc8
     * @ghidraAddress PAL: 0x001cdd68
     */
    void ShowBox(bool bShow);

    /**
     * Fade the tutorial arrow in or out.
     *
     * @param bShow Whether to show the arrow.
     * @ghidraAddress NTSC-U/C: 0x001c4fe8
     * @ghidraAddress PAL: 0x001cdd88
     */
    void ShowArrow(bool bShow);

    /**
     * Show or hide a player's avatar, or the leader avatar for player 0 when the player has none.
     *
     * @param nPlayer The player.
     * @param bShow Whether to show the avatar.
     * @ghidraAddress NTSC-U/C: 0x001c5008
     * @ghidraAddress PAL: 0x001cdda8
     */
    void ShowAvatar(int nPlayer, bool bShow);

    /**
     * Slide the song position bar in or out.
     *
     * @param bShow Whether to show the bar.
     * @ghidraAddress NTSC-U/C: 0x001c50a0
     * @ghidraAddress PAL: 0x001cde40
     */
    void ShowSongPos(bool bShow);

    /**
     * Show or hide the track label.
     *
     * @param bShow Whether to show the label.
     * @ghidraAddress NTSC-U/C: 0x001c50d8
     * @ghidraAddress PAL: 0x001cde78
     */
    void ShowTrackLabel(bool bShow);

    /**
     * Choose the size of a player's avatar display.
     *
     * @param nPlayer The player.
     * @param nFreqSize One of GameOptions::FreqSize.
     * @ghidraAddress NTSC-U/C: 0x001c5110
     * @ghidraAddress PAL: 0x001cdeb0
     */
    void SetFreqSize(int nPlayer, int nFreqSize);

    /**
     * Label the track a player moved to.
     *
     * @param nPlayer The player.
     * @param nInstrument The instrument of the track, one of GfxManager::Instrument.
     * @param nTrackKind The kind of the track.
     * @ghidraAddress NTSC-U/C: 0x001c5150
     * @ghidraAddress PAL: 0x001cdef0
     */
    void SetTrack(int nPlayer, int nInstrument, int nTrackKind);

    /**
     * Flash a player's pending points, or time the letter banner of a duel.
     *
     * @param nPlayer The player.
     * @param fValue The minimum swell of the flash, or the time of the banner. Nothing happens
     *               unless it is positive.
     * @ghidraAddress NTSC-U/C: 0x001c51d8
     * @ghidraAddress PAL: 0x001cdf78
     */
    void FlashPoints(int nPlayer, float fValue);

    /**
     * Move the tutorial highlight to a rectangle of the screen.
     *
     * @param fX0 The left edge, in the unit square of the screen.
     * @param fY0 The top edge.
     * @param fX1 The right edge.
     * @param fY1 The bottom edge.
     * @param fDuration The ticks the move takes.
     * @ghidraAddress NTSC-U/C: 0x001c5250
     * @ghidraAddress PAL: 0x001cdff0
     */
    void SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration);

    /**
     * Move the tutorial arrow to point at a place of the screen.
     *
     * @param fX The horizontal place, in the unit square of the screen.
     * @param fY The vertical place.
     * @param fAngle The direction of the arrow, in degrees.
     * @param fDuration The ticks the move takes.
     * @ghidraAddress NTSC-U/C: 0x001c5270
     * @ghidraAddress PAL: 0x001ce010
     */
    void SetArrowTarget(float fX, float fY, float fAngle, float fDuration);

    /**
     * Fly the button icon of a lane.
     *
     * @param nLane The lane.
     * @param pFrom The place the flight starts at.
     * @param pTo The place the flight ends at.
     * @ghidraAddress NTSC-U/C: 0x001c5290
     * @ghidraAddress PAL: 0x001ce030
     */
    void FlyButton(int nLane, const Vector2 *pFrom, const Vector2 *pTo);

    /**
     * Start or stop the pulse of the button icon of a lane.
     *
     * @param nLane The lane.
     * @param bPulse Whether the icon pulses.
     * @ghidraAddress NTSC-U/C: 0x001c52c0
     * @ghidraAddress PAL: 0x001ce060
     */
    void SetButtonPulse(int nLane, bool bPulse);

    /**
     * Hide the button icon of a lane.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x001c52f0
     * @ghidraAddress PAL: 0x001ce090
     */
    void HideButton(int nLane);

    /**
     * Change the glyph of the button icon of a lane.
     *
     * @param nLane The lane.
     * @param pszGlyph The glyph.
     * @ghidraAddress NTSC-U/C: 0x001c5318
     * @ghidraAddress PAL: 0x001ce0b8
     */
    void SetButtonGlyph(int nLane, const char *pszGlyph);

    /**
     * Slide the picture of the controller in or out and place it.
     *
     * @param bShow Whether to show the picture.
     * @param fPosition The frame of `HUD controller map pos.tnm`, or a negative value to retain it.
     * @ghidraAddress NTSC-U/C: 0x001c5358
     * @ghidraAddress PAL: 0x001ce0f8
     */
    void ShowController(bool bShow, float fPosition);

    /**
     * Light or darken the picture of the controller.
     *
     * @param bHighlight Whether to light the picture.
     * @ghidraAddress NTSC-U/C: 0x001c53c8
     * @ghidraAddress PAL: 0x001ce168
     */
    void SetControllerHighlight(bool bHighlight);

    /**
     * Show or hide a button of the picture of the controller.
     *
     * @param nButton The button, one of JoypadButton.
     * @param bShow Whether to show the button.
     * @ghidraAddress NTSC-U/C: 0x001c53f8
     * @ghidraAddress PAL: 0x001ce198
     */
    void ShowControllerButton(int nButton, bool bShow);

    /**
     * Fill the panel of text and slide it in.
     *
     * @param pszText The text.
     * @param fSize The frame of the size of the panel.
     * @param fPosition The frame of the place of the panel, or a negative value to retain it.
     * @ghidraAddress NTSC-U/C: 0x001c5428
     * @ghidraAddress PAL: 0x001ce1c8
     */
    void OpenDialog(const char *pszText, float fSize, float fPosition);

    /**
     * Slide the panel of text out.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5458
     * @ghidraAddress PAL: 0x001ce1f8
     */
    void CloseDialog();

    /**
     * Show a player on a track. The body is empty.
     *
     * @param nPlayer The player.
     * @param nIndex The place of the player among those on the track.
     * @ghidraAddress NTSC-U/C: 0x001c5488
     * @ghidraAddress PAL: 0x001ce228
     */
    void ShowPlayerOnTrack([[maybe_unused]] int nPlayer, [[maybe_unused]] int nIndex) {
    }

    /**
     * Show one blank entry per section of a remix.
     *
     * @param nCount The number of sections.
     * @ghidraAddress NTSC-U/C: 0x001c5490
     * @ghidraAddress PAL: 0x001ce230
     */
    void SetSectionCount(int nCount);

    /**
     * Set the label of a section of a remix.
     *
     * @param nIndex The section.
     * @param pszLabel The label.
     * @ghidraAddress NTSC-U/C: 0x001c54c0
     * @ghidraAddress PAL: 0x001ce260
     */
    void SetSectionLabel(int nIndex, const char *pszLabel);

    /**
     * Slide the sections of a remix in or out.
     *
     * @param bShow Whether to show the sections.
     * @ghidraAddress NTSC-U/C: 0x001c54f0
     * @ghidraAddress PAL: 0x001ce290
     */
    void ShowSections(bool bShow);

    /**
     * Light the entry of a section of a remix.
     *
     * @param nIndex The section.
     * @ghidraAddress NTSC-U/C: 0x001c5528
     * @ghidraAddress PAL: 0x001ce2c8
     */
    void HighlightSection(int nIndex);

    /**
     * Set the text of an entry of a player's remix panel.
     *
     * @param nPlayer The player.
     * @param nIndex The entry.
     * @param pszText The text.
     * @param nPanel The panel, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001c5558
     * @ghidraAddress PAL: 0x001ce2f8
     */
    void SetRemixPanelText(int nPlayer, int nIndex, const char *pszText, int nPanel);

    /**
     * Move the cursor of a player's remix panel to an entry.
     *
     * @param nPlayer The player.
     * @param nIndex The entry, or a negative value for none.
     * @param nPanel The panel, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001c55c0
     * @ghidraAddress PAL: 0x001ce360
     */
    void SelectRemixPanelEntry(int nPlayer, int nIndex, int nPanel);

    /**
     * Blink the cursor of a player's remix panel.
     *
     * @param nPlayer The player.
     * @param nPanel The panel, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001c5620
     * @ghidraAddress PAL: 0x001ce3c0
     */
    void FlashRemixPanel(int nPlayer, int nPanel);

    /**
     * Light or darken an entry of a player's remix panel.
     *
     * @param nPlayer The player.
     * @param nIndex The entry.
     * @param bLit Whether to light the entry.
     * @param nPanel The panel, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001c5668
     * @ghidraAddress PAL: 0x001ce408
     */
    void SetRemixPanelLit(int nPlayer, int nIndex, int bLit, int nPanel);

    /**
     * Slide out a panel of a player's remix menu.
     *
     * @param nPlayer The player.
     * @param nPanel The panel, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001c56d0
     * @ghidraAddress PAL: 0x001ce470
     */
    void ShowRemixPanel(int nPlayer, int nPanel);

    /**
     * Slide a player's remix menu in or out.
     *
     * @param nPlayer The player.
     * @param bShow Whether to show the menu.
     * @ghidraAddress NTSC-U/C: 0x001c5710
     * @ghidraAddress PAL: 0x001ce4b0
     */
    void ShowRemix(int nPlayer, bool bShow);

    /**
     * Show the stick marker of a remix.
     *
     * @param nDirection The material, one of HudStick::Direction.
     * @param pPosition The place on the diagram.
     * @ghidraAddress NTSC-U/C: 0x001c5760
     * @ghidraAddress PAL: 0x001ce500
     */
    void ShowStick(int nDirection, const Vector2 *pPosition);

    /**
     * Hide the stick marker of a remix.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5790
     * @ghidraAddress PAL: 0x001ce530
     */
    void HideStick();

    /**
     * Move the display camera by an offset from its place in the layout.
     *
     * @param pOffset The offset.
     * @ghidraAddress NTSC-U/C: 0x001c57c0
     * @ghidraAddress PAL: 0x001ce560
     */
    void SetCameraOffset(const Vector3 *pOffset);

    /**
     * Return every part to its state at the start of a song, hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5860
     * @ghidraAddress PAL: 0x001ce600
     */
    void Reset();

    /**
     * Let the parts show again once the song starts.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5918
     * @ghidraAddress PAL: 0x001ce6b8
     */
    void ClearStarted();

    /**
     * Show every score of a shared-screen game at its place in the ranking, with the bar of its
     * share of the points.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5920
     * @ghidraAddress PAL: 0x001ce6c0
     */
    void ShowCheckpoint();

    /**
     * Show the result of a song: hide the energy bar of a lost song, or clear the pending points
     * of a won one.
     *
     * @param bWon Whether the song was won.
     * @param nMode Not read.
     * @ghidraAddress NTSC-U/C: 0x001c5c80
     * @ghidraAddress PAL: 0x001cea20
     */
    void ShowResult(bool bWon, int nMode);

    /**
     * Advance the display, and show its parts once the song is about to start.
     *
     * @param fFrame The frame of `hud.view`, the song position in ticks.
     * @param fSongDelta The song time since the last poll.
     * @param fUnused Passed to OvyLocalPlayer::Poll(), which does not read it.
     * @param fRealDelta The time since the last poll.
     * @param fFuture The length of the future part of the song position bar.
     * @ghidraAddress NTSC-U/C: 0x001c5d18
     * @ghidraAddress PAL: 0x001ceab8
     */
    void Poll(float fFrame, float fSongDelta, float fUnused, float fRealDelta, float fFuture);

    /**
     * Draw the display: the layout, the common parts, the parts of every player, and the avatars,
     * the winner's last.
     *
     * @ghidraAddress NTSC-U/C: 0x001c5f00
     * @ghidraAddress PAL: 0x001ceca0
     */
    void Draw();

    /**
     * Read the settings of the display from a configuration section and its defaults.
     *
     * @param pConfig The configuration section, or null.
     * @param pDefaults The section of defaults, or null.
     * @param bUnused Not read.
     * @ghidraAddress NTSC-U/C: 0x001c3690
     * @ghidraAddress PAL: 0x001cc430
     */
    static void LoadConfig(DataArray *pConfig, DataArray *pDefaults, bool bUnused);

    /**
     * Choose the head-up display layout from the community and the rule set of the game.
     *
     * A solo game and an online remix use `HUD1`, any other duel `HUDd`, and any other game
     * `HUDm`.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6540
     * @ghidraAddress PAL: 0x001bf2e0
     */
    static void SetHudPrefix();

    /**
     * The letter of the head-up display layout, `1`, `d`, or `m`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b1e0
     */
    static char sHudLetter;

    /**
     * The prefix of the head-up display layout's scene objects, `HUD1`, `HUDd`, or `HUDm`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b1e4
     */
    static const char *sHudPrefix;

    /**
     * The time the head-up display takes to assemble, `assembly_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af918
     */
    static float sAssemblyTime;

    /**
     * The depth range the display camera draws into, `hud_z`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b158
     */
    static Vector2 sHudZ;

    /**
     * The scale of the points that fly off a score, `hud_points_scale`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af93c
     */
    static float sPointsScale;

    /**
     * The ticks the points of a duel take to move, `duel_points_move_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af940
     */
    static float sDuelPointsMoveTicks;

    /**
     * One over sDuelPointsMoveTicks.
     *
     * @ghidraAddress NTSC-U/C: 0x003af944
     */
    static float sDuelPointsMoveRate;

    HudCommon *mCommon;                    /*!< The parts of the whole game. */
    Rnd::View *mHudView;                   /*!< `hud.view`. */
    Rnd::Cam *mCam;                        /*!< `hud.cam`. */
    Vector3 mCamPos;                       /*!< The place of mCam in the layout. */
    float mLastTime;                       /*!< The system time of the last poll. */
    int mStarted;                          /*!< Whether the parts show since the song started. */
    std::vector<OvyLocalPlayer *> mLocals; /*!< The parts of the players on this console. */
    std::vector<OvyAllPlayer *> mAlls;     /*!< The parts of every player. */
};
