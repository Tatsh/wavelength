#include "met/metagameutil.h"

#include <string.h>

#include "game/avatarcam.h"
#include "os/locale.h"
#include "rnd/cam.h"
#include "rnd/manager.h"
#include "rnd/meshvert.h"

namespace {

// Values of the player numbers GetPlayerColorName() takes.
constexpr int kNoPlayer = -1;
constexpr int kPlayerGreen = 0;
constexpr int kPlayerPurple = 1;
constexpr int kPlayerRed = 2;
constexpr int kPlayerYellow = 3;

// Values of the connection kinds GetConnectionTypeName() takes.
constexpr int kConnectionModem = 1;
constexpr int kConnectionBroadband = 2;
constexpr int kConnectionOffline = 3;

// Values of the duel difficulties GetDuelDifficultyAbbrev() takes.
constexpr int kDuelEasy = 1;
constexpr int kDuelMedium = 2;
constexpr int kDuelHard = 3;

// The ranks FindRankMaterial() distinguishes.
constexpr int kRankFirst = -1;
constexpr int kRankSecond = 0;
constexpr int kRankThird = 1;
constexpr int kRankFourth = 2;
constexpr int kRankFifth = 3;

// The vertices of the opposite corners of the mesh DrawAvatarOnMesh() fills.
constexpr int kMeshCornerFirst = 0;
constexpr int kMeshCornerLast = 3;
constexpr int kNumMeshCorners = 2;

// The depth range an avatar drawn on a mesh renders into.
constexpr float kAvatarOnMeshNear = 0.0f;
constexpr float kAvatarOnMeshFar = 0.25f;

void TransformPoint(Vector3 &point, const float (&xfm)[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount]) {
    const float flX = point.x;
    const float flY = point.y;
    const float flZ = point.z;
    point.x = xfm[0][0] * flX + xfm[1][0] * flY + xfm[2][0] * flZ + xfm[3][0];
    point.y = xfm[0][1] * flX + xfm[1][1] * flY + xfm[2][1] * flZ + xfm[3][1];
    point.z = xfm[0][2] * flX + xfm[1][2] * flY + xfm[2][2] * flZ + xfm[3][2];
}

Rnd::Mat *FindMat(const char *pszName) {
    return dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszName));
}

} // namespace

Rnd::Mat *g_pWinnerMat;

void FindWinnerMaterial() {
    g_pWinnerMat = FindMat("freq_winner.mat");
}

Rnd::Font *FindColorFont(const char *pszFace, const char *pszColor) {
    const char *pszFormat;
    if (strcmp(pszColor, "red") == 0) {
        pszFormat = "%s_1_red.font";
    } else if (strcmp(pszColor, "green") == 0) {
        pszFormat = "%s_1_green.font";
    } else if (strcmp(pszColor, "purple") == 0) {
        pszFormat = "%s_1_pink.font";
    } else if (strcmp(pszColor, "yellow") == 0) {
        pszFormat = "%s_1_yellow.font";
    } else {
        pszFormat = "%s_1_white.font";
    }
    return dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find(FormatString(pszFormat, pszFace)));
}

Rnd::Font *FindLucidaFont(const char *pszColor) {
    return FindColorFont("lucida", pszColor);
}

Rnd::Mat *FindColorMaterial(const char *pszColor) {
    if (strcmp(pszColor, "red") == 0) {
        return FindMat("fcolor_2.mat");
    }
    if (strcmp(pszColor, "green") == 0) {
        return FindMat("fcolor_0.mat");
    }
    if (strcmp(pszColor, "purple") == 0) {
        return FindMat("fcolor_1.mat");
    }
    if (strcmp(pszColor, "yellow") == 0) {
        return FindMat("fcolor_3.mat");
    }
    return FindMat("fcolor_0.mat");
}

const char *GetPlayerColorName(int nPlayer) {
    switch (nPlayer) {
    case kPlayerGreen:
        return "green";
    case kPlayerPurple:
        return "purple";
    case kPlayerRed:
        return "red";
    case kPlayerYellow:
        return "yellow";
    case kNoPlayer:
    default:
        return "white";
    }
}

const char *GetConnectionTypeName(int nType) {
    const char *pszToken;
    switch (nType) {
    case kConnectionModem:
        pszToken = "modem";
        break;
    case kConnectionBroadband:
        pszToken = "broadband";
        break;
    case kConnectionOffline:
        pszToken = "offline";
        break;
    default:
        pszToken = "none";
        break;
    }
    return TheLocale.Localize(pszToken, true);
}

