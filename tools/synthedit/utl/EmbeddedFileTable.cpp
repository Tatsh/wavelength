#include "utl/EmbeddedFileTable.h"

EmbeddedFileTable::EmbeddedFileTable() {
}

EmbeddedFileTable::~EmbeddedFileTable() {
}

EmbeddedFileTable &EmbeddedFileTable::Instance() {
    // 0x100c38f8
    static EmbeddedFileTable table;
    return table;
}

void EmbeddedFileTable::ClearAll() {
    Instance().Clear();
}

void EmbeddedFileTable::Clear() {
    std::vector<EmbeddedFile *>::iterator it;
    for (it = mFiles.begin(); it != mFiles.end(); ++it) {
        delete *it;
    }
    mFiles.erase(mFiles.begin(), mFiles.end());
}
