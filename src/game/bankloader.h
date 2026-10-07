#pragma once

#include <vector>

#include "game/playmap.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/string.h"

/**
 * Loader of the sound banks of a song, from its track named "BANK".
 *
 * The class is not polymorphic. The RTTI of the nested BankLoader::PermBankLoader and
 * BankLoader::SwapBankLoader records the name. A permanent bank stays in its synthesiser slot for
 * the whole song. The swapped banks take turns in two slots after the permanent banks, each from
 * the bar the track gives.
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
     * Loader of one permanent bank, bound to a synthesiser bank slot.
     *
     * The RTTI includes the nested name. The class is not polymorphic.
     */
    class PermBankLoader {
    public:
        /**
         * Construct a loader and query the size of the bank from the synthesiser.
         *
         * @param nSlot The synthesiser bank slot.
         * @param pszFile The bank file.
         * @ghidraAddress NTSC-U/C: 0x00151898
         * @ghidraAddress PAL: 0x001530e0
         */
        PermBankLoader(unsigned short nSlot, const char *pszFile);

        /**
         * Unload the bank when Load() was called.
         *
         * @ghidraAddress NTSC-U/C: 0x00151908
         * @ghidraAddress PAL: 0x00153150
         */
        ~PermBankLoader();

        /**
         * Start loading the bank into its slot.
         *
         * @ghidraAddress NTSC-U/C: 0x00151980
         * @ghidraAddress PAL: 0x001531c8
         */
        void Load();

        /**
         * Report whether the bank finished loading.
         *
         * @return Whether Load() was called and the synthesiser reports the slot loaded.
         * @ghidraAddress NTSC-U/C: 0x001519c8
         * @ghidraAddress PAL: 0x00153210
         */
        bool IsLoaded() const;

        unsigned short mSlot; /*!< The synthesiser bank slot. */
        String mFile;         /*!< The bank file. */
        int mBlockCount;      /*!< The size the synthesiser reports for the file, or -1. */
        int mLoading;         /*!< Whether Load() was called. */
    };

    /**
     * Loader that swaps the banks of a song through two synthesiser slots as the song moves on.
     *
     * The RTTI includes the nested name and the nested SwapBankLoader::SwapBank. The class is not
     * polymorphic. Each bar of the song has a bank. While one slot plays, the loader loads the next
     * different bank into the other slot ahead of the bar that needs it.
     */
    class SwapBankLoader {
    public:
        /** One bank file and its size. */
        struct SwapBank {
            String mFile; /*!< The bank file. */
            int mSize;    /*!< The size the synthesiser reports for the file. */
        };

        /** Values of mState. */
        enum State {
            kStateIdle = 0,    /*!< Nothing is loading. */
            kStateLoading = 1, /*!< The first bank loads. */
            kStateLoaded = 2,  /*!< The first bank is loaded. */
        };

        /**
         * Construct a loader with no banks.
         *
         * @param nBaseSlot The first of the two synthesiser slots.
         * @param pPlayMap The play map of the song.
         * @param nLeadBars The bars before bar 0 that have a bank.
         * @param nNumBars The bars of the song.
         * @param nTicksPerBar The song ticks in one bar.
         * @ghidraAddress NTSC-U/C: 0x00154bf0
         * @ghidraAddress PAL: 0x00156458
         */
        SwapBankLoader(unsigned short nBaseSlot,
                       PlayMap *pPlayMap,
                       int nLeadBars,
                       int nNumBars,
                       int nTicksPerBar);

        /**
         * Unload the banks.
         *
         * @ghidraAddress NTSC-U/C: 0x00154d18
         * @ghidraAddress PAL: 0x00156580
         */
        ~SwapBankLoader();

        /**
         * Add a bank that plays from a bar to the end of the song.
         *
         * @param pszFile The bank file.
         * @param nStartBar The first bar.
         * @ghidraAddress NTSC-U/C: 0x00154e00
         * @ghidraAddress PAL: 0x00156668
         */
        void AddBank(const char *pszFile, int nStartBar);

        /**
         * Report the size of the largest bank.
         *
         * @return The size, or 0 without banks.
         * @ghidraAddress NTSC-U/C: 0x001550e8
         * @ghidraAddress PAL: 0x00156950
         */
        int GetMaxBankSize() const;

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
         * @return Whether it is loaded, and true unless it is loading.
         * @ghidraAddress NTSC-U/C: 0x00155170
         * @ghidraAddress PAL: 0x001569d8
         */
        bool PollLoad();

        /**
         * Load the bank the song needs next.
         *
         * @ghidraAddress NTSC-U/C: 0x001551d8
         * @ghidraAddress PAL: 0x00156a40
         */
        void Start();

        /**
         * Withdraw the scheduled load and unload the slots.
         *
         * @ghidraAddress NTSC-U/C: 0x001551f8
         * @ghidraAddress PAL: 0x00156a60
         */
        void Unload();

        /**
         * Withdraw the scheduled load and load the bank the song needs next.
         *
         * @ghidraAddress NTSC-U/C: 0x001552b8
         * @ghidraAddress PAL: 0x00156b20
         */
        void Restart();

    private:
        friend class BankLoader;

        /**
         * Report the bank of a bar.
         *
         * @param nBar The bar, from -mLeadBars.
         * @return The index into mBanks, or -1.
         * @ghidraAddress NTSC-U/C: 0x0034cca8
         * @ghidraAddress PAL: 0x003ba0d8
         */
        int GetBank(int nBar) const {
            return mBankOfBar[nBar + mLeadBars];
        }

        /**
         * Set the bank of a bar.
         *
         * @param nBar The bar, from -mLeadBars.
         * @param nBank The index into mBanks.
         * @ghidraAddress NTSC-U/C: 0x0034ccc8
         * @ghidraAddress PAL: 0x003ba0f8
         */
        void SetBank(int nBar, int nBank) {
            mBankOfBar[nBar + mLeadBars] = nBank;
        }

        /**
         * Load the next bank that differs from the bank of the current bar, and schedule this
         * routine again.
         *
         * @ghidraAddress NTSC-U/C: 0x00155308
         * @ghidraAddress PAL: 0x00156b70
         */
        void Advance();

        /**
         * Report the slot the next load goes to.
         *
         * @return The slot.
         * @ghidraAddress NTSC-U/C: 0x001554e0
         * @ghidraAddress PAL: 0x00156d48
         */
        unsigned short GetLoadSlot() const;

        /**
         * Load a bank into the next slot unless a slot already has it.
         *
         * @param nBank The index into mBanks.
         * @ghidraAddress NTSC-U/C: 0x00155508
         * @ghidraAddress PAL: 0x00156d70
         */
        void LoadBank(int nBank);

        /** Number of slots the loader alternates between. */
        static constexpr int kNumSlots = 2;

        unsigned short mBaseSlot;       /*!< The first of the two synthesiser slots. */
        const char *mLoaded[kNumSlots]; /*!< The text of the file in each slot, or null. */
        int mLeadBars;                  /*!< The bars before bar 0 that have a bank. */
        int mNumBars;                   /*!< The bars of the song. */
        int mTicksPerBar;               /*!< The song ticks in one bar. */
        PlayMap *mPlayMap;              /*!< The play map of the song. */
        std::vector<SwapBank> mBanks;   /*!< The banks. */
        std::vector<int> mBankOfBar;    /*!< The bank of each bar from -mLeadBars, or -1. */
        int mState;                     /*!< One of State. */
        int mLoadCount;                 /*!< The loads since the slots were empty. */
        Ptr<Command> mAdvanceCmd;       /*!< The command that calls Advance(). */
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
