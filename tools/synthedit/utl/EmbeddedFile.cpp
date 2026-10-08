#include "utl/EmbeddedFile.h"

EmbeddedFile::~EmbeddedFile() {
    delete mFile;
}
