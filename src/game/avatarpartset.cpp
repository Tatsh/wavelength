#include "game/avatarpartset.h"

#include <algorithm>
#include <cstring>

#include "game/avatar.h"
#include "game/torsopart.h"
#include "os/debug.h"
#include "os/system.h"
#include "rnd/rndrenderer.h"
#include "script/symbol.h"

namespace {

// The first node of a `types` entry is its name. The choices start after the tag of the array.
constexpr int kTypeName = 0;
constexpr int kFirstType = 1;

// The first node of a part of a description is the part.
constexpr int kPartKey = 0;

// The colour a description gives a part without a `color`.
constexpr float kDefaultComponent = 0.5f;
constexpr float kOpaque = 1.0f;

// The byte Clear() greys each colour component with.
constexpr unsigned char kClearedComponent = 127;

// The largest colour byte.
constexpr float kMaxByte = 255.0f;
constexpr double kMaxByteDouble = 255.0;

// The components of a colour, in the order the bytes are stored.
constexpr int kRed = 0;
constexpr int kGreen = 1;
constexpr int kBlue = 2;

// A count above the differences of any avatar, where AcquirePlayer() starts its search.
constexpr int kMoreDifferences = 666;

// The fewest parts AcquirePlayer() leaves pending.
constexpr int kOnePending = 1;

// The prefix of the `win` animations.
const char *const kWinAnimPrefix = "win";
constexpr int kWinAnimPrefixLength = 3;

// The time Poll() starts from.
constexpr float kNoTime = -99999.0f;

// Write one byte of a set.
inline void WriteByte(BinStream &stream, unsigned char nByte) {
    stream.Write(&nByte, sizeof(nByte));
}

DataArray *PartTypes(int nPart) {
    return AvatarPartSet::GetParts()->FindArray(nPart)->FindArray("types", true);
}

DataArray *EmblemTypes() {
    return AvatarPartSet::GetParts()
        ->FindArray(AvatarPartSet::kPartTorso)
        ->FindArray("emblem", true)
        ->FindArray("types", true);
}

// Fill a list with the names of the choices of a `types` array.
void ListTypes(DataArray *pTypes, std::vector<const char *> *pList) {
    const int nTypes = pTypes->Size();
    pList->resize(nTypes - kFirstType, nullptr);
    for (int i = kFirstType; i < nTypes; ++i) {
        (*pList)[i - kFirstType] = pTypes->Array(i)->Sym(kTypeName);
    }
}

} // namespace

DataArray *AvatarPartSet::sParts = nullptr;
float AvatarPartSet::sLastTime = kNoTime;

void AvatarPartSet::Init() {
    Avatar::Init();
    sParts =
        SystemConfig()->FindArray("db", true)->FindArray("avatar", true)->FindArray("parts", true);
}

void AvatarPartSet::Terminate() {
    Avatar::Terminate();
}

void AvatarPartSet::GetPartTypes(int nPart, std::vector<const char *> *pTypes) {
    ListTypes(PartTypes(nPart), pTypes);
}

void AvatarPartSet::GetEmblemTypes(std::vector<const char *> *pTypes) {
    ListTypes(EmblemTypes(), pTypes);
}

AvatarPartSet::AvatarPartSet() {
    mPlayer = nullptr;
    mBaseAnim = nullptr;
    mBaseAnimFlags = 0;
    mPoseAnim = nullptr;
    mVersion = 1;
    Load(SystemConfig()
             ->FindArray("db", true)
             ->FindArray("avatar", true)
             ->FindArray("default_settings", true));
}

AvatarPartSet::~AvatarPartSet() {
    ReleasePlayer();
}

AvatarPartSet &AvatarPartSet::operator=(const AvatarPartSet &other) {
    if (this == &other) {
        return *this;
    }
    mVersion = other.mVersion;
    mEmblem = other.mEmblem;
    for (int i = 0; i < kNumParts; ++i) {
        mParts[i].SetType(other.mParts[i].GetType());
        for (int j = kRed; j <= kBlue; ++j) {
            mParts[i].SetColor(j, other.mParts[i].GetColor(j));
        }
    }
    ReleasePlayer();
    return *this;
}

