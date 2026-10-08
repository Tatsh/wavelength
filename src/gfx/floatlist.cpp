#include "gfx/floatlist.h"

void LoadFloatList(std::vector<float> &values, const DataArray *pData) {
    values.resize(pData->mSize - 1);
    for (unsigned i = 0; i < values.size(); ++i) {
        values[i] = pData->Float(i + 1);
    }
}

float EvalPolynomial(const std::vector<float> &coefficients, float flX) {
    float flValue = 0.0f;
    for (const auto &flCoefficient : coefficients) {
        flValue = flX * flValue + flCoefficient;
    }
    return flValue;
}
