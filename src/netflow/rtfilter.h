#pragma once

/**
 * One filter of the game list of the lobby server.
 *
 * The RTTI of the vector of filters identifies the type. The record is 0xc bytes.
 */
struct RTFilter {
    /** Values of mField. */
    enum Field {
        kFieldGameLevel = 4,     /*!< The song of the game. */
        kFieldRulesSet = 6,      /*!< The game mode and the skill level. */
        kFieldGenericField2 = 8, /*!< The chat channel of the host. */
        kFieldGenericField3 = 9, /*!< Non-zero while the host takes players. */
    };

    /** Values of mComparison. */
    enum Comparison {
        kComparisonLessOrEqual = 1, /*!< The field is at most mValue. */
        kComparisonEqual = 2,       /*!< The field equals mValue. */
        kComparisonMoreOrEqual = 3, /*!< The field is at least mValue. */
        kComparisonNotEqual = 5,    /*!< The field differs from mValue. */
    };

    /**
     * Report whether two filters are the same.
     *
     * Inline.
     *
     * @param other The other filter.
     * @return Whether every member is equal.
     */
    bool operator==(const RTFilter &other) const {
        return mField == other.mField && mComparison == other.mComparison && mValue == other.mValue;
    }

    int mField;      /*!< One of Field. */
    int mComparison; /*!< One of Comparison. */
    int mValue;      /*!< The value the field is compared with. */
};