void AvatarPartSet::Load(DataArray *pData) {
    mVersion = 1;
    for (int i = 1; i < pData->Size(); ++i) {
        DataArray *pPart = pData->Array(i);
        const int nPart = pPart->Int(kPartKey);
        const char *pszType;
        pPart->FindSymbol("type", &pszType, true);
        mParts[nPart].SetType(static_cast<unsigned char>(FindPartType(nPart, pszType)));

        Color color;
        color.a = kOpaque;
        color.r = kDefaultComponent;
        color.g = kDefaultComponent;
        color.b = kDefaultComponent;
        pPart->FindColor("color", &color, false);
        mParts[nPart].SetColor(kRed, ColorToByte(color.r));
        mParts[nPart].SetColor(kGreen, ColorToByte(color.g));
        mParts[nPart].SetColor(kBlue, ColorToByte(color.b));

        if (nPart == kPartTorso) {
            const char *pszEmblem;
            pPart->FindSymbol("emblem", &pszEmblem, true);
            mEmblem = static_cast<unsigned char>(FindEmblem(pszEmblem));
        }
    }
    ReleasePlayer();
}

void AvatarPartSet::SetPart(int nPart, const char *pszType) {
    const char *pszSymbol = LookupSymbol(pszType);
    mParts[nPart].SetType(static_cast<unsigned char>(FindPartType(nPart, pszSymbol)));
    if (nPart == kPartLeftArm) {
        mParts[kPartRightArm].SetType(
            static_cast<unsigned char>(FindPartType(kPartRightArm, pszSymbol)));
    }
    ReleasePlayer();
}

void AvatarPartSet::SetColor(int nPart, const Color *pColor) {
    mParts[nPart].SetColor(kRed, ColorToByte(pColor->r));
    mParts[nPart].SetColor(kGreen, ColorToByte(pColor->g));
    mParts[nPart].SetColor(kBlue, ColorToByte(pColor->b));
    if (nPart == kPartLeftArm) {
        for (int i = kRed; i <= kBlue; ++i) {
            mParts[kPartRightArm].SetColor(i, mParts[kPartLeftArm].GetColor(i));
        }
    }
    if (mPlayer != nullptr) {
        mPlayer->GetPart(nPart)->SetColor(pColor);
        if (nPart == kPartLeftArm) {
            mPlayer->GetPart(kPartRightArm)->SetColor(pColor);
        }
    }
}

void AvatarPartSet::SetEmblem(const char *pszEmblem) {
    mEmblem = static_cast<unsigned char>(FindEmblem(LookupSymbol(pszEmblem)));
    ReleasePlayer();
}

bool AvatarPartSet::IsIncomplete() const {
    for (int i = 0; i < kNumParts; ++i) {
        if (i == kPartHeadGear || i == kPartFaceGear || i == kPartInstrument) {
            continue;
        }
        if (mParts[i].GetType() == 0) {
            return true;
        }
    }
    return false;
}

void AvatarPartSet::Clear() {
    for (int i = 0; i < kNumParts; ++i) {
        if (i == kPartInstrument) {
            continue;
        }
        mParts[i].SetType(0);
        mParts[i].SetColor(kRed, kClearedComponent);
        mParts[i].SetColor(kGreen, kClearedComponent);
        mParts[i].SetColor(kBlue, kClearedComponent);
    }
    mEmblem = 0;
    ReleasePlayer();
}

unsigned char AvatarPartSet::ColorToByte(float fComponent) {
    return static_cast<unsigned char>(static_cast<int>(fComponent * kMaxByte));
}

float AvatarPartSet::ByteToColor(unsigned char nByte) {
    // The division is in double precision.
    return static_cast<float>(static_cast<double>(nByte) / kMaxByteDouble);
}

