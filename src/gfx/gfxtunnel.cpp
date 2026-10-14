#include "gfx/gfxtunnel.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <utility>

#include "game/gamedb.h"
#include "game/song.h"
#include "game/triggermgr.h"
#include "gfx/gemerasefx.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxtunnelpanel.h"
#include "gfx/gfxutil.h"
#include "gfx/particlearms.h"
#include "gfx/playercamfx.h"
#include "gfx/ship.h"
#include "gfx/spotlightfx.h"
#include "gfx/tnlblurs.h"
#include "gfx/tnlcatcher.h"
#include "gfx/tnlcripfx.h"
#include "gfx/tnldivider.h"
#include "gfx/tnlfire.h"
#include "gfx/tnlflashfx.h"
#include "gfx/tnlgemcapfx.h"
#include "gfx/tnlgems.h"
#include "gfx/tnlgrid.h"
#include "gfx/tnlplayer.h"
#include "gfx/tnlslowdownfx.h"
#include "gfx/tnlstreakgemfx.h"
#include "gfx/tnltrackfx.h"
#include "gfx/tnlwinshell.h"
#include "math/rand.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/debug.h"
#include "os/string.h"
#include "os/system.h"
#include "os/timer.h"
#include "rnd/rndcam.h"
#include "rnd/rndenviron.h"
#include "rnd/rndlight.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndmatanim.h"
#include "rnd/rndmesh.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndrenderer.h"
#include "rnd/rndtransanim.h"
#include "script/dataarray.h"
#include "script/scriptfunction.h"

namespace {

// The tick of an event that never comes.
constexpr float kNeverTick = 1e9f;

// The ticks of one beat.
constexpr float kTicksPerBeat = 480.0f;

// The length of a move of a player to the next track, in milliseconds.
constexpr float kMoveLengthMs = 400.0f;

// The players a console shows, each with a local view `tnl local<n>.view`.
constexpr int kMaxLocalViews = 4;

// The flashes of the gems.
constexpr int kNumFlashes = 30;

// The capture bursts, eight for each player and twelve more.
constexpr int kGemCapsPerPlayer = 8;
constexpr int kExtraGemCaps = 12;

// The entry of `max_gems` the tunnel reads, and the entry it would read for a game of several
// players on this console.
constexpr int kMaxGemsIndex = 1;
constexpr int kMaxGemsLocalIndex = 2;

// The fires of the captured tracks, and the fires of the freestyle of a solo game.
constexpr int kNumTrackFires = 2;
constexpr int kNumFreestyleFires = 6;
constexpr int kFreestyleFirePlayer = -2;

// The keys of the turn of a gem, which loops from no turn to a full turn.
constexpr int kRotationKeys = 9;
constexpr int kRotationLoops = 1;
constexpr float kFullTurn = 6.28318548f;

// The unlock index of the arena of the boss.
constexpr int kBossArena = 4;

// The first bar the geometry builds, before the song starts.
constexpr int kLeadInBar = -4;

// The colours and the steps of the targets of the activators, `target <n>_<c>.mesh`.
constexpr int kNumTargetColors = 4;
constexpr int kNumTargetSteps = 3;

// A bar that ended this many ticks before the song is dropped.
constexpr float kBarKeepTicks = 960.0f;

// A gem this many ticks behind the song is not placed.
constexpr float kLateGemTicks = 960.0f;

// The cursors are told of the gems of a span that ends more than this many ticks ahead of the song.
constexpr float kCursorLeadTicks = 80.0f;

// The milliseconds an erased gem waits for each tick it lies ahead of the song.
constexpr float kEraseMsPerTick = 0.1734375f;

// The colour TnlGems::AddGem() receives for a gem in its own colour.
constexpr char kOwnColor = -1;

// The tick a missed gem shows from.
constexpr float kMissShowTick = -1000.0f;

// The frames of `boundary_condition.view` for each unit of energy.
constexpr float kConditionFrames = 1000.0f;

// The ticks of two bars.
constexpr float kTwoBarTicks = 3840.0f;

// The length of a buffer for the name of a fire or of a gem group.
constexpr int kNameLength = 32;

constexpr const char *kTriggersFile = "Tunnel/triggers.txt";

// The camera blend algorithm and length of BlendToArenaPath().
constexpr char kArenaBlendAlgorithm[] = "multi";
constexpr float kArenaBlendMs = 1000.0f;
constexpr float kArenaBlendOn = 1.0f;
constexpr float kArenaBlendOff = 0.0f;

constexpr float kPi = 3.1415927f;
constexpr float kHalfTurnDegrees = 180.0f;

// The values of GfxManager::mTunnelShape, the `circular` cheat, that force a circular tunnel in a
// solo game.
constexpr signed char kTunnelShapeCircular = 1;
constexpr signed char kTunnelShapeFlipped = 2;

// The cameras BuildOuterCams() fits for each player: `tnl cam<n>`, `tnl cam<n>z`,
// `tnl win cam<n>`, `outer <n>.cam`, and `outer near <n>.cam`.
constexpr int kCamsPerPlayer = 5;

// The split screen places the players in two columns, each cell half of the screen.
constexpr int kScreenColumns = 2;
constexpr float kScreenCell = 0.5f;

// The bound a search for the extent of the cross section starts from.
constexpr float kNoBound = 1e9f;

// The boundaries between the sections fit the cross section with this margin.
constexpr float kDividerMargin = 1.2f;
constexpr float kDividerShrink = 1.1f;

// The meshes `HUD <kind>_gem<n>.mesh` of each kind of powerup gem.
constexpr int kNumHudGemMeshes = 4;

// The kind of panel of `panel mat highlight.mat`, below every kind, which sets the default.
constexpr int kHighlightPanelKind = -999;

// The kinds of panel `panel intro <n>.mat` before the song.
constexpr int kNumIntroPanels = 4;

// The detail of the panels the geometry receives, eight cross sections a bar.
constexpr int kPanelDetail = 4;

// A capture chunk is this many widths of a panel across.
constexpr float kChunkWidths = 1.18778443f;

// The ticks of one bar.
constexpr int kTicksPerBar = 1920;
constexpr float kBarTicks = 1920.0f;

// The tick of the first bar the geometry builds.
constexpr float kLeadInTick = static_cast<float>(kLeadInBar) * kBarTicks;

// The arguments of `blend_to_path` with a loop, and of `arena_done_in`.
constexpr int kBlendLoopArgs = 5;
constexpr int kArenaDoneInArgs = 3;

// The arguments of `set_particle_player_color`: the colour of the first slot, then the alphas the
// particles spawn with and fade to.
constexpr int kFirstSlotColorArg = 2;
constexpr int kStartAlphaArg = 6;
constexpr int kEndAlphaArg = 7;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

// The horizontal normal of a segment of the cross section, its direction crossed with the up
// axis.
inline Vector3 SegmentNormal(const GfxTunnel::Segment &segment) {
    const Vector2 delta{segment.second.x - segment.first.x, segment.second.y - segment.first.y};
    const Vector3 direction{delta.x, 0.0f, delta.y};
    const Vector3 up{0.0f, 1.0f, 0.0f};
    Vector3 normal;
    CrossVec3(&direction.x, &up.x, &normal.x);
    return normal;
}

inline float DegreesToRadians(float flDegrees) {
    return (flDegrees * kPi) / kHalfTurnDegrees;
}

inline void ScaleVector(Vector3 *pVector, float flScale) {
    pVector->x *= flScale;
    pVector->y *= flScale;
    pVector->z *= flScale;
}

} // namespace

// The static initialiser of the unit, and the routines that run it to construct and to destroy the
// statics of the tunnel. The same initialiser constructs the statics of TnlFire and TnlGrid.
// NTSC-U/C: 0x001e14f8, PAL: 0x001ea298
// NTSC-U/C: 0x001e1948, PAL: 0x001ea6e8
// NTSC-U/C: 0x001e1968, PAL: 0x001ea708

GfxTunnel *GfxTunnel::sCurrent = nullptr;
int GfxTunnel::sSplitScreen = 0;
int GfxTunnel::sShowPanels = 1;
int GfxTunnel::sShowDividers = 1;
int GfxTunnel::sInvertTunnel = 0;
int GfxTunnel::sFlipTunnel = 0;
int GfxTunnel::sCenterTunnel = 0;
int GfxTunnel::sOpaquePanels = 0;
int GfxTunnel::sShowDisabledPanels = 1;
int GfxTunnel::sShowSeeker = 1;
int GfxTunnel::sSeekerEndcapsAlways = 0;
float GfxTunnel::sSeekerOpacity = 1.0f;
int GfxTunnel::sShowActivator = 1;
int GfxTunnel::sShowMissGems = 1;
int GfxTunnel::sTunnelBars = 5;
int GfxTunnel::sTunnelLowResBars = 6;
int GfxTunnel::sUseSlab = 0;
float GfxTunnel::sSlabDepth = 0.25f;
float GfxTunnel::sSlabAngle = 0.0f;
float GfxTunnel::sCameraFov = 70.0f;
float GfxTunnel::sLetterboxFov = 90.0f;
float GfxTunnel::sGemRotatePeriod = 2000.0f;
float GfxTunnel::sGemHitFlashSize = 1.5f;
float GfxTunnel::sGemHitFlashGrowth = 0.005f;
float GfxTunnel::sGemPitchFlashSize = 1.5f;
float GfxTunnel::sGemPitchFlashGrowth = 0.005f;
float GfxTunnel::sWinSequenceDuration = 3840.0f;
float GfxTunnel::sFadeToBlackDuration = 1000.0f;
float GfxTunnel::sPreviewCamChangeTime = 1000.0f;
float GfxTunnel::sDuelFailDarkening = 0.5f;
float GfxTunnel::sMeshGemScale = 1.0f;
float GfxTunnel::sPanelWidth = 0.841903567f;
Vector3 GfxTunnel::sReserved310{7.0f, 7.0f, 7.0f, 7.0f};
// Yes, the fourth word of these vectors is zero because the binary never writes it.
Vector3 GfxTunnel::sBoundaryOffset{0.0f, 0.0f, -1.0f, 0.0f};
Color GfxTunnel::sSeekerColor{0.0f, 0.0f, 0.0f, 0.0f};
Vector2 GfxTunnel::sArenaZ{0.5f, 1.0f};
Vector2 GfxTunnel::sTnlZ{0.05f, 0.497f};
Vector2 GfxTunnel::sTnlzZ{0.1f, 0.5f};
LinearInterpolator GfxTunnel::sTrackLightFade(0.0f, 1.0f, -kTicksPerBeat, 0.0f);
std::vector<GfxTunnel::Segment> GfxTunnel::sShape;
std::vector<GfxTunnel::Segment> GfxTunnel::sSegments;
std::vector<Vector2> GfxTunnel::sTrackPoints;
Vector3 GfxTunnel::sBurstScale{1.0f, 1.0f, 1.0f, 0.0f};
Vector3 GfxTunnel::sGridQuarterScale{1.0f, 1.0f, 1.0f, 0.0f};
Vector3 GfxTunnel::sGridEighthScale{1.0f, 1.0f, 1.0f, 0.0f};

void GfxTunnel::LayoutDividers(const std::vector<Segment> *pSegments, float flScale) {
    Vector2 low{kNoBound, kNoBound};
    Vector2 high{-kNoBound, -kNoBound};
    for (auto it = pSegments->begin(); it != pSegments->end(); ++it) {
        const Vector2 *points[] = {&it->first, &it->second};
        for (const Vector2 *pPoint : points) {
            if (pPoint->x < low.x) {
                low.x = pPoint->x;
            }
            if (pPoint->y < low.y) {
                low.y = pPoint->y;
            }
            if (high.x < pPoint->x) {
                high.x = pPoint->x;
            }
            if (high.y < pPoint->y) {
                high.y = pPoint->y;
            }
        }
    }
    // Yes, the binary measures the low corner and never uses it.
    (void)low;
    const float flSize =
        (sqrtf((high.x * high.x) + (high.y * high.y)) * kDividerMargin) / kDividerShrink;
    const Vector3 scale{flSize, flSize, flSize};
    const Vector3 offset{
        sBoundaryOffset.x * flScale, sBoundaryOffset.y * flScale, sBoundaryOffset.z * flScale};
    TnlDivider::SetLayout(&scale, &offset);
}