const char *GetConnectionTypeAbbrev(int nType) {
    const char *pszToken;
    switch (nType) {
    case kConnectionModem:
        pszToken = "modem_abbrev";
        break;
    case kConnectionBroadband:
        pszToken = "broadband_abbrev";
        break;
    case kConnectionOffline:
        pszToken = "offline_abbrev";
        break;
    default:
        pszToken = "none_abbrev";
        break;
    }
    return TheLocale.Localize(pszToken, true);
}

const char *GetDuelDifficultyAbbrev(int nDifficulty) {
    const char *pszToken;
    switch (nDifficulty) {
    case kDuelEasy:
        pszToken = "duel_easy_abbrev";
        break;
    case kDuelMedium:
        pszToken = "duel_medium_abbrev";
        break;
    case kDuelHard:
        pszToken = "duel_hard_abbrev";
        break;
    default:
        return "";
    }
    return TheLocale.Localize(pszToken, false);
}

Rnd::Mat *FindRankMaterial(int nRank) {
    switch (nRank) {
    case kRankSecond:
        return FindMat("rank_02.mat");
    case kRankThird:
        return FindMat("rank_03.mat");
    case kRankFourth:
        return FindMat("rank_04.mat");
    case kRankFifth:
        return FindMat("rank_05.mat");
    case kRankFirst:
    default:
        return FindMat("rank_01.mat");
    }
}

Rnd::Mat *FindGradeMaterial(int nGrade, bool bEnd) {
    if (bEnd) {
        return FindMat(FormatString("grade_end_0%d.mat", nGrade));
    }
    return FindMat(FormatString("grade_0%d.mat", nGrade));
}

void DrawAvatarOnMesh(AvatarPartSet *pAvatar, Rnd::Mesh *pMesh) {
    if (pAvatar->IsWinAnim()) {
        Rnd::Mat *pOldMat = pMesh->mMat;
        pMesh->SetMat(g_pWinnerMat);
        static_cast<Rnd::Drawable *>(pMesh)->Draw();
        pMesh->SetMat(pOldMat);
    }

    const std::vector<Rnd::MeshVert> &verts = pMesh->mVertsOwner->mVerts;
    Vector3 corners[] = {verts[kMeshCornerFirst].mPoint, verts[kMeshCornerLast].mPoint};
    Vector2 ptScreen[kNumMeshCorners];
    for (int i = 0; i < kNumMeshCorners; ++i) {
        TransformPoint(corners[i], pMesh->mWorldXfm);
    }
    for (int i = 0; i < kNumMeshCorners; ++i) {
        Rnd::Cam::sCurrent->WorldToScreen(corners[i], ptScreen[i]);
    }
    const Rnd::Cam::Rect rect{
        ptScreen[0].x, ptScreen[0].y, ptScreen[1].x - ptScreen[0].x, ptScreen[1].y - ptScreen[0].y};

    float fNear;
    float fFar;
    GetAvatarDepthRange(&fNear, &fFar);
    SetAvatarDepthRange(kAvatarOnMeshNear, kAvatarOnMeshFar);
    pAvatar->Render(&rect);
    SetAvatarDepthRange(fNear, fFar);
}

const char *FormatGenreTempo(const SongEntry &song, const RemixInfo *pInfo) {
    const char *pszGenre = song.GetGenreName();
    const float fTempo = pInfo->mTempo;
    return FormatString(TheLocale.Localize("GENRE_BPM", true), pszGenre, static_cast<int>(fTempo));
}

bool TrimSpaces(String *pText) {
    if (pText->mLength == 0) {
        return false;
    }

    bool bTrimmed = false;
    int nLeading = 0;
    while (static_cast<unsigned>(nLeading) < static_cast<unsigned>(pText->mLength) &&
           (*pText)[nLeading] == ' ') {
        ++nLeading;
    }
    if (nLeading != 0) {
        pText->Erase(0, nLeading);
        bTrimmed = true;
    }

    int nLast = pText->mLength - 1;
    while (nLast > 0 && (*pText)[nLast] == ' ') {
        --nLast;
    }
    if (static_cast<unsigned>(nLast) < static_cast<unsigned>(pText->mLength - 1)) {
        // The count is one more than the characters after nLast, as in the binary.
        pText->Erase(nLast + 1, pText->mLength - nLast);
        bTrimmed = true;
    }
    return bTrimmed;
}
