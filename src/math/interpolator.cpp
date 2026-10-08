#include "math/interpolator.h"

#include <math.h>
#include <string.h>

#include "os/debug.h"
#include "os/mem.h"

namespace {

// An input range narrower than this is treated as empty.
constexpr float kMinRange = 1e-6F;

// Defaults of the optional last number of a configuration array.
constexpr float kDefaultExponent = 2.0F;
constexpr float kDefaultSeverity = 10.0F;

// Indices of a configuration array: the class symbol, then the numbers.
enum ConfigIndex {
    kConfigType = 0,
    kConfigY0 = 1,
    kConfigY1 = 2,
    kConfigX0 = 3,
    kConfigX1 = 4,
    kConfigShape = 5,
    kConfigTableX0 = 1,
    kConfigTableX1 = 2,
    kConfigTableFirst = 3
};

// The size of an array that includes the optional exponent or severity.
constexpr int kConfigSizeWithShape = kConfigShape + 1;

// The allocation tag of the tables.
constexpr const char *kTableTag = "Interp";

// Read the numbers of an array into the table of an interpolator.
inline void FillTable(TableInterpolator *pInterp, const DataArray *pArray, int nFirst) {
    for (int i = 0; i < pInterp->mCount; ++i) {
        pInterp->mTable[i] = pArray->Float(i + nFirst);
    }
}

// The optional last number of a configuration array.
inline float ShapeOrDefault(const DataArray *pArray, float fDefault) {
    return pArray->Size() >= kConfigSizeWithShape ? pArray->Float(kConfigShape) : fDefault;
}

} // namespace

float Interpolator::Eval(float fX) {
    if (fX <= mX0) {
        return mY0;
    }
    if (fX >= mX1) {
        return mY1;
    }
    return Interp(fX);
}

Interpolator *ObjectToInterpolator(const DataArray *pArray) {
    if (strcmp(pArray->Sym(kConfigType), "linear") == 0) {
        return new LinearInterpolator(pArray->Float(kConfigY0),
                                      pArray->Float(kConfigY1),
                                      pArray->Float(kConfigX0),
                                      pArray->Float(kConfigX1));
    }
    if (strcmp(pArray->Sym(kConfigType), "exp") == 0) {
        const float fY0 = pArray->Float(kConfigY0);
        const float fY1 = pArray->Float(kConfigY1);
        const float fX0 = pArray->Float(kConfigX0);
        const float fX1 = pArray->Float(kConfigX1);
        return new ExpInterpolator(fY0, fY1, fX0, fX1, ShapeOrDefault(pArray, kDefaultExponent));
    }
    if (strcmp(pArray->Sym(kConfigType), "invexp") == 0) {
        const float fY0 = pArray->Float(kConfigY0);
        const float fY1 = pArray->Float(kConfigY1);
        const float fX0 = pArray->Float(kConfigX0);
        const float fX1 = pArray->Float(kConfigX1);
        return new InvExpInterpolator(fY0, fY1, fX0, fX1, ShapeOrDefault(pArray, kDefaultExponent));
    }
    if (strcmp(pArray->Sym(kConfigType), "atan") == 0) {
        const float fY0 = pArray->Float(kConfigY0);
        const float fY1 = pArray->Float(kConfigY1);
        const float fX0 = pArray->Float(kConfigX0);
        const float fX1 = pArray->Float(kConfigX1);
        return new ATanInterpolator(fY0, fY1, fX0, fX1, ShapeOrDefault(pArray, kDefaultSeverity));
    }
    if (strcmp(pArray->Sym(kConfigType), "table") == 0) {
        const float fX0 = pArray->Float(kConfigTableX0);
        return new TableInterpolator(fX0, pArray->Float(kConfigTableX1), pArray, kConfigTableFirst);
    }
    if (strcmp(pArray->Sym(kConfigType), "tablelin") == 0) {
        const float fX0 = pArray->Float(kConfigTableX0);
        return new TableLinInterpolator(
            fX0, pArray->Float(kConfigTableX1), pArray, kConfigTableFirst);
    }
    DebugWarn("unknown interpolator type: %s\nat %d in %s",
              pArray->Sym(kConfigType),
              pArray->mLine,
              pArray->mFile);
    return nullptr;
}

#pragma mark - LinearInterpolator

LinearInterpolator::LinearInterpolator(float fY0, float fY1, float fX0, float fX1) {
    LinearInterpolator::Reset(fY0, fY1, fX0, fX1);
}