void GfxTunnel::ScaleHudGemMeshes(const Vector3 *pScale, const char *pszKind) {
    for (int i = 0; i < kNumHudGemMeshes; ++i) {
        // Yes, the binary does not test the mesh for null.
        RndMesh *pMesh = FindObject<RndMesh>(FormatString("HUD %s_gem%d.mesh", pszKind, i));
        Transform &xfm = pMesh->mLocalXfm;
        xfm.mBasisX.x *= pScale->x;
        xfm.mBasisX.y *= pScale->x;
        xfm.mBasisX.z *= pScale->x;
        xfm.mBasisY.x *= pScale->y;
        xfm.mBasisY.y *= pScale->y;
        xfm.mBasisY.z *= pScale->y;
        xfm.mBasisZ.x *= pScale->z;
        xfm.mBasisZ.y *= pScale->z;
        xfm.mBasisZ.z *= pScale->z;
        pMesh->mDirty = 1;
    }
}

void GfxTunnel::ScaleHudGems(float flScrollSpeed) {
    const float flScale = 1.0f / flScrollSpeed;
    const Vector3 scale{flScale, flScale, flScale};
    ScaleHudGemMeshes(&scale, "auto");
    ScaleHudGemMeshes(&scale, "bump");
    ScaleHudGemMeshes(&scale, "crip");
    ScaleHudGemMeshes(&scale, "free");
    ScaleHudGemMeshes(&scale, "mult");
    ScaleHudGemMeshes(&scale, "slow");
    // Yes, the binary builds an identity transform and never uses it.
    const Transform identity{Vector3{1.0f, 0.0f, 0.0f},
                             Vector3{0.0f, 1.0f, 0.0f},
                             Vector3{0.0f, 0.0f, 1.0f},
                             Vector3{0.0f, 0.0f, 0.0f}};
    (void)identity;
}

void GfxTunnel::LoadConfig(
    GfxTunnel *pTunnel, DataArray *pConfig, DataArray *pDefaults, int nNumTracks, int nOption) {
    DataArray *pShape;
    if (nNumTracks < 0) {
        if (pTunnel == nullptr) {
            return;
        }
        nNumTracks = pTunnel->mNumTracks;
    }

    TnlTrackFX::LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    SpotLightFX::LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    PlayerCamFX::LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    TnlCatcher::LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    TnlGems::LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    // Yes, the binary reads the members of pTunnel from here on without testing it for null.
    pTunnel->mSlowdown->LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    pTunnel->mWinShell->LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    pTunnel->mBlurs->LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    pTunnel->mGeom->LoadConfig(pConfig, pDefaults, nOption);
    if (nOption != 0) {
        for (auto it = pTunnel->mPlayers.begin(); it != pTunnel->mPlayers.end(); ++it) {
            (*it)->LoadConfig(
                pConfig, pDefaults, pTunnel, nOption, (it == pTunnel->mPlayers.begin()) ? 1 : 0);
        }
    }

    FindConfigVector2(pConfig, pDefaults, "arena_z", &sArenaZ, true);
    FindConfigVector2(pConfig, pDefaults, "tnl_z", &sTnlZ, true);
    FindConfigVector2(pConfig, pDefaults, "tnlz_z", &sTnlzZ, true);
    FindConfigFloat(pConfig, pDefaults, "camera_fov", &sCameraFov, true);
    FindConfigFloat(pConfig, pDefaults, "camera_letterbox_fov", &sLetterboxFov, true);
    sCameraFov = DegreesToRadians(sCameraFov);
    sLetterboxFov = DegreesToRadians(sLetterboxFov);
    FindConfigFloat(pConfig, pDefaults, "gem_rotate_period", &sGemRotatePeriod, true);
    FindConfigFloat(pConfig, pDefaults, "gem_hit_flash_size", &sGemHitFlashSize, true);
    sGemHitFlashSize *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "gem_hit_flash_time", &sGemHitFlashGrowth, true);
    sGemHitFlashGrowth = TheGfxManager.mScrollSpeed / sGemHitFlashGrowth;
    FindConfigFloat(pConfig, pDefaults, "gem_pitch_flash_size", &sGemPitchFlashSize, true);
    sGemPitchFlashSize *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "gem_pitch_flash_time", &sGemPitchFlashGrowth, true);
    sGemPitchFlashGrowth = TheGfxManager.mScrollSpeed / sGemPitchFlashGrowth;
    FindConfigFloat(pConfig, pDefaults, "mesh_gem_scale", &sMeshGemScale, true);
    FindConfigFloat(pConfig, pDefaults, "track_capture_speed", &TnlFire::sCaptureSpeed, true);
    FindConfigFloat(
        pConfig, pDefaults, "track_autocapture_speed", &TnlFire::sAutoCaptureSpeed, true);
    FindConfigBool(pConfig, pDefaults, "splitscreen", &sSplitScreen, true);
    FindConfigBool(pConfig, pDefaults, "show_panels", &sShowPanels, true);
    FindConfigBool(pConfig, pDefaults, "opaque_panels", &sOpaquePanels, true);
    FindConfigBool(pConfig, pDefaults, "show_dividers", &sShowDividers, true);
    FindConfigBool(pConfig, pDefaults, "show_disabled_panels", &sShowDisabledPanels, true);
    FindConfigBool(pConfig, pDefaults, "show_seeker", &sShowSeeker, true);
    FindConfigBool(pConfig, pDefaults, "seeker_endcaps_always", &sSeekerEndcapsAlways, true);
    FindConfigColor(pConfig, pDefaults, "seeker_color", &sSeekerColor, true);
    FindConfigFloat(pConfig, pDefaults, "seeker_opacity", &sSeekerOpacity, true);
    FindConfigBool(pConfig, pDefaults, "show_activator", &sShowActivator, true);
    FindConfigBool(pConfig, pDefaults, "show_miss_gems", &sShowMissGems, true);
    FindConfigBool(pConfig, pDefaults, "use_slab", &sUseSlab, true);
    FindConfigFloat(pConfig, pDefaults, "slab_depth", &sSlabDepth, true);
    sSlabDepth *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "slab_angle", &sSlabAngle, true);
    sSlabAngle = DegreesToRadians(sSlabAngle);
    FindConfigInt(pConfig, pDefaults, "capture_max_chunks", &TnlFire::sMaxChunks, true);
    FindConfigVector2(
        pConfig, pDefaults, "capture_chunk_interval", &TnlFire::sCaptureInterval, true);
    FindConfigVector2(
        pConfig, pDefaults, "autocapture_chunk_interval", &TnlFire::sAutoCaptureInterval, true);
    FindConfigVector3(pConfig, pDefaults, "capture_chunk_world_force", &TnlFire::sChunkForce, true);
    FindConfigVector3(pConfig,
                      pDefaults,
                      "capture_chunk_initial_velocity_min",
                      &TnlFire::sCaptureVelocity[TnlFire::kRangeMin],
                      true);
    FindConfigVector3(pConfig,
                      pDefaults,
                      "capture_chunk_initial_velocity_max",
                      &TnlFire::sCaptureVelocity[TnlFire::kRangeMax],
                      true);
    FindConfigVector3(pConfig,
                      pDefaults,
                      "autocapture_chunk_initial_velocity_min",
                      &TnlFire::sAutoCaptureVelocity[TnlFire::kRangeMin],
                      true);
    FindConfigVector3(pConfig,
                      pDefaults,
                      "autocapture_chunk_initial_velocity_max",
                      &TnlFire::sAutoCaptureVelocity[TnlFire::kRangeMax],
                      true);
    const float flScrollSpeed = TheGfxManager.mScrollSpeed;
    ScaleVector(&TnlFire::sChunkForce, flScrollSpeed);
    ScaleVector(&TnlFire::sCaptureVelocity[TnlFire::kRangeMin], flScrollSpeed);
    ScaleVector(&TnlFire::sCaptureVelocity[TnlFire::kRangeMax], flScrollSpeed);
    ScaleVector(&TnlFire::sAutoCaptureVelocity[TnlFire::kRangeMin], flScrollSpeed);
    ScaleVector(&TnlFire::sAutoCaptureVelocity[TnlFire::kRangeMax], flScrollSpeed);
    FindConfigVector2(pConfig, pDefaults, "capture_chunk_lifespan", &TnlFire::sChunkLifespan, true);
    FindConfigBool(pConfig, pDefaults, "capture_chunks_lit", &TnlFire::sChunksLit, true);
    FindConfigFloat(
        pConfig, pDefaults, "capture_chunk_y_displace", &TnlFire::sChunkYDisplace, true);
    TnlFire::sChunkYDisplace *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "win_sequence_duration", &sWinSequenceDuration, true);
    FindConfigFloat(pConfig, pDefaults, "fade_to_black_duration", &sFadeToBlackDuration, true);
    FindConfigFloat(pConfig, pDefaults, "preview_cam_change_time", &sPreviewCamChangeTime, true);
    FindConfigFloat(pConfig, pDefaults, "duel_fail_darkening", &sDuelFailDarkening, true);
    FindConfigVector3(pConfig, pDefaults, "grid_quarter_scale", &sGridQuarterScale, true);
    FindConfigVector3(pConfig, pDefaults, "grid_eighth_scale", &sGridEighthScale, true);
    FindConfigVector3(pConfig, pDefaults, "boundary_offset", &sBoundaryOffset, true);
    FindConfigInt(pConfig, pDefaults, "tunnel_bars", &sTunnelBars, true);
    FindConfigInt(pConfig, pDefaults, "tunnel_low_res_bars", &sTunnelLowResBars, true);
    sTrackLightFade.Reset(
        0.0f, TheGfxManager.mInvScrollSpeed, sTrackLightFade.mX0, sTrackLightFade.mX1);

    if (FindConfigArray(pConfig, pDefaults, "tunnel", &pShape, true)) {
        sInvertTunnel = 0;
        sFlipTunnel = 0;
        sCenterTunnel = 0;
        int i = 1;
        while (true) {
            const char *pszFlag = pShape->Sym(i);
            if (strcasecmp(pszFlag, "invert") == 0) {
                sInvertTunnel = 1;
            } else if (strcasecmp(pszFlag, "flip") == 0) {
                sFlipTunnel = 1;
            } else if (strcasecmp(pszFlag, "center") == 0) {
                sCenterTunnel = 1;
            } else {
                break;
            }
            ++i;
        }
        const char *pszStyle = pShape->Sym(i++);

        const signed char nCheatShape = TheGfxManager.mTunnelShape;
        const bool bFlipped = nCheatShape == kTunnelShapeFlipped;
        if (bFlipped) {
            sFlipTunnel = 1;
        }
        const bool bCheat = nCheatShape == kTunnelShapeCircular || bFlipped;
        if ((bCheat && TheGameDb->mCommunity == GameDb::kCommunitySolo) ||
            strcasecmp(pszStyle, "circular") == 0) {
            sShape.resize(0);
        } else if (strcasecmp(pszStyle, "continuous") == 0) {
            // The points that follow the style chain the segments end to end.
            sShape.resize(((pShape->Size() - i) / 2) - 1);
            for (unsigned j = 0; j < sShape.size(); ++j) {
                sShape[j].first = Vector2{pShape->Float(i), pShape->Float(i + 1)};
                sShape[j].second = Vector2{pShape->Float(i + 2), pShape->Float(i + 3)};
                i += 2;
            }
        } else if (strcasecmp(pszStyle, "angular") == 0) {
            sShape.resize(nNumTracks);
            std::vector<Vector2> points(nNumTracks + 1);
            BuildTrackArc(&points, pShape);
            for (unsigned j = 0; j < sShape.size(); ++j) {
                sShape[j].first = points[j];
                sShape[j].second = points[j + 1];
            }
        } else {
            DebugWarn("Unrecognized tunnel style: %s", pszStyle);
        }
    }

    if (nOption != 0 && pTunnel != nullptr) {
        pTunnel->SetShowPanels(sShowPanels);
        pTunnel->SetShowDividers(sShowDividers);
        pTunnel->SetShowActivator(sShowActivator);
        pTunnel->BuildPanels(0, pTunnel->mArena->mCamPath);
    }
    delete pTunnel->mArms;
    pTunnel->mArms = nullptr;
    DataArray *pArms;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        pArms = pDefaults->FindArray("duel_particle_arms", true);
    } else {
        pArms = pDefaults->FindArray(
            FormatString("%s_arena_particle_arms", pTunnel->mArena->mName.c_str()), false);
        if (pArms == nullptr) {
            pArms = pDefaults->FindArray("particle_arms", true);
        }
    }
    pTunnel->mArms = new ParticleArms(pArms, pTunnel->mGeom);
    if (nOption != 0) {
        TheTriggerMgr.Load(kTriggersFile, nullptr);
    }
}

