#include "game/notecondition.h"

#include "os/debug.h"

namespace {

constexpr char kFirstNote = 'A';
constexpr char kLastNote = 'G';

} // namespace

NoteCondition::NoteCondition(DataArray *pCondition) {
    mNote = pCondition->Sym(1)[0];
    if (mNote < kFirstNote || mNote > kLastNote) {
        DebugWarn("Note %c not between A through G", mNote, pCondition->mFile, pCondition->mLine);
    }
}
