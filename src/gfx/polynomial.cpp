#include "gfx/polynomial.h"

Polynomial::Polynomial() = default;

Polynomial::~Polynomial() = default;

void Polynomial::Load(const DataArray *pData) {
    mCoefficients.resize(pData->Size() - 1);
    for (unsigned i = 0; i < mCoefficients.size(); ++i) {
        mCoefficients[i] = pData->Float(i + 1);
    }
}

float Polynomial::Eval(float flX) const {
    float flValue = 0.0f;
    for (const auto &flCoefficient : mCoefficients) {
        flValue = flX * flValue + flCoefficient;
    }
    return flValue;
}
