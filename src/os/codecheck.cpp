#include "os/codecheck.h"

#ifdef VIDEO_STANDARD_PAL

const unsigned char **CodeCheck::sStart;
const unsigned char **CodeCheck::sEnd;
const unsigned char **CodeCheck::sCursor;
unsigned int *CodeCheck::sCrc;
int *CodeCheck::sExpected;
unsigned int *CodeCheck::sTable;
int CodeCheck::sTableBuilt = 0;

#endif