GfxTunnel::GfxTunnel(const std::vector<int> &instruments,
                     const std::vector<int> &trackTypes,
                     GfxArena *pArena,
                     int nOption,
                     float fStartTick,
                     float fSongTicks)
    : mTrackPlayersDirty(0), mPanelColorsDirty(0), mDefaultFocusTrack(-1), mDivider(nullptr),
      mArms(nullptr), mGems(nullptr), mGeom(new TnlGeom()), mArena(pArena),
      mTrackColorLight(nullptr), mTrackColorPlayer(-1), mFadeCurve(0.0f, 0.0f, 0.0f, 1.0f),
      mMissGemGroup(-1), mSlotGemGroup(-1), mGemGroup(-1), mTracks(instruments.size(), TrackData()),
      mNumTracks(static_cast<int>(instruments.size())), mLetterbox(0.0f), mCaptureReveals(1),
      mBossJourneyDone(0), mIdle(0), mUnlockDoneTicks(0.0f), mBossDoneTicks(0.0f),
      mResultDoneTick(kNeverTick), mBossJourneyDoneTick(kNeverTick), mCamFX(nullptr),
      mPathBlendCurve(0.0f, 0.0f, 0.0f, 1.0f, 3.0f), mPathBlending(0), mCenter{0.0f, 0.0f, 0.0f} {
    Timer timer;
    Timer loadTimer;
    timer.Start();
    loadTimer.Start();

    mTransparentView = FindObject<RndView>("tnl transparent");
    mGridView = FindObject<RndView>("grid1.view");
    if (mArena->mCamPath == nullptr) {
        mArena->SetCamPath(FindObject<RndTransAnim>("tunnel path"));
    }
    mSlowdown = new TnlSlowdownFX("slowdown_shell", this);
    mWinShell = new TnlWinShell();
    mBlurs = new TnlBlurs();
    DataArray *pConfig = GetModeGfxConfig();
    DataArray *pDefaults = GetGfxConfig();
    LoadConfig(this, pConfig, pDefaults, mNumTracks, 0);
    mSpotLights = new SpotLightFX();
    mGemErase = new GemEraseFX(this);

    for (int i = 0; i < mNumTracks; ++i) {
        mTracks[i].mIndex = i;
        mTracks[i].mInstrument = instruments[i];
        mTracks[i].mType = trackTypes[i];
        if ((TheGameDb->mCommunity == GameDb::kCommunitySolo ||
             TheGameDb->mRuleSet == GameDb::kRuleSetRemix) &&
            instruments[i] == GfxManager::kInstrumentDrum && mDefaultFocusTrack == -1) {
            mDefaultFocusTrack = static_cast<signed char>(i);
        }
    }

    // Yes, the binary multiplies and divides by the same number.
    const float flMsPerTick = (*TheGfxManager.mMsPerTick * kTicksPerBeat) / kTicksPerBeat;
    SetFilterScale(FindObject<RndAnimatable>("realtime.view"), flMsPerTick);
    mGeom->SetMoveLength(kMoveLengthMs / flMsPerTick);

    RndView *pTestDraw = FindObject<RndView>("tnl test draw");
    if (pTestDraw != nullptr) {
        pTestDraw->SetShowing(0);
    }
    RndView *pTunnelView = FindObject<RndView>("tnl.view");
    pTunnelView->RemoveAnim(FindObject<RndAnimatable>("test anim"));
    RndCam *pTestCam = FindObject<RndCam>("test cam");
    if (pTestCam != nullptr) {
        pTunnelView->RemoveTrans(pTestCam);
        pTunnelView->RemoveDraw(pTestCam);
    }
    RndView *pSlide = FindObject<RndView>("tnl cam slide1");
    pSlide->RemoveTrans(FindObject<RndTransformable>("flare gem test.view"));

    for (int i = BuildOuterCams(); i < kMaxLocalViews; ++i) {
        FindObject<RndView>(FormatString("tnl local%d.view", i + 1))->SetShowing(0);
    }

    DataArray *pMaxGems;
    // Yes, the binary discards this call's result.
    (void)FindConfigArray(pConfig, pDefaults, "max_gems", &pMaxGems, true);
    // Yes, the binary compares the solo test with the local community, so it always reads the
    // first count.
    const int nSolo = (TheGameDb->mCommunity == GameDb::kCommunitySolo) ? 1 : 0;
    const int nMaxGems =
        pMaxGems->Int((nSolo == GameDb::kCommunityLocal) ? kMaxGemsLocalIndex : kMaxGemsIndex);
    mArena->mView->SetShowing(1);

    RndParticleSys *pFlashSys = FindObject<RndParticleSys>("gem_flash.ps");
    mFlashes.resize(kNumFlashes);
    for (unsigned i = 0; i < mFlashes.size(); ++i) {
        mFlashes[i] = new TnlFlashFX(pFlashSys);
    }
    mTrackFX.resize(mNumTracks);
    for (unsigned i = 0; i < mTrackFX.size(); ++i) {
        mTrackFX[i] = new TnlTrackFX(static_cast<char>(i));
    }
    mCrippleFX.resize(TheGameDb->GetNumPlayers());
    for (unsigned i = 0; i < mCrippleFX.size(); ++i) {
        mCrippleFX[i] = new TnlCripFX(static_cast<int>(i));
    }
    mGemCaps.resize((TheGameDb->GetNumPlayers() * kGemCapsPerPlayer) + kExtraGemCaps);
    for (unsigned i = 0; i < mGemCaps.size(); ++i) {
        mGemCaps[i] = new TnlGemCapFX();
    }

    mGemBurstViews.resize(kNumGemBursts);
    LoadViews(&mGemBurstViews[kGemBurstGem], "gem explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstAuto], "auto_explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstMult], "mult_explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstSlow], "slow_explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstFree], "free_explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstBump], "bump_explode %d.view");
    LoadViews(&mGemBurstViews[kGemBurstCrip], "crip_explode %d.view");
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix ||
        TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mPlayerBurstViews.resize(TheGameDb->GetNumPlayers());
        for (unsigned i = 0; i < mPlayerBurstViews.size(); ++i) {
            String format(FormatString("gem explode %%d_%c.view",
                                       TheGameDb->GetPlayerColor(static_cast<int>(i))[0]));
            LoadViews(&mPlayerBurstViews[i], format.c_str());
        }
    }

    LoadTrackColors();
    mCheckpointMat = FindObject<RndMat>("panel ckpt.mat");
    mCheckpointFinishMat = FindObject<RndMat>("panel ckpt finish.mat");
    mFadeMesh = FindObject<RndMesh>("tunnel fade.mesh");
    mFadeMesh->SetShowing(1);
    mGeom->SetNumPlayers(TheGameDb->GetNumPlayers());
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        TnlGeom::Player *pPlayer = mGeom->GetPlayer(i);
        RndTransformable *pCamSlide =
            FindObject<RndTransformable>(FormatString("tnl cam slide%d", i + 1));
        RndView *pActivator = FindObject<RndView>(FormatString("activator%d.view", i));
        pPlayer->SetCamSlide(pCamSlide);
        pPlayer->SetActivator(pActivator);
    }
    BuildPanels(1, mArena->mCamPath);

    char szName[kNameLength];
    for (int i = 0; i < kNumTrackFires; ++i) {
        sprintf(szName, "fire%d", i);
        mFires.push_back(new TnlFire(szName, TnlFire::kChunkPlayer, this, true));
    }
    for (int i = 0; i < kNumFreestyleFires; ++i) {
        sprintf(szName, "fire fs%d", i);
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            mFires.push_back(new TnlFire(szName, kFreestyleFirePlayer, this, true));
        } else {
            TnlFire::Disable(szName, mTransparentView);
        }
    }

    FindObject<RndDrawable>("boundary msg")->SetShowing(1);
    FindObject<RndDrawable>("boundary_ring.mesh")->SetShowing(1);
    RndEnviron *pShown;
    RndEnviron *pHidden;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pHidden = FindObject<RndEnviron>("tunnel multi.env");
        pShown = FindObject<RndEnviron>("tunnel solo.env");
        mTrackColorLight = FindObject<RndLight>("tunnel track color.lit");
        for (mTrackColorPlayer = 0; mTrackColorPlayer < TheGameDb->GetNumPlayers() &&
                                    !TheGameDb->IsLocalPlayer(mTrackColorPlayer);
             ++mTrackColorPlayer) {
        }
        (void)TheGameDb->GetNumPlayers(); // Yes, the binary discards this call's result.
    } else {
        pShown = FindObject<RndEnviron>("tunnel multi.env");
        pHidden = FindObject<RndEnviron>("tunnel solo.env");
        mTrackColorLight = nullptr;
        mTrackColorPlayer = -1;
    }
    pHidden->SetShowing(0);
    pShown->SetShowing(1);
    ScaleLights(pShown);

    mGems = new TnlGems(this, nMaxGems, mNumTracks);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        mGems->mHasFocusTrack = 1;
    }
    if (TheGfxManager.mMonkeyGems != 0) {
        mGemGroup = mGems->AddMeshGroups("monkey", nullptr, 0.0f);
    } else {
        mGemGroup = mGems->AddCompositeMeshGroups("gem_b", nullptr, 0.0f);
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix ||
        TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mPlayerGemGroups.resize(TheGameDb->GetNumPlayers());
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            sprintf(szName, "gem_%c", TheGameDb->GetPlayerColor(i)[0]);
            mPlayerGemGroups[i] = mGems->AddCompositeMeshGroups(szName, nullptr, 0.0f);
        }
    }
    mGems->ClearInstances("gem_crate");
    mPowerupGemGroups.resize(kNumPowerupGems);
    mPowerupGemGroups[kPowerupGemAuto] = mGems->AddMeshGroups("gem_auto", nullptr, 0.0f);
    mPowerupGemGroups[kPowerupGemMult] = mGems->AddMeshGroups("gem_mult", nullptr, 0.0f);
    mPowerupGemGroups[kPowerupGemSlow] = mGems->AddMeshGroups("gem_slow", nullptr, 0.0f);
    mPowerupGemGroups[kPowerupGemFree] = mGems->AddMeshGroups("gem_free", nullptr, 0.0f);
    mPowerupGemGroups[kPowerupGemCrip] = mGems->AddMeshGroups("gem_crip", nullptr, 0.0f);
    mPowerupGemGroups[kPowerupGemBump] = mGems->AddMeshGroups("gem_bump", nullptr, 0.0f);
    mGems->ClearInstances("gem_neut");
    mMissGemGroup = mGems->AddMeshGroups("gem_miss", nullptr, 0.0f);
    bool bRotateSlots = false;
    RndDrawable *pSlots;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mSlotGemGroup = mGems->AddMeshGroups("gem_duel_slot", nullptr, 0.0f);
        pSlots = FindObject<RndDrawable>("gem_duel_slot0.mm");
    } else {
        mSlotGemGroup = mGems->AddMeshGroups("gem_slot", nullptr, 0.0f);
        pSlots = FindObject<RndDrawable>("gem_slot0.mm");
        bRotateSlots = true;
    }
    mInstrumentGemGroups.resize(GfxManager::kNumInstruments);
    mInstrumentGemGroups[GfxManager::kInstrumentDrum] = mGems->AddSpriteGroup("gem_drum");
    mInstrumentGemGroups[GfxManager::kInstrumentBass] = mGems->AddSpriteGroup("gem_bass");
    mInstrumentGemGroups[GfxManager::kInstrumentSynth] = mGems->AddSpriteGroup("gem_synth");
    mInstrumentGemGroups[GfxManager::kInstrumentGuitar] = mGems->AddSpriteGroup("gem_guitar");
    mInstrumentGemGroups[GfxManager::kInstrumentVocal] = mGems->AddSpriteGroup("gem_vox");
    mInstrumentGemGroups[GfxManager::kInstrumentFx] = mGems->AddSpriteGroup("gem_fx");
    mGems->ApplyRenderScales();

    RndAnimatable *pRealtime = FindObject<RndAnimatable>("realtime.view");
    mGems->AddRotations(mGemGroup,
                        kRotationKeys,
                        kRotationLoops,
                        pRealtime,
                        0.0f,
                        0.0f,
                        sGemRotatePeriod,
                        kFullTurn);
    for (unsigned i = 0; i < mPowerupGemGroups.size(); ++i) {
        mGems->AddRotations(mPowerupGemGroups[i],
                            kRotationKeys,
                            kRotationLoops,
                            pRealtime,
                            0.0f,
                            0.0f,
                            sGemRotatePeriod,
                            kFullTurn);
    }
    if (!mPlayerGemGroups.empty()) {
        mGems->AddRotations(mPlayerGemGroups.front(),
                            kRotationKeys,
                            kRotationLoops,
                            pRealtime,
                            0.0f,
                            0.0f,
                            sGemRotatePeriod,
                            kFullTurn);
    }
    if (bRotateSlots) {
        mGems->AddRotations(mSlotGemGroup,
                            kRotationKeys,
                            kRotationLoops,
                            pRealtime,
                            0.0f,
                            0.0f,
                            sGemRotatePeriod,
                            kFullTurn);
    }
    (void)FindObject<RndView>("tnl.view"); // Yes, the binary discards the view it finds.
    RndDrawable *pMisses = FindObject<RndDrawable>("gem_miss0.mm");
    mTransparentView->AddDraw(pSlots, pMisses);
    if (TheGfxManager.mMonkeyGems != 0) {
        RndDrawable *pMonkeys = FindObject<RndDrawable>("monkey0.mm");
        FindObject<RndDrawable>("tnl opaque")->AddDraw(pMonkeys, nullptr);
    }
    mTrackGemGroups.resize(mNumTracks);
    UpdateTrackGemGroups();
    if (TheGfxManager.mBlackPanels != 0) {
        FindObject<RndMesh>("panel mat scale.mesh")->SetShowing(0);
    }

    int nMainPlayer = -1;
    int nLocalPlayers = 0;
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        int nCam = -1;
        if (TheGameDb->IsLocalPlayer(i)) {
            nCam = nLocalPlayers;
            nMainPlayer = i;
            ++nLocalPlayers;
        }
        const char nOtherExcluded =
            (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) ? -1 : mSlotGemGroup;
        TnlPlayer *pPlayer = new TnlPlayer(i,
                                           nCam,
                                           TheGameDb->GetPlayerColor(i),
                                           this,
                                           mGeom,
                                           nOption,
                                           mMissGemGroup,
                                           nOtherExcluded);
        mPlayers.push_back(pPlayer);
        pPlayer->LoadConfig(pConfig, pDefaults, this, 0, i == 0);
    }
    mCamFX = new PlayerCamFX(nMainPlayer, mNumTracks, mArena->mUnlockIndex == kBossArena);
    if (TheGameDb->mCommunity != GameDb::kCommunityLocal) {
        mStreakGems.resize(mNumTracks);
        for (int i = 0; i < mNumTracks; ++i) {
            mStreakGems[i] = new TnlStreakGemFX(i, nMainPlayer);
        }
    }
    mDivider = new TnlDivider();
    mSavedClearColor = TheRnd->mClearColor;
    if (mArena->mEnviron != nullptr) {
        TheRnd->SetClearColor(mArena->mEnviron->mFogColor);
    }
    SetShowPanels(sShowPanels);
    SetShowDividers(sShowDividers);
    SetShowActivator(sShowActivator);
    mGeom->mMinFirstBar = kLeadInBar;
    if (1.0f <= fSongTicks) {
        mGeom->mEndBar = static_cast<int>(floorf(fSongTicks * mGeom->mBarsPerTick)) + 1;
    }
    mGeom->Poll(fStartTick, 0.0f);
    TheTriggerMgr.Load(kTriggersFile, TheGfxManager.FindPreload(kTriggersFile));
    ScriptFunction::Register(OnBlendToPath, "blend_to_path", this);
    ScriptFunction::Register(OnArenaDoneIn, "arena_done_in", this);
    ScriptFunction::Register(OnSetParticlePlayerColor, "set_particle_player_color", this);
    ScriptFunction::Register(OnGfxDebug, "gfx_debug", this);
    ScaleHudGems(TheGfxManager.mScrollSpeed);
    timer.Stop(); // Yes, the binary never stops loadTimer.
}

