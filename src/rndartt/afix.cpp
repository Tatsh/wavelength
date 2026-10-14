#include "rndartt/afix.h"

namespace {

constexpr int kFixedOne = 0x100;

} // namespace

int AFix::onehalf = kFixedOne / 2;

int AFix::e = 695;

int AFix::pi = 804;

int AFix::epsilon = 1;

int AFix::max = 0x7fffff;
