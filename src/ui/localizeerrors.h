#pragma once

#include <list>
#include <map>
#include <memory>

#include "os/string.h"

/**
 * Record of text tokens that went without a localisation, kept for the report at the end of a load.
 *
 * The RTTI includes the class name through the lists that store it. The object is 0x20 bytes.
 * UIPanel::LocalizeTexts() records the tokens of a panel's file that the locale lacks, and
 * UIManager::ReportLocalizeErrors() records the locale's tokens no loaded object used. Store()
 * keeps a copy until the report. The edit mode of the `ui` configuration turns the token counts on.
 */
class LocalizeErrors {
public:
    /** What a record lists, the values of mType. */
    enum Type {
        kTypeMissing = 0, /*!< Tokens of a file that the locale lacks. */
        kTypeUnused = 1,  /*!< Tokens of the locale that no loaded object used. */
    };

    /** One token of a record. */
    struct SingleError {
        String mObject; /*!< The object that named the token, empty for an unused token. */
        String mText;   /*!< The text the object showed, or the unused token. */
    };

    /**
     * Construct an empty record of a file's missing tokens.
     *
     * Inline. UIPanel::LocalizeTexts() expands it.
     *
     * @param pszFile The file.
     */
    explicit LocalizeErrors(const char *pszFile) : mFile(pszFile), mType(kTypeMissing) {
    }

    /**
     * Construct an empty record with no file.
     *
     * Inline. UIManager::ReportLocalizeErrors() expands it.
     *
     * @param nType One of Type.
     */
    explicit LocalizeErrors(int nType) : mType(nType) {
    }

    /**
     * Copy another record.
     *
     * @param other The record.
     * @ghidraAddress NTSC-U/C: 0x003786b8
     * @ghidraAddress PAL: 0x003e6de8
     */
    LocalizeErrors(const LocalizeErrors &other) = default;

    /**
     * Keep a copy of the record for the report.
     *
     * @ghidraAddress NTSC-U/C: 0x00209978
     * @ghidraAddress PAL: 0x00212778
     */
    void Store();

    /**
     * Add to the number of times a token was used.
     *
     * @param pszToken The token.
     * @param nCount The number to add.
     * @ghidraAddress NTSC-U/C: 0x00209a80
     * @ghidraAddress PAL: 0x00212880
     */
    static void CountToken(const char *pszToken, int nCount);

    /**
     * Enter every token of the locale in the counts, with no uses added.
     *
     * @ghidraAddress NTSC-U/C: 0x00209c30
     * @ghidraAddress PAL: 0x00212a30
     */
    static void CountLocaleTokens();

    /** @return The file the tokens came from, or empty. */
    const String &GetFile() const {
        return mFile;
    }

    /** @return One of Type. */
    int GetType() const {
        return mType;
    }

    /** @return The tokens. */
    std::list<SingleError> &GetErrors() {
        return mErrors;
    }

    /** @return The tokens. */
    const std::list<SingleError> &GetErrors() const {
        return mErrors;
    }

    /** @return The records Store() retained, or null before the first Store(). */
    static std::list<LocalizeErrors> *GetRecords() {
        return sRecords.get();
    }

    /** @return The number of uses of each token, or null before the first CountToken(). */
    static std::map<String, int> *GetTokenCounts() {
        return sTokenCounts.get();
    }

    /** Delete the records Store() retained. */
    static void ClearRecords() {
        sRecords.reset();
    }

    /** Delete the counts of the uses of each token. */
    static void ClearTokenCounts() {
        sTokenCounts.reset();
    }

private:
    String mFile;                   /*!< The file the tokens came from, or empty. */
    int mType;                      /*!< One of Type. */
    std::list<SingleError> mErrors; /*!< The tokens. */

    /**
     * The records Store() retained, created on first use.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcdc
     * @ghidraAddress PAL: 0x0042eb74
     */
    static std::unique_ptr<std::list<LocalizeErrors>> sRecords;

    /**
     * The number of uses of each token, created on first use.
     *
     * @ghidraAddress NTSC-U/C: 0x003afce0
     * @ghidraAddress PAL: 0x0042eb78
     */
    static std::unique_ptr<std::map<String, int>> sTokenCounts;
};
