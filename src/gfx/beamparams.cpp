#include "gfx/beamparams.h"

#include "gfx/gfxconfig.h"
#include "rnd/manager.h"

namespace {

// The points of a look that is not loaded.
constexpr int kNotLoaded = -1;

// The fold of a line before the look is loaded, in degrees.
constexpr float kDefaultFoldAngle = 90.0f;

constexpr float kPi = 3.14159274f;
constexpr float kDegreesPerHalfTurn = 180.0f;

} // namespace

BeamParams::BeamParams(const char *pszName)
    : mName(pszName), mPoints(kNotLoaded), mWidth(0.0f), mFoldAngle(kDefaultFoldAngle),
      mMat(nullptr) {
    mAmplitudes[kEndStart] = 0.0f;
    mMaxTravelPeriod.y = 1.0f;
    mMinFrequency.x = 0.0f;
    mMinFrequency.y = 0.0f;
    mMaxFrequency.x = 0.0f;
    mMaxFrequency.y = 0.0f;
    mMinTravelPeriod.x = 1.0f;
    mMinTravelPeriod.y = 1.0f;
    mMaxTravelPeriod.x = 1.0f;
    mAmplitudes[kEndFinish] = 0.0f;
}

BeamParams::~BeamParams() = default;

void BeamParams::Load(DataArray *pConfig, DataArray *pDefaults, bool, float fScale) {
    if (pConfig != nullptr) {
        pConfig = pConfig->FindArray("beams", false);
    }
    if (pDefaults != nullptr) {
        pDefaults = pDefaults->FindArray("beams", false);
    }
    if (pConfig != nullptr) {
        pConfig = pConfig->FindArray(mName.c_str(), false);
    }
    if (pDefaults != nullptr) {
        pDefaults = pDefaults->FindArray(mName.c_str(), false);
    }
    FindConfigInt(pConfig, pDefaults, "points", &mPoints, true);
    FindConfigFloat(pConfig, pDefaults, "width", &mWidth, true);
    FindConfigFloat(pConfig, pDefaults, "fold_angle", &mFoldAngle, true);
    mWidth *= fScale;
    const char *pszMat;
    FindConfigSymbol(pConfig, pDefaults, "mat", &pszMat, true);
    mMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszMat));
    FindConfigColor(pConfig, pDefaults, "start_color", &mColors[kEndStart], true);
    FindConfigColor(pConfig, pDefaults, "finish_color", &mColors[kEndFinish], true);
    FindConfigFloat(pConfig, pDefaults, "start_amplitude", &mAmplitudes[kEndStart], true);
    FindConfigFloat(pConfig, pDefaults, "finish_amplitude", &mAmplitudes[kEndFinish], true);
    mAmplitudes[kEndStart] *= fScale;
    mAmplitudes[kEndFinish] *= fScale;
    FindConfigVector2(pConfig, pDefaults, "min_frequency", &mMinFrequency, true);
    FindConfigVector2(pConfig, pDefaults, "max_frequency", &mMaxFrequency, true);
    FindConfigVector2(pConfig, pDefaults, "min_travel_period", &mMinTravelPeriod, true);
    FindConfigVector2(pConfig, pDefaults, "max_travel_period", &mMaxTravelPeriod, true);
}

void BeamParams::Apply(Rnd::Line *pLine, Rnd::Mat *pMat) const {
    if (mPoints < 0) {
        return;
    }
    pMat->Copy(mMat, 0);
    pLine->SetNumPoints(mPoints);
    pLine->mWidth = mWidth;
    pLine->SetFoldAngle((mFoldAngle * kPi) / kDegreesPerHalfTurn);
}

const Color *BeamParams::GetColor(int nEnd, const Color *pDefault) const {
    const Color *pColor = &mColors[nEnd];
    return (pColor->a == 0.0f) ? pDefault : pColor;
}
