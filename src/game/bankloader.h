#pragma once

#include <vector>

#include "game/playmap.h"
#include "os/string.h"

/**
 * Loader of the sound banks of a song, from its track named "BANK".
 *
 * The class is not polymorphic. The RTTI of the nested PermBankLoader records the name. A
 * permanent bank stays in its synthesiser slot for the whole song. The swapped banks take turns in
 * two slots after the permanent banks, each from the bar the track gives.
 */
class BankLoader {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,    /*!< Not loading. */
        kStateLoading = 1, /*!< The banks load. */
        kStateLoaded = 2,  /*!< Every bank is loaded. */
        kStatePlaying = 3, /*!< The song plays. */
    };

    /**
     * Loader of one permanent bank.
     *
     * The RTTI includes the name. Only the members BankLoader uses are declared.
     */
    class PermBankLoader {
    public:
        /**
         * Construct a loader and ask the synthesiser how many blocks the bank needs.
         *
         * @param nSlot The synthesiser slot.
         * @param pszFile The bank file.
         * @ghidraAddress NTSC-U/C: 0x00151898
         * @ghidraAddress PAL: 0x001530e0
         */
        PermBankLoader(unsigned short nSlot, const char *pszFile);

        /**
         * Release the loader.
         *
         * @ghidraAddress NTSC-U/C: 0x00151908
         * @ghidraAddress PAL: 0x00153150
         */
        ~PermBankLoader();

        /**
         * Start loading the bank.
         *
         * @ghidraAddress NTSC-U/C: 0x00151980
         * @ghidraAddress PAL: 0x001531c8
         */
        void Load();

        /**
         * Report whether the bank is loaded.
         *
         * @return False until a started load finishes.
         * @ghidraAddress NTSC-U/C: 0x001519c8
         * @ghidraAddress PAL: 0x00153210
         */
        bool IsLoaded();

        unsigned short mSlot; /*!< The synthesiser slot. */
        String mFile;         /*!< The bank file. */
        int mBlocks;          /*!< The blocks the bank needs, or -1. */
    };

    /**
     * Loader of the banks that take turns in two slots.
     *
     * The RTTI includes the name. Only the members BankLoader uses are declared.
     */
    class SwapBankLoader {
    public:
        /**
         * Construct a loader with no bank.
         *
         * @param nSlot The first of the two synthesiser slots.
         * @param pPlayMap The map of the song positions.
         * @param nIntroBars The bars before bar 0.
         * @param nNumBars The length of the song in bars.
         * @param nTicksPerBar The song ticks in one bar.
         * @ghidraAddress NTSC-U/C: 0x00154bf0
         * @ghidraAddress PAL: 0x00156458
         */
        SwapBankLoader(unsigned short nSlot,
                       PlayMap *pPlayMap,
                       int nIntroBars,
                       int nNumBars,
                       int nTicksPerBar);

        /**
         * Release the loader.
         *
         * @ghidraAddress NTSC-U/C: 0x00154d18
         * @ghidraAddress PAL: 0x00156580
         */
        ~SwapBankLoader();

        /**
         * Add a bank that plays from a bar on.
         *
         * @param pszFile The bank file.
         * @param nBar The bar.
         * @ghidraAddress NTSC-U/C: 0x00154e00
         * @ghidraAddress PAL: 0x00156668
         */
        void Add(const char *pszFile, int nBar);

        /**
         * Report the blocks the largest bank needs.
         *
         * @return The blocks.
         * @ghidraAddress NTSC-U/C: 0x001550e8
         * @ghidraAddress PAL: 0x00156950
         */
        int GetMaxBlocks();

        /**
         * Start loading the first bank.
         *
         * @ghidraAddress NTSC-U/C: 0x00155128
         * @ghidraAddress PAL: 0x00156990
         */
        void Load();

        /**
         * Report whether the first bank is loaded.
         *
         * @return Whether the load finished.
         * @ghidraAddress NTSC-U/C: 0x00155170
         * @ghidraAddress PAL: 0x001569d8
         */
        bool PollLoad();

        /**
         * Swap the banks as the song plays.
         *
         * @ghidraAddress NTSC-U/C: 0x001551d8
         * @ghidraAddress PAL: 0x00156a40
         */
        void Start();

        /**
         * Stop swapping the banks.
         *
         * @ghidraAddress NTSC-U/C: 0x001551f8
         * @ghidraAddress PAL: 0x00156a60
         */
        void Stop();

        /**
         * Withdraw the scheduled swap and swap for the song position again.
         *
         * @ghidraAddress NTSC-U/C: 0x001552b8
         * @ghidraAddress PAL: 0x00156b20
         */
        void Restart();

        unsigned short mSlot; /*!< The first of the two synthesiser slots. */
    };

    /**
     * Construct a loader with no bank.
     *
     * @param nFirstSlot The synthesiser slot of the first bank.
     * @param pPlayMap The map of the song positions.
     * @param nIntroBars The bars before bar 0.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x00149b80
     * @ghidraAddress PAL: 0x0014b540
     */
    BankLoader(unsigned short nFirstSlot,
               PlayMap *pPlayMap,
               int nIntroBars,
               int nNumBars,
               int nTicksPerBar);

    /**
     * Stop a load and release the loaders.
     *
     * @ghidraAddress NTSC-U/C: 0x00149bc0
     * @ghidraAddress PAL: 0x0014b580
     */
    ~BankLoader();

    /**
     * Add a permanent bank in the next free slot.
     *
     * @param pszFile The bank file.
     * @ghidraAddress NTSC-U/C: 0x00149c88
     * @ghidraAddress PAL: 0x0014b648
     */
    void AddPermBank(const char *pszFile);

    /**
     * Add a swapped bank that plays from a bar on.
     *
     * @param pszFile The bank file.
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00149e00
     * @ghidraAddress PAL: 0x0014b7c0
     */
    void AddSwapBank(const char *pszFile, int nBar);

    /**
     * Start loading the sound banks.
     *
     * The first load also divides the synthesiser memory among the slots and loads the permanent
     * banks.
     *
     * @ghidraAddress NTSC-U/C: 0x00149e90
     * @ghidraAddress PAL: 0x0014b850
     */
    void Load();

    /**
     * Advance the load and report whether it finished.
     *
     * @return Whether every bank is loaded.
     * @ghidraAddress NTSC-U/C: 0x00149f88
     * @ghidraAddress PAL: 0x0014b948
     */
    bool PollLoad();

    /**
     * Enter the playing state and start swapping the banks.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a028
     * @ghidraAddress PAL: 0x0014b9e8
     */
    void Start();

    /**
     * Stop a load or the swapping that is under way.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a058
     * @ghidraAddress PAL: 0x0014ba18
     */
    void Stop();

    /**
     * Swap the banks for the song position again.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a090
     * @ghidraAddress PAL: 0x0014ba50
     */
    void Restart();

    /**
     * Report the number of slots the banks fill.
     *
     * @return The permanent banks, and two more when there are swapped banks.
     * @ghidraAddress NTSC-U/C: 0x0014a0b8
     * @ghidraAddress PAL: 0x0014ba78
     */
    int GetNumSlots();

    /**
     * Report the first synthesiser bank slot after the slots the banks fill.
     *
     * @return The slot.
     * @ghidraAddress NTSC-U/C: 0x0014a0e8
     * @ghidraAddress PAL: 0x0014baa8
     */
    unsigned short GetEndBankSlot();

    int mState;                               /*!< One of State. */
    unsigned short mFirstSlot;                /*!< The slot of the first bank. */
    int mIntroBars;                           /*!< The bars before bar 0. */
    int mNumBars;                             /*!< The length of the song in bars. */
    int mTicksPerBar;                         /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;                        /*!< The map of the song positions. */
    std::vector<PermBankLoader *> mPermBanks; /*!< The permanent banks. */
    SwapBankLoader *mSwapBanks;               /*!< The swapped banks, or null. */
    int mLoadCount;                           /*!< The loads started. */

private:
    /**
     * Give each synthesiser slot of the song the memory its banks need.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a118
     * @ghidraAddress PAL: 0x0014bad8
     */
    void PartitionSlots();
};