int GfxTunnel::BuildOuterCams() {
    (void)TheGameDb->GetNumPlayers(); // Yes, the binary discards this call's result.
    int nCams = 1;
    if (sSplitScreen != 0) {
        nCams = TheGameDb->GetNumPads();
    }
    RndCam *pOuterTemplate = FindObject<RndCam>("outer 1.cam");
    RndCam *pTemplate = FindObject<RndCam>(FormatString("tnl cam%d", nCams));
    mOverlayCam = FindObject<RndCam>("tnl overlay.cam");
    // Yes, the field of view is the radians LoadConfig() left in sCameraFov, and the outer clip
    // distances are not scaled by the scroll speed.
    const float flFov = sCameraFov;
    const float flFar = pTemplate->mFarPlane * TheGfxManager.mScrollSpeed;
    const float flNear = pTemplate->mNearPlane * TheGfxManager.mScrollSpeed;
    const float flOuterFar = pOuterTemplate->mFarPlane;
    const float flOuterNear = pOuterTemplate->mNearPlane;
    Rect rect = pTemplate->mScreenRect;
    mOuterCams.clear();
    mOuterCams.reserve(nCams * kCamsPerPlayer);

    int i;
    for (i = 0; i < nCams; ++i) {
        const int n = i + 1;
        RndCam *pCam = FindObject<RndCam>(FormatString("tnl cam%d", n));
        RndCam *pCamZ = FindObject<RndCam>(FormatString("tnl cam%dz", n));
        RndCam *pWinCam = FindObject<RndCam>(FormatString("tnl win cam%d", n));
        RndCam *pOuter = FindObject<RndCam>(FormatString("outer %d.cam", n));
        RndCam *pOuterNear = FindObject<RndCam>(FormatString("outer near %d.cam", n));
        RndView *pLocal = FindObject<RndView>(FormatString("tnl local%d.view", n));
        mOuterCams.push_back(pCam);
        mOuterCams.push_back(pCamZ);
        mOuterCams.push_back(pWinCam);
        mOuterCams.push_back(pOuter);
        mOuterCams.push_back(pOuterNear);

        rect.x = static_cast<float>(i % kScreenColumns) * kScreenCell;
        rect.y = static_cast<float>(i / kScreenColumns) * kScreenCell;
        pCam->SetFrustum(flNear, flFar, flFov);
        pCam->mScreenRect = rect;
        pCam->UpdateProjection();
        pCam->mZRange = sTnlZ;

        pCamZ->mScreenRect = pCam->mScreenRect;
        pCamZ->UpdateProjection();
        pCamZ->SetFrustum(flNear, flFar, flFov);
        pCamZ->mZRange = sTnlzZ;

        mOverlayCam->mScreenRect = rect;
        mOverlayCam->UpdateProjection();

        pWinCam->SetFrustum(pWinCam->mNearPlane, pWinCam->mFarPlane, flFov);
        pWinCam->mScreenRect = rect;
        pWinCam->UpdateProjection();
        pWinCam->mZRange = Vector2{sTnlzZ.y, sTnlzZ.y};
        pLocal->SetShowing(1);

        pOuter->mScreenRect = rect;
        pOuter->UpdateProjection();
        pOuter->SetFrustum(flOuterNear, flOuterFar, flFov);
        pOuter->mZRange = sArenaZ;
        pOuter->SetShowing(1);

        pOuterNear->mScreenRect = rect;
        pOuterNear->UpdateProjection();
        pOuterNear->SetFrustum(flNear, flFar, flFov);
        pOuterNear->mZRange = sTnlzZ;
        pOuterNear->SetShowing(1);

        ScaleTransKeys(FormatString("tnl cam intro%d.tnm", n), TheGfxManager.mScrollSpeed);
    }
    return i;
}

RndView *GfxTunnel::CreateMainView() {
    DataArray *pConfig = GetModeGfxConfig();
    DataArray *pDefaults = GetGfxConfig();
    // Yes, the binary loads `camera_fov` again in degrees over the radians LoadConfig() left.
    FindConfigFloat(pConfig, pDefaults, "camera_fov", &sCameraFov, true);
    FindConfigVector2(pConfig, pDefaults, "tnlz_z", &sTnlzZ, true);
    RndCam *pCam = FindObject<RndCam>("tnl cam1");
    const float flFov = DegreesToRadians(sCameraFov);
    const float flNear = pCam->mNearPlane * TheGfxManager.mScrollSpeed;
    const float flFar = pCam->mFarPlane * TheGfxManager.mScrollSpeed;
    Rect rect = pCam->mScreenRect;
    RndCam *pCamZ = FindObject<RndCam>("tnl cam1z");
    RndView *pLocal = FindObject<RndView>("tnl local1.view");
    RndView *pSlide = FindObject<RndView>("tnl cam slide1");
    RndView *pCamFX = FindObject<RndView>("tnl cam fx1");

    rect.x = 0.0f;
    rect.y = 0.0f;
    pCamZ->mScreenRect = rect;
    pCamZ->UpdateProjection(); // Yes, SetFrustum() rebuilds the projection again.
    pCamZ->SetFrustum(flNear, flFar, flFov);
    pCamZ->mZRange = sTnlzZ;

    const Transform identity{Vector3{1.0f, 0.0f, 0.0f},
                             Vector3{0.0f, 1.0f, 0.0f},
                             Vector3{0.0f, 0.0f, 1.0f},
                             Vector3{0.0f, 0.0f, 0.0f}};
    pSlide->mLocalXfm = identity;
    pSlide->mDirty = 1;
    pCamFX->mLocalXfm = identity;
    pCamFX->mDirty = 1;
    TnlPlayer::LoadCamXfm(pConfig, pDefaults, pCam);
    (void)pSlide->UpdateWorldXfm(nullptr, 0); // Yes, the binary discards this call's result.
    pLocal->SetShowing(1);
    return pLocal;
}

void GfxTunnel::LoadViews(std::vector<RndView *> *pViews, const char *pszFormat) {
    for (int i = 1;; ++i) {
        RndView *pView = FindObject<RndView>(FormatString(pszFormat, i));
        if (pView == nullptr) {
            break;
        }
        ClearMeshSpheres(pView);
        pViews->push_back(pView);
    }
    if (pViews->empty()) {
        DebugWarn("Couldn't find gem captures for %s", pszFormat);
    }
}

void GfxTunnel::LoadTrackColors() {
    const char *pszSuffix;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo ||
        TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        pszSuffix = "";
    } else {
        pszSuffix = " multi";
    }
    mInstrumentColors.resize(GfxManager::kNumInstruments);
    for (int i = 0; i < GfxManager::kNumInstruments; ++i) {
        const char *pszInstrument = "";
        switch (i) {
        case GfxManager::kInstrumentDrum:
            pszInstrument = "drum";
            break;
        case GfxManager::kInstrumentBass:
            pszInstrument = "bass";
            break;
        case GfxManager::kInstrumentSynth:
            pszInstrument = "synth";
            break;
        case GfxManager::kInstrumentGuitar:
            pszInstrument = "guitar";
            break;
        case GfxManager::kInstrumentVocal:
            pszInstrument = "vocal";
            break;
        case GfxManager::kInstrumentFx:
            pszInstrument = "fx";
            break;
        default:
            DebugWarn("unknown instrument: %d", i);
            break;
        }
        RndMat *pMat =
            FindObject<RndMat>(FormatString("panel mat %s%s.mat", pszInstrument, pszSuffix));
        mInstrumentColors[i] = pMat->mBaseColor;
        mInstrumentColors[i].a = 1.0f;
    }
    mTrackColors.resize(mNumTracks);
    if (TheGfxManager.mBlackPanels == 0) {
        for (int i = 0; i < mNumTracks; ++i) {
            mTrackColors[i] = mInstrumentColors[GetTrackInstrument(i)];
        }
    } else {
        for (int i = 0; i < mNumTracks; ++i) {
            mTrackColors[i] = Color{0.0f, 0.0f, 0.0f, 1.0f};
        }
    }
}

GfxTunnel::~GfxTunnel() {
    ScriptFunction::Unregister(OnGfxDebug);
    ScriptFunction::Unregister(OnSetParticlePlayerColor);
    ScriptFunction::Unregister(OnArenaDoneIn);
    ScriptFunction::Unregister(OnBlendToPath);
    TheRnd->SetClearColor(mSavedClearColor);
    delete mCamFX;
    mOuterCams.clear();
    mTrackColors.clear();
    delete mGems;
    for (auto it = mFlashes.begin(); it != mFlashes.end(); ++it) {
        delete *it;
    }
    for (auto it = mGemCaps.begin(); it != mGemCaps.end(); ++it) {
        delete *it;
    }
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        delete *it;
    }
    for (auto it = mCrippleFX.begin(); it != mCrippleFX.end(); ++it) {
        delete *it;
    }
    for (auto it = mTrackFX.begin(); it != mTrackFX.end(); ++it) {
        delete *it;
    }
    delete mWinShell;
    delete mGemErase;
    delete mSpotLights;
    delete mSlowdown;
    delete mBlurs;
    for (auto it = mFires.begin(); it != mFires.end(); ++it) {
        delete *it;
    }
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        delete *it;
    }
    for (auto it = mPanels.begin(); it != mPanels.end(); ++it) {
        delete *it;
    }
    mPanels.clear();
    delete mDivider;
    delete mArms;
    delete mGeom;
}

