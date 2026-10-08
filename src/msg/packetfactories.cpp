#include "msg/arbiterpacket.h"
#include "msg/bumperpacket.h"
#include "msg/capturepacket.h"
#include "msg/cripplerpacket.h"
#include "msg/editgempacket.h"
#include "msg/erasebarpacket.h"
#include "msg/erasesectionpacket.h"
#include "msg/finalscorepacket.h"
#include "msg/freestylebumppacket.h"
#include "msg/freestylepacket.h"
#include "msg/fxpacket.h"
#include "msg/messagefactory.h"
#include "msg/multiplierpacket.h"
#include "msg/mutevocalspacket.h"
#include "msg/playerupdatepacket.h"
#include "msg/previewpacket.h"
#include "msg/remixerupdatepacket.h"
#include "msg/sectionchangepacket.h"
#include "msg/slowdownpacket.h"

namespace {

// The identities the registrars below pass, the same values the Type() words hold.
constexpr int kPlayerUpdatePacketType = 401;
constexpr int kArbiterPacketType = 402;
constexpr int kCapturePacketType = 403;
constexpr int kBumperPacketType = 404;
constexpr int kCripplerPacketType = 405;
constexpr int kSlowdownPacketType = 406;
constexpr int kMultiplierPacketType = 407;
constexpr int kFreestylePacketType = 408;
constexpr int kFreestyleBumpPacketType = 409;
constexpr int kFinalScorePacketType = 410;
constexpr int kRemixerUpdatePacketType = 411;
constexpr int kPreviewPacketType = 412;
constexpr int kEditGemPacketType = 413;
constexpr int kEraseBarPacketType = 414;
constexpr int kEraseSectionPacketType = 415;
constexpr int kSectionChangePacketType = 416;
constexpr int kFXPacketType = 417;
constexpr int kMuteVocalsPacketType = 418;

// The unit's static initialiser at NTSC-U/C: 0x00123558, PAL: 0x00124cd8, and its global
// constructor at NTSC-U/C: 0x00123728, PAL: 0x00124ea8, construct the factories.
// NTSC-U/C: 0x00435f30
const MessageFactory kPlayerUpdatePacketFactory(kPlayerUpdatePacketType, PlayerUpdatePacket::New);
// NTSC-U/C: 0x00435f38
const MessageFactory kArbiterPacketFactory(kArbiterPacketType, ArbiterPacket::New);
// NTSC-U/C: 0x00435f40
const MessageFactory kCapturePacketFactory(kCapturePacketType, CapturePacket::New);
// NTSC-U/C: 0x00435f48
const MessageFactory kBumperPacketFactory(kBumperPacketType, BumperPacket::New);
// NTSC-U/C: 0x00435f50
const MessageFactory kCripplerPacketFactory(kCripplerPacketType, CripplerPacket::New);
// NTSC-U/C: 0x00435f58
const MessageFactory kSlowdownPacketFactory(kSlowdownPacketType, SlowdownPacket::New);
// NTSC-U/C: 0x00435f60
const MessageFactory kMultiplierPacketFactory(kMultiplierPacketType, MultiplierPacket::New);
// NTSC-U/C: 0x00435f68
const MessageFactory kFreestylePacketFactory(kFreestylePacketType, FreestylePacket::New);
// NTSC-U/C: 0x00435f70
const MessageFactory kFreestyleBumpPacketFactory(kFreestyleBumpPacketType,
                                                 FreestyleBumpPacket::New);
// NTSC-U/C: 0x00435f78
const MessageFactory kFinalScorePacketFactory(kFinalScorePacketType, FinalScorePacket::New);
// NTSC-U/C: 0x00435f80
const MessageFactory kRemixerUpdatePacketFactory(kRemixerUpdatePacketType,
                                                 RemixerUpdatePacket::New);
// NTSC-U/C: 0x00435f88
const MessageFactory kSectionChangePacketFactory(kSectionChangePacketType,
                                                 SectionChangePacket::New);
// NTSC-U/C: 0x00435f90
const MessageFactory kFXPacketFactory(kFXPacketType, FXPacket::New);
// NTSC-U/C: 0x00435f98
const MessageFactory kMuteVocalsPacketFactory(kMuteVocalsPacketType, MuteVocalsPacket::New);
// NTSC-U/C: 0x00435fa0
const MessageFactory kEditGemPacketFactory(kEditGemPacketType, EditGemPacket::New);
// NTSC-U/C: 0x00435fa8
const MessageFactory kEraseBarPacketFactory(kEraseBarPacketType, EraseBarPacket::New);
// NTSC-U/C: 0x00435fb0
const MessageFactory kEraseSectionPacketFactory(kEraseSectionPacketType, EraseSectionPacket::New);
// NTSC-U/C: 0x00435fb8
const MessageFactory kPreviewPacketFactory(kPreviewPacketType, PreviewPacket::New);

} // namespace

Message *PlayerUpdatePacket::New() {
    return new PlayerUpdatePacket;
}

Message *ArbiterPacket::New() {
    return new ArbiterPacket;
}

Message *CapturePacket::New() {
    return new CapturePacket;
}

Message *BumperPacket::New() {
    return new BumperPacket;
}

Message *CripplerPacket::New() {
    return new CripplerPacket;
}

Message *SlowdownPacket::New() {
    return new SlowdownPacket;
}

Message *MultiplierPacket::New() {
    return new MultiplierPacket;
}

Message *FreestylePacket::New() {
    return new FreestylePacket;
}

Message *FreestyleBumpPacket::New() {
    return new FreestyleBumpPacket;
}

Message *FinalScorePacket::New() {
    return new FinalScorePacket;
}

Message *RemixerUpdatePacket::New() {
    return new RemixerUpdatePacket;
}

Message *SectionChangePacket::New() {
    return new SectionChangePacket;
}

Message *FXPacket::New() {
    return new FXPacket;
}

Message *MuteVocalsPacket::New() {
    return new MuteVocalsPacket;
}

Message *EditGemPacket::New() {
    return new EditGemPacket;
}

Message *EraseBarPacket::New() {
    return new EraseBarPacket;
}

Message *EraseSectionPacket::New() {
    return new EraseSectionPacket;
}

Message *PreviewPacket::New() {
    return new PreviewPacket; // Retail allocates 0x14 bytes here, four short of the class.
}
