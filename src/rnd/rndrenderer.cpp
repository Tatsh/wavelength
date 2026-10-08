#include "rnd/rndrenderer.h"

#include "os/system.h"
#include "script/dataarray.h"

namespace {

constexpr float kDefaultClearGrey = 0.3f;
constexpr float kOpaque = 1.0f;
constexpr int kDefaultScreenWidth = 640;
constexpr int kDefaultScreenHeight = 480;
constexpr int kDefaultScreenBpp = 16;
constexpr int kDefaultTimerMaxMs = 50;

// Elements of the `clear_color` array after its tag.
constexpr int kClearColorRed = 1;
constexpr int kClearColorGreen = 2;
constexpr int kClearColorBlue = 3;

} // namespace

RndRenderer::RndRenderer() {
    mClearColor.b = kDefaultClearGrey;
    mClearColor.a = kOpaque;
    mScreenWidth = kDefaultScreenWidth;
    mScreenHeight = kDefaultScreenHeight;
    mScreenBpp = kDefaultScreenBpp;
    mTimerMaxMs = kDefaultTimerMaxMs;
    mClearColor.r = kDefaultClearGrey;
    mClearColor.g = kDefaultClearGrey;
    mFrameCount = 0;
    mShowTimers = 0;
    mShowStats = 0;
    mShowRate = 0;
}

void RndRenderer::Init() {
    DataArray *pConfig = SystemConfig()->FindArray("rnd", true);
    pConfig->FindInt("bpp", &mScreenBpp, false);
    pConfig->FindInt("height", &mScreenHeight, false);
    pConfig->FindInt("width", &mScreenWidth, false);
    pConfig->FindInt("show_timers", &mShowTimers, false);
    pConfig->FindInt("show_rate", &mShowRate, false);
    pConfig->FindInt("show_stats", &mShowStats, false);
    pConfig->FindInt("timer_maxms", &mTimerMaxMs, false);
    DataArray *pClear = pConfig->FindArray("clear_color", false);
    if (pClear != nullptr) {
        const float fRed = pClear->Float(kClearColorRed);
        const float fGreen = pClear->Float(kClearColorGreen);
        mClearColor.b = pClear->Float(kClearColorBlue);
        mClearColor.r = fRed;
        mClearColor.g = fGreen;
    }
}

void RndRenderer::Terminate() {
}

void RndRenderer::SetClearColor(const Color &color) {
    mClearColor = color;
}

void RndRenderer::SetShowTimers(int bShow, int nMaxMs) {
    mTimerMaxMs = nMaxMs;
    mShowTimers = bShow;
}

void RndRenderer::SetShowStats(int bShow) {
    mShowStats = bShow;
}

void RndRenderer::SetShowRate(int bShow) {
    mShowRate = bShow;
}

void RndRenderer::DumpStats() {
}

void RndRenderer::ScreenDump([[maybe_unused]] const char *pszName) {
}

void RndRenderer::DrawRect([[maybe_unused]] const Rect &rect, [[maybe_unused]] const Color &color) {
}

float RndRenderer::DrawString([[maybe_unused]] const char *pszText,
                              const Vector2 &pos,
                              [[maybe_unused]] const Vector2 &charSize,
                              [[maybe_unused]] const Color &color) {
    return pos.y;
}

void RndRenderer::BeginFrame() {
    ++mFrameCount;
}

void RndRenderer::EndFrame() {
}

void RndRenderer::ClearAndFlip() {
}

void RndRenderer::RestoreFrameBufferTarget() {
}

float RndRenderer::AspectRatio() {
    return static_cast<float>(mScreenHeight) / static_cast<float>(mScreenWidth);
}

bool RndRenderer::SupportsTextures() {
    return false;
}