void GfxTunnel::UpdateTrackGemGroups() {
    for (int i = 0; i < mNumTracks; ++i) {
        mTrackGemGroups[i] = mInstrumentGemGroups[mTracks[i].mInstrument];
    }
}

void GfxTunnel::SetShowPanels(int bShow) {
    mGeom->mShowPanels = bShow;
}

void GfxTunnel::SetShowDividers(int bShow) {
    mDivider->mEnabled = bShow;
}

void GfxTunnel::SetShowActivator(int bShow) {
    const char colors[] = {'g', 'p', 'r', 'y'};
    for (int c = 0; c < kNumTargetColors; ++c) {
        for (int n = 0; n < kNumTargetSteps; ++n) {
            RndDrawable *pTarget =
                FindObject<RndDrawable>(FormatString("target %d_%c.mesh", n, colors[c]));
            if (pTarget != nullptr) {
                pTarget->SetShowing(bShow);
            }
        }
    }
}

void GfxTunnel::BuildPanels(int bResetPanels, RndTransAnim *pCamPath) {
    // Yes, the timer is started and never stopped.
    Timer timer;
    timer.Start();

    auto panel = mPanels.begin();
    while (panel != mPanels.end()) {
        delete *panel;
        panel = mPanels.erase(panel);
    }

    sSegments = sShape;
    sTrackPoints.resize(mNumTracks);
    if (static_cast<int>(sSegments.size()) != mNumTracks) {
        if (!sSegments.empty()) {
            DebugNotify("track count in tunnel shape (%d tracks)\n"
                        "did not match track count in game (%d)",
                        static_cast<int>(sSegments.size()),
                        mNumTracks);
        }
        sSegments.resize(mNumTracks);
        const float flHalfStep = kPi / static_cast<float>(mNumTracks);
        const float flRadius = 1.0f / tanf(flHalfStep);
        float flAngle = -flHalfStep;
        for (int i = 0; i < mNumTracks; ++i) {
            const float flNext = flAngle + (flHalfStep + flHalfStep);
            Segment &segment = sSegments[i];
            segment.first.x = flRadius * sinf(flAngle);
            segment.first.y = -flRadius * cosf(flAngle);
            segment.second.x = flRadius * sinf(flNext);
            segment.second.y = -flRadius * cosf(flNext);
            flAngle = flNext;
        }
    }

    if (sInvertTunnel != 0 && TheGfxManager.mTunnelShape != kTunnelShapeCircular) {
        for (int i = 0, j = mNumTracks - 1; i < j; ++i, --j) {
            std::swap(sSegments[i].first, sSegments[j].second);
            std::swap(sSegments[i].second, sSegments[j].first);
        }
    }
    if (sFlipTunnel != 0 && TheGfxManager.mTunnelShape != kTunnelShapeCircular) {
        for (int i = 0; i < mNumTracks; ++i) {
            sSegments[i].first.y = -sSegments[i].first.y;
            sSegments[i].second.y = -sSegments[i].second.y;
        }
    }
    if (sCenterTunnel != 0) {
        const Segment &left = sSegments[(mNumTracks - 1) / 2];
        const Segment &right = sSegments[mNumTracks / 2];
        Vector3 leftNormal = SegmentNormal(left);
        Vector3 rightNormal = SegmentNormal(right);
        Vec3Normalize(&leftNormal.x, &leftNormal.x);
        Vec3Normalize(&rightNormal.x, &rightNormal.x);
        Vector3 normal{leftNormal.x + rightNormal.x,
                       leftNormal.y + rightNormal.y,
                       leftNormal.z + rightNormal.z};
        Vec3Normalize(&normal.x, &normal.x);
        const Vector2 center{((left.second.x + right.first.x) * 0.5f) + normal.x,
                             ((left.second.y + right.first.y) * 0.5f) + normal.z};
        for (int i = 0; i < mNumTracks; ++i) {
            sSegments[i].first.x -= center.x;
            sSegments[i].first.y -= center.y;
            sSegments[i].second.x -= center.x;
            sSegments[i].second.y -= center.y;
        }
    }
    for (int i = 0; i < mNumTracks; ++i) {
        sSegments[i].first.x *= TheGfxManager.mScrollSpeed;
        sSegments[i].first.y *= TheGfxManager.mScrollSpeed;
        sSegments[i].second.x *= TheGfxManager.mScrollSpeed;
        sSegments[i].second.y *= TheGfxManager.mScrollSpeed;
    }

    const Segment &left = sSegments[(mNumTracks - 1) / 2];
    const Segment &right = sSegments[mNumTracks / 2];
    const Vector2 middle{left.second.x + right.first.x, left.second.y + right.first.y};
    mCenter.x = middle.x * 0.5f;
    // Yes, the binary halves the depth in double precision.
    mCenter.z = static_cast<float>(static_cast<double>(middle.y) * 0.5);
    mCenter.y = 0.0f;
    for (int i = mNumTracks; i > 0; --i) {
        // Yes, the binary counts down an empty loop over the tracks.
    }

    for (unsigned i = 0; i < sSegments.size(); ++i) {
        const Vector3 normal = SegmentNormal(sSegments[i]);
        sTrackPoints[i] = Vector2{normal.x, normal.z};
        Rnd::Normalize(sTrackPoints[i], sTrackPoints[i]);
    }

    mGeom->SetPanelMat(FindObject<RndMat>("panel mat highlight.mat"), kHighlightPanelKind);
    for (int i = 0; i < kNumIntroPanels; ++i) {
        mGeom->SetPanelMat(FindObject<RndMat>(FormatString("panel intro %d.mat", i + 1)), i);
    }
    const char *pszSuffix;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo ||
        TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        pszSuffix = "";
    } else {
        pszSuffix = " multi";
    }
    float flSlabDepth = sSlabDepth;
    if (TheGfxManager.mTunnelShape == kTunnelShapeFlipped &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        flSlabDepth += flSlabDepth;
    }
    RndMat *pBassMat = FindObject<RndMat>(FormatString("panel mat bass%s.mat", pszSuffix));
    RndMatAnim *pBassAnim =
        FindObject<RndMatAnim>(FormatString("panel mat bass serge%s.mnm", pszSuffix));
    // Yes, the binary discards whether the geometry has a track.
    (void)mGeom->Init(&sSegments,
                      sTunnelBars,
                      kPanelDetail,
                      sTunnelLowResBars,
                      flSlabDepth,
                      sSlabAngle,
                      pCamPath,
                      &mTrackColors,
                      pBassMat,
                      pBassAnim);
    if (bResetPanels != 0) {
        for (int nTrack = 0; nTrack < mGeom->mNumTracks; ++nTrack) {
            for (int nBar = 0; nBar < mGeom->mNumBars; ++nBar) {
                mGeom->SetPanelState(nTrack, nBar, 1, nullptr, 0, true, nullptr);
            }
        }
    }

    TnlFire::sChunkScales.resize(sSegments.size());
    const Segment &first = sSegments[0];
    const Vector2 width{first.first.x - first.second.x, first.first.y - first.second.y};
    const float flWidth = sqrtf((width.x * width.x) + (width.y * width.y));
    sPanelWidth = flWidth;
    const float flChunkSize = flWidth * kChunkWidths;
    const Vector3 chunkScale{flChunkSize, flChunkSize, TheGfxManager.mScrollSpeed};
    TnlFire::sPanelSize = flChunkSize;
    for (auto it = TnlFire::sChunkScales.begin(); it != TnlFire::sChunkScales.end(); ++it) {
        *it = chunkScale;
    }
    LayoutDividers(&sSegments, TheGfxManager.mScrollSpeed);

    const float flPanelSize = TnlFire::sPanelSize;
    const float flScrollSpeed = TheGfxManager.mScrollSpeed;
    sBurstScale.z = flScrollSpeed;
    TnlGrid::sQuarterScale.x = sGridQuarterScale.x * flPanelSize;
    TnlGrid::sQuarterScale.z = sGridQuarterScale.z * flPanelSize;
    TnlGrid::sEighthScale.x = sGridEighthScale.x * flPanelSize;
    TnlGrid::sEighthScale.z = sGridEighthScale.z * flPanelSize;
    sBurstScale.x = sMeshGemScale * flScrollSpeed;
    sBurstScale.y = sMeshGemScale * flScrollSpeed;
    TnlGrid::sQuarterScale.y = sGridQuarterScale.y * flPanelSize;
    TnlGrid::sEighthScale.y = sGridEighthScale.y * flPanelSize;
}

bool GfxTunnel::HasActiveBar(int nTrack, float flTick) {
    bool bActive = false;
    int bFound;
    const auto bar = mTracks[nTrack].FindBar(flTick, TheGameDb->mSongTick, &bFound);
    if (bFound != 0 && bar->mOwner < 0) {
        bActive = bar->mInSong != 0;
    }
    return bActive;
}

void GfxTunnel::OnGemRemoved(const TnlGem *pGem) {
    mSpotLights->RemoveLight(pGem->mTrack, pGem->mTick);
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mCursor.OnGemRemoved(pGem);
    }
}

void GfxTunnel::CollectGemTicks(std::vector<float> *pTicks,
                                char nTrack,
                                float flFromTick,
                                float flToTick) {
    int nFree = static_cast<int>(pTicks->capacity() - pTicks->size());
    if (nFree == 0) {
        return;
    }
    std::list<BarData> &bars = mTracks[nTrack].mBars;
    const float flFirstTick =
        static_cast<float>(static_cast<int>(floorf(flFromTick * mGeom->mBarsPerTick))) * kBarTicks;
    float flNextTick = flFirstTick;
    for (auto it = bars.begin(); it != bars.end(); ++it) {
        // Yes, the binary tests each bar against the first bar, not against flNextTick.
        if (it->mTick + it->mTicks <= flFirstTick) {
            continue;
        }
        if (flToTick <= it->mTick) {
            return;
        }
        flNextTick = it->mTick + kBarTicks;
        if (it->mInSong != 0 && it->mOwner < 0) {
            pTicks->push_back(it->mTick);
            if (--nFree == 0) {
                return;
            }
        }
    }
    if (nFree == 0) {
        return;
    }
    while (flNextTick < flToTick) {
        pTicks->push_back(flNextTick);
        flNextTick += kBarTicks;
        if (--nFree == 0) {
            return;
        }
    }
}

Particle *GfxTunnel::Flash(const Vector3 *pPos, float flSize, float flGrowth) {
    for (auto it = mFlashes.begin(); it != mFlashes.end(); ++it) {
        Particle *pParticle = (*it)->Start(pPos, flSize, flGrowth);
        if (pParticle != nullptr) {
            return pParticle;
        }
    }
    return nullptr;
}

bool GfxTunnel::Burst(
    const std::vector<RndView *> *pViews, char nTrack, bool bErase, float flTick, float flLateral) {
    RndView *pView = (*pViews)[RandomInt(0, static_cast<int>(pViews->size()))];
    for (auto it = mGemCaps.begin(); it != mGemCaps.end(); ++it) {
        if ((*it)->IsIdle()) {
            (*it)->Start(this, nTrack, pView, flTick, flLateral, &sBurstScale, bErase);
            return true;
        }
    }
    return false;
}

void GfxTunnel::StartFire(int nPlayer,
                          int nTrack,
                          int bAuto,
                          float flTick,
                          float flStartTick,
                          float flEndTick,
                          const Color *pInnerColor,
                          const Color *pOuterColor,
                          const std::vector<float> *pGemTicks) {
    TnlFire *pOldest = nullptr;
    float flOldestTick = kNeverTick;
    const Color *pTrackColor = mGeom->GetTrackColor(nTrack);
    for (auto it = mFires.begin(); it != mFires.end(); ++it) {
        if ((*it)->Start(nPlayer,
                         nTrack,
                         bAuto,
                         pInnerColor,
                         pOuterColor,
                         pTrackColor,
                         pGemTicks,
                         flTick,
                         flStartTick,
                         flEndTick)) {
            return;
        }
        const float flFireTick = (*it)->StartTick(nPlayer);
        if (flFireTick < flOldestTick) {
            pOldest = *it;
            flOldestTick = flFireTick;
        }
    }
    if (pOldest != nullptr) {
        pOldest->Reset(true);
        // Yes, the binary discards whether the fire started.
        (void)pOldest->Start(nPlayer,
                             nTrack,
                             bAuto,
                             pInnerColor,
                             pOuterColor,
                             pTrackColor,
                             pGemTicks,
                             flTick,
                             flStartTick,
                             flEndTick);
    }
}

