#include "netflow/netgameparams.h"

#include "game/gamedb.h"

namespace {

// The power-up levels.
constexpr int kPowerupLow = 0;
constexpr int kPowerupHigh = 1;
constexpr int kPowerupOff = 2;

// The settings the stream writes as signed bytes, in order.
enum Setting {
    kSettingSkillLevel,
    kSettingMaxPlayers,
    kSettingRuleSet,
    kSettingCommunity,
    kSettingPowerupLevel,
    kSettingCount,
};

// Write one setting of a game as a byte.
inline void WriteByte(BinStream &stream, int nValue) {
    const signed char cValue = static_cast<signed char>(nValue);
    stream.Write(&cValue, sizeof(cValue));
}

} // namespace

PrnStream &operator<<(PrnStream &stream, const NetGameParams &params) {
    stream.Print(params.mSong.c_str());
    if (params.mRuleSet == GameDb::kRuleSetRemix || params.mLoadRemix != 0) {
        (stream << " ").Print(params.mRemixName.c_str());
    }
    stream << " sk:" << params.mSkillLevel << " max:" << params.mMaxPlayers;
    switch (params.mRuleSet) {
    case GameDb::kRuleSetGame:
        stream << " game";
        break;
    case GameDb::kRuleSetRemix:
        stream << " remix";
        break;
    case GameDb::kRuleSetDuel:
        stream << " duel";
        break;
    default:
        break;
    }
    switch (params.mCommunity) {
    case GameDb::kCommunitySolo:
        stream << " solo";
        break;
    case GameDb::kCommunityLocal:
        stream << " multi";
        break;
    case GameDb::kCommunityOnline:
        stream << " net";
        break;
    default:
        break;
    }
    stream << " pu:";
    switch (params.mPowerupLevel) {
    case kPowerupHigh:
        stream << " high";
        break;
    case kPowerupLow:
        stream << " low";
        break;
    case kPowerupOff:
        stream << " off";
        break;
    default:
        break;
    }
    if (params.mLoadRemix != 0) {
        stream << " remix-data";
    }
    if (params.mRemixReadOnly != 0) {
        stream << " read-only";
    }
    if (params.mPracticeMode != 0) {
        stream << " practice";
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const NetGameParams &params) {
    BinStream &out =
        stream.WriteString(params.mSong.c_str()).WriteString(params.mRemixName.c_str());
    WriteByte(out, params.mSkillLevel);
    WriteByte(out, params.mMaxPlayers);
    WriteByte(out, params.mRuleSet);
    WriteByte(out, params.mCommunity);
    WriteByte(out, params.mPowerupLevel);
    WriteByte(out, params.mRemixReadOnly);
    WriteByte(out, params.mLoadRemix);
    WriteByte(out, params.mPracticeMode);
    return stream;
}

BinStream &operator>>(BinStream &stream, NetGameParams &params) {
    BinStream &in = stream >> params.mSong >> params.mRemixName;
    signed char settings[kSettingCount];
    for (signed char &cSetting : settings) {
        in.Read(&cSetting, sizeof(cSetting));
    }
    unsigned char cFlag;
    in.Read(&cFlag, sizeof(cFlag));
    params.mRemixReadOnly = cFlag != 0;
    in.Read(&cFlag, sizeof(cFlag));
    params.mLoadRemix = cFlag != 0;
    in.Read(&cFlag, sizeof(cFlag));
    params.mPracticeMode = cFlag != 0;
    params.mPowerupLevel = settings[kSettingPowerupLevel];
    params.mSkillLevel = settings[kSettingSkillLevel];
    params.mMaxPlayers = settings[kSettingMaxPlayers];
    params.mRuleSet = settings[kSettingRuleSet];
    params.mCommunity = settings[kSettingCommunity];
    return stream;
}