int AvatarPartSet::FindPartType(int nPart, const char *pszType) {
    DataArray *pTypes = PartTypes(nPart);
    for (int i = kFirstType; i < pTypes->Size(); ++i) {
        if (pTypes->Array(i)->Sym(kTypeName) == pszType) {
            return i - kFirstType;
        }
    }
    DebugWarn("Couldn't find type %s", pszType);
    return -1;
}

int AvatarPartSet::FindEmblem(const char *pszEmblem) {
    DataArray *pTypes = EmblemTypes();
    for (int i = kFirstType; i < pTypes->Size(); ++i) {
        if (pTypes->Array(i)->Sym(kTypeName) == pszEmblem) {
            return i - kFirstType;
        }
    }
    DebugWarn("Couldn't find emblem %s", pszEmblem);
    return -1;
}

const char *AvatarPartSet::EmblemName() const {
    return EmblemTypes()->Array(mEmblem + kFirstType)->Sym(kTypeName);
}

const char *AvatarPartSet::PartName(int nPart) const {
    return PartTypes(nPart)->Array(mParts[nPart].GetType() + kFirstType)->Sym(kTypeName);
}

Color AvatarPartSet::PartColor(int nPart) const {
    Color color;
    color.r = ByteToColor(mParts[nPart].GetColor(kRed));
    color.g = ByteToColor(mParts[nPart].GetColor(kGreen));
    color.b = ByteToColor(mParts[nPart].GetColor(kBlue));
    color.a = kOpaque;
    return color;
}

void AvatarPartSet::Poll(float fTime) {
    if (fTime != sLastTime) {
        sLastTime = fTime;
        Avatar::Poll(fTime);
    }
    const int nStaleFrame = TheRnd->GetFrameCount() - 1;
    for (int i = 0; i < Avatar::NumAvatars(); ++i) {
        AvatarPartSet *pUser = Avatar::GetAvatar(i)->GetUser();
        if (pUser != nullptr && pUser->mFrame < nStaleFrame) {
            pUser->ReleasePlayer();
        }
    }
}

int AvatarPartSet::CountDifferences(Avatar *pAvatar) const {
    int nDifferences = 0;
    for (int i = 0; i < kPartInstrument; ++i) {
        if (i == kPartRightArm) {
            continue;
        }
        const char *pszType = pAvatar->GetPart(i)->GetType();
        if (pszType == nullptr || FindPartType(i, pszType) != mParts[i].GetType()) {
            ++nDifferences;
        }
    }
    const char *pszEmblem = static_cast<TorsoPart *>(pAvatar->GetPart(kPartTorso))->GetEmblem();
    if (pszEmblem == nullptr || FindEmblem(pszEmblem) != mEmblem) {
        ++nDifferences;
    }
    return nDifferences;
}

void AvatarPartSet::ReleasePlayer() {
    if (mPlayer == nullptr) {
        return;
    }
    if (mPending != 0) {
        for (int i = 0; i < kNumParts; ++i) {
            mPlayer->GetPart(i)->ReleaseType();
        }
    }
    mPending = 0;
    mPlayer->SetUser(nullptr);
    mPlayer = nullptr;
}

void AvatarPartSet::AcquirePlayer() {
    if (mPlayer != nullptr) {
        return;
    }
    int nFewest = kMoreDifferences;
    for (int i = 0; i < Avatar::NumAvatars(); ++i) {
        Avatar *pAvatar = Avatar::GetAvatar(i);
        if (pAvatar->GetUser() != nullptr) {
            continue;
        }
        const int nDifferences = CountDifferences(pAvatar);
        if (nDifferences < nFewest) {
            nFewest = nDifferences;
            mPlayer = pAvatar;
        }
    }
    mPlayer->SetUser(this);
    for (int i = 0; i < kNumParts; ++i) {
        mPlayer->GetPart(i)->SetType(PartName(i));
    }
    static_cast<TorsoPart *>(mPlayer->GetPart(kPartTorso))->SetEmblem(EmblemName());
    mPending = std::max(nFewest, kOnePending);
}