void GfxTunnel::StartFade(bool bToBlack) {
    const float flNow = SystemMs();
    mFadeCurve.Reset(
        mFadeCurve.Eval(flNow), bToBlack ? 1.0f : 0.0f, flNow, flNow + sFadeToBlackDuration);
}

int GfxTunnel::GetTrackInstrument(int nTrack) {
    return mTracks[nTrack].mInstrument;
}

void GfxTunnel::SetTrackInstrument(int nTrack, int nInstrument) {
    mTracks[nTrack].mInstrument = nInstrument;
    UpdateTrackGemGroups();
    const Color *pColor = &mInstrumentColors[nInstrument];
    mGeom->SetTrackColor(nTrack, pColor);
    if (TheGfxManager.mBlackPanels == 0) {
        mTrackColors[nTrack] = *pColor;
    }
}

int GfxTunnel::GetTrackType(int nTrack) {
    return mTracks[nTrack].mType;
}

int GfxTunnel::GetPlayerTrack(int nPlayer) {
    return mGeom->GetPlayer(nPlayer)->mTrack;
}

bool GfxTunnel::IsActivatorIdle(int nPlayer) {
    return mPlayers[nPlayer]->mActivator.mPlace.mY1 == 0.0f;
}

bool GfxTunnel::IsTrackOpen(int nTrack) const {
    return mTrackFX[nTrack]->mState != TnlTrackFX::kStateEnable;
}

bool GfxTunnel::IsTrackCaptured(int nTrack) const {
    return mTrackFX[nTrack]->mState == TnlTrackFX::kStateCapture;
}

bool GfxTunnel::IsTrackCaptured(char nTrack) const {
    return mTrackFX[nTrack]->mState == TnlTrackFX::kStateCapture;
}

const Vector3 *GfxTunnel::GetHudOffset() {
    return &mCamFX->mScreenJiggle;
}

float GfxTunnel::GetFreestyleConstraint() {
    return mArena->mFreestyle;
}

void GfxTunnel::AddPanel(GfxTunnelPanel *pPanel, int bDeferred, float flRevealTick) {
    mPanels.push_back(pPanel);
    pPanel->SetDeferred(bDeferred, flRevealTick);
}

std::list<GfxTunnel::BarData>::iterator
GfxTunnel::TrackData::FindBar(float flTick, float flNow, int *pbFound) {
    auto it = mBars.begin();
    while (it != mBars.end()) {
        const float flEnd = it->mTick + it->mTicks;
        if (flTick < flEnd) {
            *pbFound = (it->mTick <= flTick) ? 1 : 0;
            return it;
        }
        if (kBarKeepTicks < flNow - flEnd) {
            it = mBars.erase(it);
        } else {
            ++it;
        }
    }
    *pbFound = 0;
    return it;
}

TnlPlayer *GfxTunnel::GetPlayer(int nPlayer) {
    return mPlayers[nPlayer];
}

void GfxTunnel::Poll(float fTicks, float fTime) {
    const float flTime = TheGameDb->mSongTime;
    const float flTick = TheGameDb->mSongTick;
    if (mTrackPlayersDirty != 0) {
        mTrackPlayersDirty = 0;
        for (auto track = mTracks.begin(); track != mTracks.end(); ++track) {
            const signed char nNumPlaces = static_cast<signed char>(track->mPlayers.size());
            signed char nPlace = 0;
            for (auto it = track->mPlayers.begin(); it != track->mPlayers.end(); ++it) {
                TnlPlayer *pPlayer = GetPlayer(*it);
                pPlayer->mGrid.SetOrder(nPlace);
                pPlayer->mActivator.SetPlace(nPlace, nNumPlaces);
                ++nPlace;
            }
        }
    }

    if (mPanelColorsDirty != 0) {
        mPanelColorsDirty = 0;
        mGeom->SetAllPanelColors(nullptr, true);
        for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
            TnlCursor &cursor = (*it)->mCursor;
            const char nTrack = cursor.mTargetTrack;
            if (nTrack < 0) {
                continue;
            }
            const float flEndTick = cursor.mTargetEndTick;
            for (float flBarTick = cursor.mTargetStartTick; flBarTick < flEndTick;
                 flBarTick += kBarTicks) {
                const int nBar = static_cast<int>(floorf(flBarTick * mGeom->mBarsPerTick));
                mGeom->SetPanelColor(nTrack, nBar, &cursor.mColor, false);
            }
        }
    }

    mChangedRange.Clear();
    for (auto it = mTrackFX.begin(); it != mTrackFX.end(); ++it) {
        if ((*it)->Poll(this, mGeom)) {
            const char nTrack = (*it)->mTrack;
            mChangedRange.Expand(nTrack, static_cast<char>(nTrack + 1), -kNeverTick, kNeverTick);
        }
    }
    mCrippledTracks = 0;
    for (auto it = mCrippleFX.begin(); it != mCrippleFX.end(); ++it) {
        if ((*it)->Poll(fTime, this)) {
            char nTrack;
            char nPrevTrack;
            (*it)->GetWaveTracks(&nTrack, &nPrevTrack);
            if (nTrack >= 0) {
                mChangedRange.Expand(
                    nTrack, static_cast<char>(nTrack + 1), -kNeverTick, kNeverTick);
            }
            if (nPrevTrack >= 0) {
                mChangedRange.Expand(
                    nPrevTrack, static_cast<char>(nPrevTrack + 1), -kNeverTick, kNeverTick);
            }
        }
        const char nArrived = (*it)->ArrivedWaveTrack();
        if (nArrived >= 0) {
            mCrippledTracks = static_cast<signed char>(mCrippledTracks | (1 << nArrived));
        }
    }

    mGeom->Poll(flTick, fTicks);
    if (mChangedRange.mFirstTrack < mChangedRange.mEndTrack) {
        RefreshChangedRange();
        mGems->Poll(&mChangedRange);
    } else {
        mGems->Poll(nullptr);
    }
    auto panel = mPanels.begin();
    while (panel != mPanels.end()) {
        if ((*panel)->Update(this, flTick)) {
            ++panel;
        } else {
            delete *panel;
            panel = mPanels.erase(panel);
        }
    }
    for (auto it = mFlashes.begin(); it != mFlashes.end(); ++it) {
        (*it)->Poll(fTime);
    }
    for (auto it = mFires.begin(); it != mFires.end(); ++it) {
        (*it)->Update(fTicks, fTime);
    }
    mSlowdown->Update();
    mWinShell->Update();
    mSpotLights->Poll();
    mGemErase->Poll();
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        (*it)->Poll(fTime, this);
    }
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mFreestyle.Poll(fTicks, fTime);
    }

    if (mPathBlending != 0) {
        const float flBlend = mPathBlendCurve.Eval(flTime);
        mCamFX->SetBlend(flBlend);
        if (flBlend == 0.0f && mPathBlendCurve.mX1 < flTime) {
            mCamFX->SetBlendAlgorithm(nullptr, 0.0f);
            mPathBlending = 0;
        }
    }
    mCamFX->Poll(this, flTick, flTime);
    mDivider->Poll(fTime, this);
    mArms->Poll(flTick);
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->Update(fTicks, fTime);
    }

    if (mTrackColorLight != nullptr) {
        const float flPosition = mGeom->GetPlayer(mTrackColorPlayer)->mPosition;
        const float flFloor = floorf(flPosition);
        const int nTrack = static_cast<int>(flFloor);
        const float flFraction = flPosition - flFloor;
        int nFrom = nTrack % mNumTracks;
        if (nFrom < 0) {
            nFrom += mNumTracks;
        }
        int nTo = (nTrack + 1) % mNumTracks;
        if (nTo < 0) {
            nTo += mNumTracks;
        }
        const Color &from = mTrackColors[nFrom];
        const Color &to = mTrackColors[nTo];
        const float flRest = 1.0f - flFraction;
        Color color{(to.r * flFraction) + (from.r * flRest),
                    (to.g * flFraction) + (from.g * flRest),
                    (to.b * flFraction) + (from.b * flRest),
                    (to.a * flFraction) + (from.a * flRest)};
        const float flFade = sTrackLightFade.Eval(flTick);
        color.r *= flFade;
        color.g *= flFade;
        color.b *= flFade;
        color.a *= flFade;
        mTrackColorLight->SetColor(color);
    }
    mBlurs->Update();
    if (mBossJourneyDoneTick <= flTick) {
        mBossJourneyDone = 1;
    }
    if (mResultDoneTick <= flTick) {
        mIdle = 1;
    }
}

void GfxTunnel::PollCameras([[maybe_unused]] float fTicks, [[maybe_unused]] float fTime) {
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        // Yes, the binary passes the ticks and the time on, and UpdateTrail() does not read them.
        (*it)->UpdateTrail();
    }
}

void GfxTunnel::SetLetterbox(float fLetterbox) {
    if (fLetterbox == mLetterbox) {
        return;
    }
    mLetterbox = fLetterbox;
    const float flFov = ((sLetterboxFov - sCameraFov) * fLetterbox) + sCameraFov;
    for (auto it = mOuterCams.begin(); it != mOuterCams.end(); ++it) {
        (*it)->SetFrustum((*it)->mNearPlane, (*it)->mFarPlane, flFov);
    }
}

void GfxTunnel::DrawBackground() {
    for (auto it = mFires.begin(); it != mFires.end(); ++it) {
        (*it)->Draw();
    }
    mDivider->Draw();
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo ||
        TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        mGeom->DrawPanels();
    } else {
        mGeom->DrawPanelsMultiplayer();
    }
}

void GfxTunnel::DrawTracks() {
    mGems->DrawComposites();
    static_cast<RndDrawable *>(mGridView)->Draw();
    static_cast<RndDrawable *>(mTransparentView)->Draw();
    mSpotLights->Draw();
    for (auto it = mGemCaps.begin(); it != mGemCaps.end(); ++it) {
        (void)(*it)->Draw(mGeom); // Yes, the binary discards whether the burst ended.
    }
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        (*it)->Draw();
    }
    for (auto it = mCrippleFX.begin(); it != mCrippleFX.end(); ++it) {
        (*it)->Draw(this);
    }
    static_cast<RndDrawable *>(Ship::sBeamView)->Draw();
    TnlCripFX::DrawParticles();
}

void GfxTunnel::DrawCameras() {
    mGeom->DrawOverlays();
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mFreestyle.Draw();
    }
}

void GfxTunnel::DrawForeground() {
    static_cast<RndDrawable *>(mOverlayCam)->Draw();
    for (auto track = mTracks.begin(); track != mTracks.end(); ++track) {
        for (auto it = track->mPlayers.end(); it != track->mPlayers.begin();) {
            --it;
            TheGfxManager.mShips[*it]->DrawArrow();
        }
    }
    const float flOpacity = mFadeCurve.Eval(SystemMs());
    if (flOpacity != 0.0f) {
        mFadeMesh->mMat->SetAlpha(flOpacity);
        static_cast<RndDrawable *>(mFadeMesh)->Draw();
        TheGfxManager.mVortexAfterViews = 1;
    }
    mSlowdown->Draw();
}

void GfxTunnel::RefreshChangedRange() {
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        TnlGrid &grid = (*it)->mGrid;
        // Yes, the binary compares the low byte of the track of the grid.
        const char nTrack = static_cast<char>(grid.mTrack);
        if (nTrack >= mChangedRange.mFirstTrack && nTrack < mChangedRange.mEndTrack) {
            grid.RefreshTrack(grid.mTrack, true);
        }
        (*it)->mCursor.OnRangeChanged(&mChangedRange);
    }
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        (*it)->Refresh(this);
    }
    mSpotLights->UpdateRange(mGeom, &mChangedRange);
}

float GfxTunnel::PanelWidth() const {
    return sPanelWidth;
}

float GfxTunnel::BurstSpinPeriod() {
    return sGemRotatePeriod;
}

void GfxTunnel::ScaleLights(RndEnviron *pEnviron) {
    for (auto it = pEnviron->mLights.begin(); it != pEnviron->mLights.end(); ++it) {
        const float flScale = TheGfxManager.mInvScrollSpeed;
        const Color &diffuse = (*it)->mColor;
        const Color color{
            diffuse.r * flScale, diffuse.g * flScale, diffuse.b * flScale, diffuse.a * flScale};
        (*it)->SetColor(color);
    }
}

