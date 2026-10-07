#pragma once

#include <cstring>

/**
 * Ordering of C strings by their text, for maps keyed by name.
 *
 * Inline. Every map of the front end keyed by a name expands it.
 */
class StrLess {
public:
    /**
     * Report whether one string sorts before another.
     *
     * @param pszFirst The first string.
     * @param pszSecond The second string.
     * @return Whether `strcmp()` places pszFirst first.
     */
    bool operator()(const char *pszFirst, const char *pszSecond) const {
        return std::strcmp(pszFirst, pszSecond) < 0;
    }
};