void LinearInterpolator::Reset(float fY0, float fY1, float fX0, float fX1) {
    const float fRange = fX1 - fX0;
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    if (fabsf(fRange) < kMinRange) {
        mSlope = 0.0F;
    } else {
        mSlope = (fY1 - fY0) / fRange;
    }
    mOffset = (-mX0 * mSlope) + mY0;
}

float LinearInterpolator::Interp(float fX) {
    return (mSlope * fX) + mOffset;
}

#pragma mark - ExpInterpolator

ExpInterpolator::ExpInterpolator(float fY0, float fY1, float fX0, float fX1, float fExponent) {
    ExpInterpolator::Reset(fY0, fY1, fX0, fX1, fExponent);
}

void ExpInterpolator::Reset(float fY0, float fY1, float fX0, float fX1, float fExponent) {
    const float fRange = fX1 - fX0;
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    mInvXRange = fabsf(fRange) < kMinRange ? 1.0F : 1.0F / fRange;
    mExponent = fExponent;
    mRange = fY1 - fY0;
}

void ExpInterpolator::Reset(float fY0, float fY1, float fX0, float fX1) {
    Reset(fY0, fY1, fX0, fX1, mExponent);
}

float ExpInterpolator::Interp(float fX) {
    return (powf((fX - mX0) * mInvXRange, mExponent) * mRange) + mY0;
}

#pragma mark - InvExpInterpolator

InvExpInterpolator::InvExpInterpolator(
    float fY0, float fY1, float fX0, float fX1, float fExponent) {
    InvExpInterpolator::Reset(fY0, fY1, fX0, fX1, fExponent);
}

void InvExpInterpolator::Reset(float fY0, float fY1, float fX0, float fX1, float fExponent) {
    const float fRange = fX1 - fX0;
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    mInvXRange = fabsf(fRange) < kMinRange ? 1.0F : 1.0F / fRange;
    mExponent = fExponent;
    mRange = fY1 - fY0;
}

void InvExpInterpolator::Reset(float fY0, float fY1, float fX0, float fX1) {
    Reset(fY0, fY1, fX0, fX1, mExponent);
}

float InvExpInterpolator::Interp(float fX) {
    const float fPower = powf(1.0F - ((fX - mX0) * mInvXRange), mExponent);
    return ((1.0F - fPower) * mRange) + mY0;
}

#pragma mark - ATanInterpolator

ATanInterpolator::ATanInterpolator(float fY0, float fY1, float fX0, float fX1, float fSeverity)
    : mLinear(0.0F, 0.0F, 0.0F, 0.0F) {
    ATanInterpolator::Reset(fY0, fY1, fX0, fX1, fSeverity);
}

void ATanInterpolator::Reset(float fY0, float fY1, float fX0, float fX1, float fSeverity) {
    mLinear.LinearInterpolator::Reset(-fSeverity, fSeverity, fX0, fX1);
    mX0 = fX0;
    mX1 = fX1;
    mY0 = fY0;
    mY1 = fY1;
    const float fLow = atanf(-fSeverity);
    const float fRange = fY1 - fY0;
    mSeverity = fSeverity;
    mScale = fRange / (-fLow - fLow);
    mOffset = (fRange * 0.5F) + fY0;
}

void ATanInterpolator::Reset(float fY0, float fY1, float fX0, float fX1) {
    Reset(fY0, fY1, fX0, fX1, mSeverity);
}

float ATanInterpolator::Interp(float fX) {
    return (atanf(mLinear.LinearInterpolator::Interp(fX)) * mScale) + mOffset;
}

#pragma mark - TableInterpolator

TableInterpolator::TableInterpolator(int nCount, float fX0, float fX1) {
    TableInterpolator::SetSize(nCount, fX0, fX1);
    memset(mTable, 0, mCount * sizeof(float));
}

TableInterpolator::TableInterpolator(float fX0, float fX1, const DataArray *pArray, int nFirst) {
    TableInterpolator::SetSize(pArray->Size() - nFirst, fX0, fX1);
    FillTable(this, pArray, nFirst);
}

TableInterpolator::~TableInterpolator() {
    if (mTable != nullptr) {
        PoolMemFree(mTable);
        mTable = nullptr;
    }
}

float TableInterpolator::Interp(float fX) {
    const int nIndex = static_cast<int>(((fX - mX0) * mInvStep) + 0.5F);
    if (nIndex <= 0) {
        return mTable[0];
    }
    if (nIndex < mCount) {
        return mTable[nIndex];
    }
    return mTable[mCount - 1];
}