void GfxTunnel::OnBlendToPath(DataArray *pCommand, void *pUserData) {
    GfxTunnel *pTunnel = static_cast<GfxTunnel *>(pUserData);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        return;
    }
    const float flLength = pCommand->Float(1);
    RndTransAnim *pAnim = FindObject<RndTransAnim>(pCommand->Sym(2));
    const float flFrame = pCommand->Float(3);
    int bLoop = 0;
    float flLoopStart = 0.0f;
    float flLoopEnd = 0.0f;
    if (pCommand->Size() >= kBlendLoopArgs) {
        flLoopStart = pCommand->Float(4);
        bLoop = 1;
        flLoopEnd = pCommand->Float(5);
    }
    const int bUseTime = (TheTriggerMgr.mClock == TriggerMgr::kClockReal) ? 1 : 0;
    const float flStart = (bUseTime != 0) ? TheGameDb->mSongTime : TheGameDb->mSongTick;
    pTunnel->mGeom->BlendPath(
        pAnim, bLoop, bUseTime, flLength, flStart, flFrame, flLoopStart, flLoopEnd);
}

void GfxTunnel::OnArenaDoneIn(DataArray *pCommand, void *pUserData) {
    GfxTunnel *pTunnel = static_cast<GfxTunnel *>(pUserData);
    if (pCommand->Size() == kArenaDoneInArgs && pCommand->Type(1) == DataArray::kNodeInt &&
        pCommand->Type(2) == DataArray::kNodeSymbol) {
        const float flValue = pCommand->Float(1);
        const char *pszCommand = pCommand->Sym(2);
        if (strcmp(pszCommand, "unlock") == 0) {
            pTunnel->mUnlockDoneTicks = flValue / *TheGfxManager.mMsPerTick;
            return;
        }
        if (strcmp(pszCommand, "boss") == 0) {
            pTunnel->mBossDoneTicks = flValue / *TheGfxManager.mMsPerTick;
            return;
        }
        if (strcmp(pszCommand, "all") == 0) {
            // Yes, the binary does not convert the value to ticks here.
            pTunnel->mBossDoneTicks = flValue;
            pTunnel->mUnlockDoneTicks = flValue;
            return;
        }
    }
    DebugWarn("Usage: %s <ticks> <cmd>, where cmd is one of:\nunlock, boss, or all",
              pCommand->Sym(0));
}

void GfxTunnel::OnSetParticlePlayerColor(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    // Yes, the binary does not test the particle system for null.
    RndParticleSys *pSys = FindObject<RndParticleSys>(pCommand->Sym(1));
    DataArray *pColor =
        pCommand->Array(TheGameDb->GetPlayerSlot(TheTriggerMgr.mPlayer) + kFirstSlotColorArg);
    Color color{pColor->Float(0), pColor->Float(1), pColor->Float(2), 1.0f};
    color.a = pCommand->Float(kStartAlphaArg);
    pSys->mStartColorLow = color;
    pSys->mStartColorHigh = color;
    color.a = pCommand->Float(kEndAlphaArg);
    pSys->mEndColorLow = color;
    pSys->mEndColorHigh = color;
}

void GfxTunnel::OnGfxDebug([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    static_cast<GfxTunnel *>(pUserData)->PrintBars(0);
}

void GfxTunnel::PrintBars(int nPlayer) {
    std::list<BarData> &bars = mTracks[mGeom->GetPlayer(nPlayer)->mTrack].mBars;
    for (auto it = bars.begin(); it != bars.end(); ++it) {
        DebugPrint("--bar from %f to %f (bar %d): enabled? %c  owner: %d\n",
                   static_cast<double>(it->mTick),
                   static_cast<double>(it->mTick + it->mTicks),
                   static_cast<int>(it->mTick) / kTicksPerBar,
                   (it->mInSong != 0) ? 't' : 'f',
                   it->mOwner);
    }
    DebugPrint("\n");
}

void GfxTunnel::ReportParticles() {
    mArms->Report();
}

void GfxTunnel::SetEnergy(float flEnergy, [[maybe_unused]] int nZone) {
    mDivider->mConditionFrame = flEnergy * kConditionFrames;
}

void GfxTunnel::SetStreakMultiplier(int nPlayer,
                                    int nMultiplier,
                                    [[maybe_unused]] int nNextMultiplier,
                                    [[maybe_unused]] const char *pszMultiplier,
                                    const char *pszNextMultiplier,
                                    [[maybe_unused]] bool bBoosted) {
    // Yes, the binary discards whether the colour changed.
    (void)GetPlayer(nPlayer)->SelectColor(nMultiplier);
    if (TheGameDb->IsLocalPlayer(nPlayer) && !mStreakGems.empty()) {
        mStreakGems.front()->SetText(pszNextMultiplier);
    }
}

void GfxTunnel::SetMultiplierEndTick(int nPlayer, float flTick) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    if (pPlayer != nullptr) {
        pPlayer->mCursor.StartMultiplier(mGems, flTick);
    }
}

void GfxTunnel::ShowSlowdown([[maybe_unused]] int nPlayer,
                             float flStartTick,
                             float flEndTick,
                             float flStopTick) {
    mSlowdown->Start(flStartTick, flEndTick, flStopTick);
    mBlurs->Insert("slowdown_end", flEndTick, flStopTick);
    mBlurs->Insert("slowdown_start", TheGameDb->mSongTick, flStartTick);
}

void GfxTunnel::ShowCripple(int nAttacker, int nVictim) {
    auto it = mCrippleFX.begin();
    while (it != mCrippleFX.end() && (*it)->mState != TnlCripFX::kStateIdle) {
        ++it;
    }
    if (it == mCrippleFX.end()) {
        // Yes, the binary takes the last crippler without testing the vector for emptiness.
        --it;
    }
    (*it)->Start(this, static_cast<char>(nAttacker), static_cast<char>(nVictim));
}

void GfxTunnel::ShowBump(int nAttacker, int nVictim, int nTrack) {
    TnlPlayer *pAttacker = GetPlayer(nAttacker);
    TnlPlayer *pVictim = GetPlayer(nVictim);
    if (pAttacker != nullptr) {
        pAttacker->mActivator.Bump(false, nTrack, nullptr);
    }
    if (pVictim != nullptr) {
        pVictim->mActivator.Bump(
            true, nTrack, (pAttacker != nullptr) ? &pAttacker->mActivator : nullptr);
    }
}

void GfxTunnel::SetPlayerTrack(int nPlayer,
                               int nTrack,
                               [[maybe_unused]] int nTrackType,
                               [[maybe_unused]] int nInstrument) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    (void)mGeom->GetPlayer(pPlayer->mPlayer); // Yes, the binary discards this call's result.
    pPlayer->mFreestyle.SetActive(false, 0, false);
    mGeom->GetPlayer(pPlayer->mPlayer)->SetTrack(static_cast<char>(nTrack));
    pPlayer->mGrid.SetTrack(nTrack);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix && TheGameDb->IsLocalPlayer(nPlayer)) {
        mGems->mFocusTrack = static_cast<char>(nTrack);
    } else if (mDefaultFocusTrack >= 0) {
        mGems->mFocusTrack = -1; // Yes, the binary clears the focus track when a default is set.
    }
}

void GfxTunnel::SetPlayerFreestyle(int nPlayer, bool bVictory, bool bQuick) {
    GetPlayer(nPlayer)->mFreestyle.SetActive(true, bVictory, bQuick);
    mGeom->GetPlayer(nPlayer)->LeaveTrack();
    if (mDefaultFocusTrack >= 0) {
        mGems->mFocusTrack = mDefaultFocusTrack;
    }
}

int GfxTunnel::GetViewedTrack(int nPlayer) {
    return GetPlayer(nPlayer)->mFreestyle.GetTrack();
}

void GfxTunnel::SetFreestylePosition(int nPlayer, const Vector3 &stick) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    if (pPlayer != nullptr) {
        pPlayer->mFreestyle.SetStick(stick);
    }
}

void GfxTunnel::ShowScratchNote(int nPlayer, float flTick, float flDuration) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    if (pPlayer != nullptr) {
        pPlayer->mFreestyle.Scratch(flTick, flDuration);
    }
}

void GfxTunnel::SetTrackPlayers(int nTrack, int nPlayers, const int *pPlayers) {
    std::vector<signed char> &players = mTracks[nTrack].mPlayers;
    players.resize(nPlayers, 0);
    for (int i = 0; i < nPlayers; ++i) {
        players[i] = static_cast<signed char>(pPlayers[i]);
    }
    mTrackPlayersDirty = 1;
}

void GfxTunnel::PlaceGem(
    int nTrack, int nSlot, int nPlayer, float flTick, int bSlot, int nPowerup) {
    const float flNow = TheGameDb->mSongTick;
    if (kLateGemTicks < flNow - flTick) {
        return;
    }
    char nColor = kOwnColor;
    float flShowTick = flNow;
    int bFlash = 0;
    bool bTellCursors = false;
    signed char nType;
    if (bSlot != 0) {
        nType = mSlotGemGroup;
    } else if (nPlayer >= 0) {
        const int nRuleSet = TheGameDb->mRuleSet;
        if (nRuleSet == GameDb::kRuleSetDuel ||
            (nRuleSet == GameDb::kRuleSetRemix && GetTrackType(nTrack) == Song::kTrackTypePitch)) {
            nType = mPlayerGemGroups[nPlayer];
            bTellCursors = true;
        } else {
            nType = mTrackGemGroups[nTrack];
            if (TheGameDb->mRuleSet == GameDb::kRuleSetGame &&
                TheGameDb->mCommunity != GameDb::kCommunitySolo) {
                nColor = static_cast<char>(TheGameDb->GetPlayerSlot(nPlayer));
            }
            bFlash = 1;
            if (mCaptureReveals != 0) {
                TnlTrackFX *pTrackFX = mTrackFX[nTrack];
                if (pTrackFX->mState == TnlTrackFX::kStateCapture) {
                    flShowTick = pTrackFX->CaptureEndTime() / *TheGfxManager.mMsPerTick;
                } else {
                    flShowTick = flNow + ((flTick - flNow) / TnlFire::sCaptureSpeed);
                }
            }
        }
    } else if (nPowerup == 0) {
        nType = mGemGroup;
        bTellCursors = true;
    } else {
        const float flLateral =
            (static_cast<float>(nSlot) * TnlGem::kSlotWidth) + TnlGem::kSlotOffset;
        nType = mPowerupGemGroups[nPowerup - 1];
        mSpotLights->AddLight(static_cast<char>(nTrack), mGeom, flTick, flLateral);
        bTellCursors = true;
    }
    mGems->AddGem(static_cast<char>(nTrack),
                  nType,
                  static_cast<char>(nSlot),
                  bFlash,
                  nColor,
                  flTick,
                  flShowTick);
    if (bTellCursors) {
        for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
            (*it)->mCursor.OnGemAdded(static_cast<char>(nTrack), nType, mGems, flTick);
        }
    }
}

void GfxTunnel::RemoveGem(int nTrack, int nSlot, float flTick) {
    mGems->RemoveGem(static_cast<char>(nTrack), static_cast<char>(nSlot), flTick);
    if (kCursorLeadTicks < flTick - TheGameDb->mSongTick) {
        for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
            (*it)->mCursor.OnGemAdded(static_cast<char>(nTrack), -1, mGems, flTick);
        }
    }
}

void GfxTunnel::ClearGems(int nTrack, bool bErase, float flStartTick, float flEndTick) {
    const float flTime = TheGameDb->mSongTime;
    const float flNow = TheGameDb->mSongTick;
    if (bErase) {
        for (auto it = mGems->Find(flStartTick); it != mGems->mGems.end() && it->mTick < flEndTick;
             ++it) {
            if (it->mTrack != nTrack) {
                continue;
            }
            const float flEraseTime = flTime + ((it->mTick - flNow) * kEraseMsPerTick);
            const float flRemoveTick = flEraseTime / *TheGfxManager.mMsPerTick;
            if ((it->mType & TnlGem::kTypeSprite) == 0) {
                int nGemPlayer = static_cast<int>(mPlayerGemGroups.size()) - 1;
                while (nGemPlayer >= 0 &&
                       mPlayerGemGroups[nGemPlayer] != static_cast<signed char>(it->mType)) {
                    --nGemPlayer;
                }
                mGemErase->Add(static_cast<char>(nTrack),
                               static_cast<char>(nGemPlayer),
                               flEraseTime,
                               it->mTick,
                               it->Lateral());
            }
            it->mRemoveTick = flRemoveTick;
        }
    } else {
        mGems->RemoveGems(static_cast<char>(nTrack), flStartTick, flEndTick);
    }
    if (kCursorLeadTicks < flEndTick - flNow) {
        for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
            (*it)->mCursor.OnTicksChanged(static_cast<char>(nTrack), mGems, flStartTick, flEndTick);
        }
    }
}

void GfxTunnel::HideNextPhrase(bool bNow) {
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        (*it)->Hide(bNow);
    }
}

