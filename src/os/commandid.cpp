#include "os/commandid.h"

namespace {

constexpr int kDefaultValue = 0;

} // namespace

CommandId::CommandId(int nValue) : mValue(nValue) {
}

CommandId::CommandId(const CommandId &other) : mValue(other.mValue) {
}

CommandId &CommandId::operator=(const CommandId &other) {
    mValue = other.mValue;
    return *this;
}

// The unit's static initialiser constructs TheDefaultCommandId, and the global constructor of the
// unit runs the initialiser.
// NTSC-U/C: 0x002817a8, PAL: 0x0028b0a8
// NTSC-U/C: 0x002817d8, PAL: 0x0028b0d8
CommandId TheDefaultCommandId(kDefaultValue);