bool AvatarPartSet::UpdatePlayer() {
    if (mPending == 0) {
        return true;
    }
    for (int i = 0; i < kNumParts; ++i) {
        if (!mPlayer->GetPart(i)->Rebuild()) {
            return false;
        }
    }
    if (!static_cast<TorsoPart *>(mPlayer->GetPart(kPartTorso))->RebuildEmblem()) {
        return false;
    }
    for (int i = 0; i < kNumParts; ++i) {
        Color color;
        color.r = ByteToColor(mParts[i].GetColor(kRed));
        color.g = ByteToColor(mParts[i].GetColor(kGreen));
        color.b = ByteToColor(mParts[i].GetColor(kBlue));
        color.a = kOpaque;
        mPlayer->GetPart(i)->SetColor(&color);
    }
    mPlayer->Reset();
    if (mPoseAnim != nullptr) {
        mPlayer->SetPose(mPoseAnim, true);
    }
    if (mBaseAnim != nullptr) {
        mPlayer->SetBaseAnim(mBaseAnim, mBaseAnimFlags != 0, true);
    }
    mPending = 0;
    return true;
}

void AvatarPartSet::Render(const Rect *pScreenRect) {
    AcquirePlayer();
    mFrame = TheRnd->GetFrameCount();
    // An avatar one part away from the set is drawn while that part loads.
    if (UpdatePlayer() || mPending == kOnePending) {
        mPlayer->Draw(pScreenRect);
    } else {
        Avatar::DrawCamera();
    }
}

void AvatarPartSet::SetPose(const char *pszPose) {
    if (mPlayer != nullptr) {
        mPlayer->SetPose(pszPose, false);
    }
    mPoseAnim = LookupSymbol(pszPose);
}

void AvatarPartSet::SetBaseAnim(const char *pszAnim, int nFlags) {
    if (mPlayer != nullptr) {
        mPlayer->SetBaseAnim(pszAnim, nFlags != 0, false);
    }
    mBaseAnimFlags = nFlags;
    mBaseAnim = LookupSymbol(pszAnim);
}

void AvatarPartSet::CatchGem(int nLane, int nCatch) {
    if (mPlayer != nullptr) {
        mPlayer->CatchGem(nLane, nCatch);
    }
}

void AvatarPartSet::ClearAnims() {
    if (mPlayer != nullptr) {
        mPlayer->Reset();
    }
    mBaseAnim = nullptr;
    mPoseAnim = nullptr;
}

void AvatarPartSet::Save(BinStream &stream) const {
    WriteByte(stream, mVersion);
    WriteByte(stream, mEmblem);
    for (const PartSetting &part : mParts) {
        WriteByte(stream, part.GetType());
        WriteByte(stream, part.GetColor(kRed));
        WriteByte(stream, part.GetColor(kGreen));
        WriteByte(stream, part.GetColor(kBlue));
    }
}

void AvatarPartSet::Load(BinStream &stream) {
    stream.Read(&mVersion, sizeof(mVersion));
    stream.Read(&mEmblem, sizeof(mEmblem));
    // Both clamps let through the index one past the last choice, as in the binary.
    if (mEmblem >= EmblemTypes()->Size()) {
        mEmblem = 0;
    }
    for (int i = 0; i < kNumParts; ++i) {
        PartSetting &part = mParts[i];
        unsigned char nType = 0;
        stream.Read(&nType, sizeof(nType));
        part.SetType(nType);
        for (int j = kRed; j <= kBlue; ++j) {
            unsigned char nColor = 0;
            stream.Read(&nColor, sizeof(nColor));
            part.SetColor(j, nColor);
        }
        DataArray *pTypes = PartTypes(i);
        if (part.GetType() >= pTypes->Size()) {
            part.SetType(static_cast<unsigned char>(pTypes->Size() - 1));
        }
    }
}

bool AvatarPartSet::IsWinAnim() const {
    return mBaseAnim != nullptr &&
           std::strncmp(mBaseAnim, kWinAnimPrefix, kWinAnimPrefixLength) == 0;
}