void TableInterpolator::Reset(float, float, float, float) {
    DebugWarn("TableInterpolator cannot reset\nbased on end-points alone.");
}

void TableInterpolator::Resample(Interpolator &source, int nCount) {
    if (mTable == nullptr || nCount != mCount) {
        if (mTable != nullptr) {
            PoolMemFree(mTable);
        }
        mTable = static_cast<float *>(PoolMemAlloc(nCount * sizeof(float), kTableTag, 0));
        mCount = nCount;
    }
    float fX = source.mX0;
    mX0 = fX;
    mX1 = source.mX1;
    mY0 = source.mY0;
    mY1 = source.mY1;
    const float fStep = (mX1 - fX) / static_cast<float>(nCount - 1);
    for (int i = 0; i < mCount; ++i) {
        mTable[i] = source.Interp(fX);
        fX += fStep;
    }
    Update();
}

void TableInterpolator::SetSize(int nCount, float fX0, float fX1) {
    if (mTable == nullptr || nCount != mCount) {
        if (mTable != nullptr) {
            PoolMemFree(mTable);
        }
        mTable = static_cast<float *>(PoolMemAlloc(nCount * sizeof(float), kTableTag, 0));
        memset(mTable, 0, mCount * sizeof(float)); // Yes, the binary clears the old count.
        mCount = nCount;
    }
    mX0 = fX0;
    mX1 = fX1;
    Update();
    mY0 = mTable[0];
    mY1 = mTable[nCount - 1]; // An empty table reads the word before the block.
}

void TableInterpolator::Update() {
    const float fRange = mX1 - mX0;
    const float fIntervals = static_cast<float>(mCount - 1);
    mStep = fRange * (1.0F / fIntervals);
    if (fabsf(fRange) < kMinRange) {
        mInvStep = 1.0F;
    } else {
        mInvStep = fIntervals / fRange;
    }
}

#pragma mark - TableLinInterpolator

TableLinInterpolator::TableLinInterpolator(Interpolator &source, int nCount)
    : TableInterpolator(nCount, 0.0F, 0.0F), mSlopes(nullptr) {
    TableLinInterpolator::Resample(source, nCount);
}

TableLinInterpolator::TableLinInterpolator(float fX0,
                                           float fX1,
                                           const DataArray *pArray,
                                           int nFirst)
    : TableInterpolator(0, 0.0F, 0.0F), mSlopes(nullptr) {
    TableLinInterpolator::SetSize(pArray->Size() - nFirst, fX0, fX1);
    FillTable(this, pArray, nFirst);
    TableLinInterpolator::Update();
}

TableLinInterpolator::~TableLinInterpolator() {
    if (mSlopes != nullptr) {
        PoolMemFree(mSlopes);
        mSlopes = nullptr;
    }
}

float TableLinInterpolator::Interp(float fX) {
    const float fPosition = (fX - mX0) * mInvStep;
    const int nIndex = static_cast<int>(fPosition);
    const float fFraction = fPosition - static_cast<float>(nIndex);
    if (nIndex < 0) {
        return mTable[0];
    }
    if (nIndex + 1 < mCount) {
        return (fFraction * mSlopes[nIndex]) + mTable[nIndex];
    }
    return mTable[mCount - 1];
}

void TableLinInterpolator::Resample(Interpolator &source, int nCount) {
    if (mSlopes == nullptr || nCount != mCount) {
        if (mSlopes != nullptr) {
            PoolMemFree(mSlopes);
        }
        mSlopes = static_cast<float *>(PoolMemAlloc((nCount - 1) * sizeof(float), kTableTag, 0));
    }
    TableInterpolator::Resample(source, nCount);
}

void TableLinInterpolator::SetSize(int nCount, float fX0, float fX1) {
    if (mSlopes == nullptr || nCount != mCount) {
        if (mSlopes != nullptr) {
            PoolMemFree(mSlopes);
        }
        mSlopes = static_cast<float *>(PoolMemAlloc((nCount - 1) * sizeof(float), kTableTag, 0));
    }
    TableInterpolator::SetSize(nCount, fX0, fX1);
}

void TableLinInterpolator::Update() {
    TableInterpolator::Update();
    for (int i = mCount - 2; i >= 0; --i) {
        mSlopes[i] = mTable[i + 1] - mTable[i];
    }
}