void GfxTunnel::ShowNextPhrase(int nTrack, float flTick, int nSlot) {
    if (!mStreakGems.empty()) {
        mStreakGems[nTrack]->Show(flTick, nSlot);
    }
}

void GfxTunnel::SetCatchMeter(int nPlayer, float flLevel) {
    GetPlayer(nPlayer)->mCursor.SetEnergy(flLevel);
}

void GfxTunnel::ShowGemResult(
    int nTrack, int nSlot, bool bHit, int nPlayer, int nBurst, float flTick) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    pPlayer->mActivator.Hit(nSlot, bHit);
    if (bHit) {
        const float flLateral =
            (static_cast<float>(nSlot) * TnlGem::kSlotWidth) + TnlGem::kSlotOffset;
        Transform xfm;
        mGeom->CellXfm(nTrack, &xfm, false, true, flTick, flLateral);
        (void)Flash(&xfm.mTranslation, sGemHitFlashSize, sGemHitFlashGrowth);
        pPlayer->mCursor.Pulse(flTick);
        const std::vector<RndView *> *pViews;
        if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
            // The bursts of a duel take the colour of the other player.
            pViews = &mPlayerBurstViews[nPlayer ^ 1];
        } else {
            pViews = &mGemBurstViews[nBurst];
        }
        // Yes, the binary discards whether a burst effect was free.
        (void)Burst(pViews, static_cast<char>(nTrack), false, flTick, flLateral);
    } else if (sShowMissGems != 0 && TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        mGems->AddGem(static_cast<char>(nTrack),
                      mMissGemGroup,
                      static_cast<char>(nSlot),
                      0,
                      kOwnColor,
                      flTick,
                      kMissShowTick);
    }
}

void GfxTunnel::HitGem(int nPlayer, int nTrack, int nSlot, float flTick) {
    (void)GetPlayer(nPlayer); // Yes, the binary discards this call's result.
    Transform xfm;
    mGeom->CellXfm(nTrack,
                   &xfm,
                   false,
                   true,
                   flTick,
                   (static_cast<float>(nSlot) * TnlGem::kSlotWidth) + TnlGem::kSlotOffset);
    (void)Flash(&xfm.mTranslation, sGemPitchFlashSize, sGemPitchFlashGrowth);
}

void GfxTunnel::SetFreestyle(int nPlayer, bool bPlaying, int nColumn) {
    GetPlayer(nPlayer)->mFreestyle.SetPlaying(bPlaying, nColumn);
    if (TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        mGems->mHasFocusTrack = bPlaying;
    }
}

void GfxTunnel::ResetFreestyle(int nPlayer) {
    GetPlayer(nPlayer)->mFreestyle.Sample();
}

void GfxTunnel::ShowCapture(
    int nPlayer, int nTrack, int bAuto, bool bFreestyle, float flStartTick, float flEndTick) {
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        return;
    }
    TrackData &track = mTracks[nTrack];
    track.mCaptureStartTick = flStartTick;
    track.mCaptureEndTick = flEndTick;
    const float flNow = TheGameDb->mSongTick;
    if (bAuto != 0) {
        mTrackFX[nTrack]->Capture(this, flEndTick);
        return;
    }
    const Color *pColor;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pColor = TheGfxManager.GetInstrumentColor(GetTrackInstrument(nTrack));
    } else {
        pColor = TheGfxManager.GetPlayerColor(nPlayer);
    }
    StartFire(bFreestyle ? kFreestyleFirePlayer : TnlFire::kChunkPlayer,
              nTrack,
              0,
              flNow,
              flNow,
              flEndTick,
              pColor,
              pColor,
              nullptr);
}

void GfxTunnel::ShowResult([[maybe_unused]] bool bWon,
                           [[maybe_unused]] int nPlayer,
                           bool bCampaignWin,
                           bool bUnlocked) {
    const float flNow = TheGameDb->mSongTick;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        StartFade(true);
        TheGfxManager.ShowHud(false);
        mIdle = (flNow != 0.0f) ? 1 : 0; // Yes, a duel goes idle at once unless the tick is 0.
        return;
    }
    float flDoneTicks = 0.0f;
    if (bUnlocked) {
        flDoneTicks = sWinSequenceDuration + mUnlockDoneTicks;
    } else if (bCampaignWin) {
        flDoneTicks = sWinSequenceDuration;
    }
    mResultDoneTick = flNow + flDoneTicks;
    if (bCampaignWin) {
        mWinShell->Start(bUnlocked ? mArena->mUnlockIndex : TnlWinShell::kNoPath);
    } else {
        TheGfxManager.ShowHud(false);
    }
}

void GfxTunnel::StartBossJourney() {
    mBossJourneyDoneTick = TheGameDb->mSongTick + mBossDoneTicks;
}

void GfxTunnel::ShowPhrase(int nPlayer,
                           int nTrack,
                           bool bPlayable,
                           float flStartTick,
                           float flEndTick,
                           int nStyle,
                           [[maybe_unused]] bool bSlide) {
    TnlPlayer *pPlayer = GetPlayer(nPlayer);
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo &&
        TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        mPanelColorsDirty = 1;
    }
    if (bPlayable && sShowSeeker != 0) {
        pPlayer->mCursor.SetTarget(static_cast<char>(nTrack), mGems, flStartTick, flEndTick);
    } else {
        pPlayer->mCursor.Refresh(nStyle);
    }
}

void GfxTunnel::SetBar(int nTrack,
                       int nPlayer,
                       int nOwner,
                       bool bInSong,
                       bool bNow,
                       int nFlags,
                       signed char nInstrument,
                       float flTick,
                       float flTicks) {
    TrackData &track = mTracks[nTrack];
    int bFound;
    auto bar = track.FindBar(flTick, TheGameDb->mSongTick, &bFound);
    if (kLeadInTick <= flTick && flTick + flTicks <= 0.0f) {
        nFlags |= kBarFlagBeforeSong;
    }
    if (bFound != 0) {
        bar->mOwner = nOwner;
        bar->mInSong = bInSong;
        bar->mFlags = nFlags;
    } else {
        bar = track.mBars.insert(bar, BarData{flTick, flTicks, bInSong, nOwner, nFlags});
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix || (!bInSong && kTwoBarTicks < flTick)) {
        const float flSinceSection = mDivider->TicksSinceSection(flTick);
        if (flSinceSection < kBarTicks) {
            const float flSongTicks = mArena->mSongTicks;
            if (flSongTicks != 0.0f && flSongTicks <= flTick) {
                bar->mFlags |= kBarFlagNoReveal | kBarFlagCheckpoint | kBarFlagFinish;
            } else {
                bar->mFlags |= kBarFlagNoReveal | kBarFlagCheckpoint;
            }
        } else if (flSinceSection < kTwoBarTicks) {
            bar->mFlags |= kBarFlagNoReveal;
        }
    }
    const Color *pColor = (nInstrument >= 0) ? &mInstrumentColors[nInstrument] : nullptr;
    ApplyBar(&track, &*bar, nPlayer, bNow, pColor);
}

void GfxTunnel::ApplyBar(TrackData *pTrack,
                         const BarData *pBar,
                         [[maybe_unused]] int nPlayer,
                         bool bNow,
                         const Color *pColor) {
    bool bReveal = false;
    if (mCaptureReveals != 0) {
        bReveal = !bNow;
    }
    const float flNow = TheGameDb->mSongTick;
    int bWaitForTrack = 0;
    if (flNow < 0.0f) {
        bReveal = false;
    }
    if (kBarKeepTicks < flNow - (pBar->mTick + pBar->mTicks)) {
        return;
    }
    float flRevealTick;
    if (pBar->mOwner < 0) {
        flRevealTick = 0.0f;
        bReveal = false;
    } else if (mTrackFX[pTrack->mIndex]->mState == TnlTrackFX::kStateCapture) {
        flRevealTick = flNow;
        bWaitForTrack = 1;
    } else {
        flRevealTick = flNow + ((pBar->mTick - flNow) / TnlFire::sCaptureSpeed);
    }
    if (bReveal) {
        AddPanel(
            new GfxTunnelPanel(this, pTrack->mIndex, pBar, pColor), bWaitForTrack, flRevealTick);
    } else {
        GfxTunnelPanel panel(this, pTrack->mIndex, pBar, pColor);
        panel.Apply();
    }
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mGrid.RefreshTrack(pTrack->mIndex, false);
    }
}

void GfxTunnel::DimBar(int nTrack, float flTick) {
    const int nBar = static_cast<int>(floorf(flTick * mGeom->mBarsPerTick));
    const Color *pColor = mGeom->GetPanelColor(nTrack, nBar);
    if (pColor != nullptr) {
        const Color dimmed{pColor->r * sDuelFailDarkening,
                           pColor->g * sDuelFailDarkening,
                           pColor->b * sDuelFailDarkening,
                           pColor->a * sDuelFailDarkening};
        mGeom->SetPanelColor(nTrack, nBar, &dimmed, true);
    }
}

void GfxTunnel::SetTrackEnabled(int nTrack, bool bEnabled) {
    if (bEnabled) {
        mTrackFX[nTrack]->Enable(this);
    }
}

void GfxTunnel::AddCheckpoint(const char *pszLabel, float flTick, float flTextScale) {
    mDivider->AddSection(flTick, pszLabel, flTextScale);
}

void GfxTunnel::ClearCheckpoints() {
    mDivider->ClearSections();
}

void GfxTunnel::SetOption(bool bOption) {
    mCamFX->SetZoomedOut(bOption);
}

int GfxTunnel::GetOption() {
    return mCamFX->mZoomedOut;
}

void GfxTunnel::SetZoom(float flZoom) {
    mCamFX->SetZoom(flZoom);
}

void GfxTunnel::BlendToArenaPath(bool bArena) {
    const float flTime = TheGameDb->mSongTime;
    mPathBlendCurve.Reset(mPathBlendCurve.Eval(flTime),
                          bArena ? kArenaBlendOn : kArenaBlendOff,
                          flTime,
                          flTime + kArenaBlendMs);
    mCamFX->SetBlendAlgorithm(kArenaBlendAlgorithm, mPathBlendCurve.mY0);
    mPathBlending = 1;
}

void GfxTunnel::SetCatcherShown(int nPlayer, bool bShown) {
    GetPlayer(nPlayer)->mActivator.mCatcher.SetShown(bShown);
}

void GfxTunnel::ClearLanes() {
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mActivator.Reset();
    }
    mGeom->ResetPlayers();
}

void GfxTunnel::ClearAll() {
    mCamFX->Reset(false);
    mDivider->Reset();
    mGems->Clear();
    mFadeCurve.Reset(0.0f, 0.0f, 0.0f, 1.0f);
    mPathBlendCurve.Reset(0.0f, 0.0f, 0.0f, 1.0f);
    mPathBlending = 0;
    mBossJourneyDoneTick = kNeverTick;
    mResultDoneTick = kNeverTick;
    mBossJourneyDone = 0;
    mIdle = 0;
    for (auto it = mStreakGems.begin(); it != mStreakGems.end(); ++it) {
        (*it)->Hide(true);
    }
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        (*it)->mFreestyle.Reset();
        (*it)->mCursor.Reset();
        (*it)->mActivator.SetSuppressed(0);
    }
    for (auto it = mTracks.begin(); it != mTracks.end(); ++it) {
        it->mCaptureEndTick = TrackData::kNoTick;
        it->mCaptureStartTick = TrackData::kNoTick;
        it->mBars.clear();
    }
    mGeom->SetPath(mArena->mCamPath);
    mArms->LoadConfig(GetGfxConfig()->FindArray("particle_arms", true));
    for (auto it = mTrackFX.begin(); it != mTrackFX.end(); ++it) {
        (*it)->Stop(this);
    }
    auto panel = mPanels.begin();
    while (panel != mPanels.end()) {
        (*panel)->Refresh();
        delete *panel;
        panel = mPanels.erase(panel);
    }
    for (auto it = mGemCaps.begin(); it != mGemCaps.end(); ++it) {
        (*it)->Stop();
    }
    for (auto it = mCrippleFX.begin(); it != mCrippleFX.end(); ++it) {
        (*it)->Stop(this, true);
    }
    for (auto it = mFlashes.begin(); it != mFlashes.end(); ++it) {
        (*it)->Stop();
    }
    for (auto it = mFires.begin(); it != mFires.end(); ++it) {
        (*it)->Reset(true);
    }
    mSlowdown->Reset();
    mWinShell->Reset();
    mBlurs->Reset();
    mSpotLights->Clear();
    mGemErase->Clear();
    mGeom->ResetPanels();
    mGeom->Poll(TheGameDb->mSongTick, 0.0f);
    mPanelColorsDirty = 1;
    mTrackPlayersDirty = 1;
}
