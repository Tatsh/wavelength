#include "game/avatarpartset.h"

#include "game/avatarplayer.h"
#include "os/debug.h"
#include "os/system.h"
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

DataArray *PartTypes(int nPart) {
    return AvatarPartSet::sParts->FindArray(nPart)->FindArray("types", true);
}

DataArray *EmblemTypes() {
    return AvatarPartSet::sParts->FindArray(AvatarPartSet::kPartTorso)
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

void AvatarPartSet::Init() {
    AvatarPlayer::Init();
    sParts =
        SystemConfig()->FindArray("db", true)->FindArray("avatar", true)->FindArray("parts", true);
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
        mParts[i].mType = other.mParts[i].mType;
        for (int j = kRed; j <= kBlue; ++j) {
            mParts[i].mColor[j] = other.mParts[i].mColor[j];
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
        mParts[nPart].mType = static_cast<unsigned char>(FindPartType(nPart, pszType));

        Color color;
        color.a = kOpaque;
        color.r = kDefaultComponent;
        color.g = kDefaultComponent;
        color.b = kDefaultComponent;
        pPart->FindColor("color", &color, false);
        mParts[nPart].mColor[kRed] = ColorToByte(color.r);
        mParts[nPart].mColor[kGreen] = ColorToByte(color.g);
        mParts[nPart].mColor[kBlue] = ColorToByte(color.b);

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
    mParts[nPart].mType = static_cast<unsigned char>(FindPartType(nPart, pszSymbol));
    if (nPart == kPartLeftArm) {
        mParts[kPartRightArm].mType =
            static_cast<unsigned char>(FindPartType(kPartRightArm, pszSymbol));
    }
    ReleasePlayer();
}

void AvatarPartSet::SetColor(int nPart, const Color *pColor) {
    mParts[nPart].mColor[kRed] = ColorToByte(pColor->r);
    mParts[nPart].mColor[kGreen] = ColorToByte(pColor->g);
    mParts[nPart].mColor[kBlue] = ColorToByte(pColor->b);
    if (nPart == kPartLeftArm) {
        for (int i = kRed; i <= kBlue; ++i) {
            mParts[kPartRightArm].mColor[i] = mParts[kPartLeftArm].mColor[i];
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
        if (mParts[i].mType == 0) {
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
        mParts[i].mType = 0;
        mParts[i].mColor[kRed] = kClearedComponent;
        mParts[i].mColor[kGreen] = kClearedComponent;
        mParts[i].mColor[kBlue] = kClearedComponent;
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
    DebugWarn("Could not find type %s", pszType);
    return -1;
}

int AvatarPartSet::FindEmblem(const char *pszEmblem) {
    DataArray *pTypes = EmblemTypes();
    for (int i = kFirstType; i < pTypes->Size(); ++i) {
        if (pTypes->Array(i)->Sym(kTypeName) == pszEmblem) {
            return i - kFirstType;
        }
    }
    DebugWarn("Could not find emblem %s", pszEmblem);
    return -1;
}

const char *AvatarPartSet::EmblemName() const {
    return EmblemTypes()->Array(mEmblem + kFirstType)->Sym(kTypeName);
}

const char *AvatarPartSet::PartName(int nPart) const {
    return PartTypes(nPart)->Array(mParts[nPart].mType + kFirstType)->Sym(kTypeName);
}

Color AvatarPartSet::PartColor(int nPart) const {
    Color color;
    color.r = ByteToColor(mParts[nPart].mColor[kRed]);
    color.g = ByteToColor(mParts[nPart].mColor[kGreen]);
    color.b = ByteToColor(mParts[nPart].mColor[kBlue]);
    color.a = kOpaque;
    return color;
}
