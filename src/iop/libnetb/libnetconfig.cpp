#include "libnetb/libnetconfig.h"

#include <stdio.h>

#include "libnetb/libnetb.h"

namespace {

constexpr int kDefaultSendDelay = 10;
constexpr int kMinimumSendDelay = 1;
constexpr int kMaximumSendDelay = 10;
constexpr int kMicrosecondsPerMillisecond = 1000;
constexpr int kBuildDateSize = 64;

constexpr char kVersionMessage[] = "%s (%s)\n";

} // namespace

void LibnetConfig::Apply() {
    char built[kBuildDateSize];
    Libnet::FormatBuildDate(built, kBuildDateSize - 1);
    printf(kVersionMessage, Libnet::GetVersion(), built);
    int delay = mSendDelay;
    if (delay > kMaximumSendDelay) {
        delay = kMaximumSendDelay;
    } else if (delay < kMinimumSendDelay) {
        delay = kMinimumSendDelay;
    }
    Libnet::sSendDelay = delay * kMicrosecondsPerMillisecond;
}

void LibnetConfig::Init() {
    mSendDelay = kDefaultSendDelay;
    mVerbose = 0;
}
