#include "os/codecheck.h"

#ifdef VIDEO_STANDARD_PAL

std::unique_ptr<const unsigned char *> CodeCheck::sStart;
std::unique_ptr<const unsigned char *> CodeCheck::sEnd;
std::unique_ptr<const unsigned char *> CodeCheck::sCursor;
std::unique_ptr<unsigned int> CodeCheck::sCrc;
std::unique_ptr<int> CodeCheck::sExpected;
std::unique_ptr<unsigned int[]> CodeCheck::sTable;
int CodeCheck::sTableBuilt = 0;

#endif
