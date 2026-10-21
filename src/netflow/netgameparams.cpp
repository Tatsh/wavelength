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
    stream.Print(params.GetSong().c_str());
    if (params.GetRuleSet() == GameDb::kRuleSetRemix || params.GetLoadRemix() != 0) {
        (stream << " ").Print(params.GetRemixName().c_str());
    }
    stream << " sk:" << params.GetSkillLevel() << " max:" << params.GetMaxPlayers();
    switch (params.GetRuleSet()) {
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
    switch (params.GetCommunity()) {
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
    switch (params.GetPowerupLevel()) {
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
    if (params.GetLoadRemix() != 0) {
        stream << " remix-data";
    }
    if (params.GetRemixReadOnly() != 0) {
        stream << " read-only";
    }
    if (params.GetPracticeMode() != 0) {
        stream << " practice";
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const NetGameParams &params) {
    BinStream &out =
        stream.WriteString(params.GetSong().c_str()).WriteString(params.GetRemixName().c_str());
    WriteByte(out, params.GetSkillLevel());
    WriteByte(out, params.GetMaxPlayers());
    WriteByte(out, params.GetRuleSet());
    WriteByte(out, params.GetCommunity());
    WriteByte(out, params.GetPowerupLevel());
    WriteByte(out, params.GetRemixReadOnly());
    WriteByte(out, params.GetLoadRemix());
    WriteByte(out, params.GetPracticeMode());
    return stream;
}

BinStream &operator>>(BinStream &stream, NetGameParams &params) {
    BinStream &in = stream >> params.GetSong() >> params.GetRemixName();
    signed char settings[kSettingCount];
    for (signed char &cSetting : settings) {
        in.Read(&cSetting, sizeof(cSetting));
    }
    unsigned char cFlag;
    in.Read(&cFlag, sizeof(cFlag));
    params.SetRemixReadOnly(cFlag != 0);
    in.Read(&cFlag, sizeof(cFlag));
    params.SetLoadRemix(cFlag != 0);
    in.Read(&cFlag, sizeof(cFlag));
    params.SetPracticeMode(cFlag != 0);
    params.SetPowerupLevel(settings[kSettingPowerupLevel]);
    params.SetSkillLevel(settings[kSettingSkillLevel]);
    params.SetMaxPlayers(settings[kSettingMaxPlayers]);
    params.SetRuleSet(settings[kSettingRuleSet]);
    params.SetCommunity(settings[kSettingCommunity]);
    return stream;
}
