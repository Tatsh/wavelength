#pragma once

#include <afxctl.h>

#include "synthedit/HxLongSafeArray.h"
#include "synthedit/HxStringSafeArray.h"
#include "utl/Str.h"

/**
 * ActiveX control that edits synthesiser banks for the workbook add-in.
 *
 * The control is invisible at run time. The add-in calls its methods and keeps its worksheets in
 * step through its events. The class name is from the runtime class information; the names of the
 * methods and events are from the type library.
 */
class CSyntheditCtrl : public COleControl {
    DECLARE_DYNCREATE(CSyntheditCtrl)

public:
    /** @ghidraAddress 0x100025e8 */
    CSyntheditCtrl();

    /**
     * Fill the bounds with white and draw an ellipse.
     *
     * @param pdc The device context.
     * @param rcBounds The bounds of the control.
     * @param rcInvalid The area to redraw.
     * @ghidraAddress 0x10002694
     */
    virtual void OnDraw(CDC *pdc, const CRect &rcBounds, const CRect &rcInvalid);

    /**
     * Exchange the persistent properties, of which there are none besides the version.
     *
     * @param pPX The exchange.
     * @ghidraAddress 0x100026d2
     */
    virtual void DoPropExchange(CPropExchange *pPX);

    /** @ghidraAddress 0x10002710 */
    virtual void OnResetState();

    /** Dispatch identifiers of the methods, as the type library numbers them. */
    enum {
        dispidNewBank = 1L,                          /*!< NewBank(). */
        dispidAttachSampleToInstrument = 2L,         /*!< AttachSampleToInstrument(). */
        dispidDeleteInstrument = 3L,                 /*!< DeleteInstrument(). */
        dispidDeleteSample = 4L,                     /*!< DeleteSample(). */
        dispidNewInstrument = 5L,                    /*!< NewInstrument(). */
        dispidNewSample = 6L,                        /*!< NewSample(). */
        dispidRetireBank = 7L,                       /*!< RetireBank(). */
        dispidShutdownSynthCtrl = 8L,                /*!< ShutdownSynthCtrl(). */
        dispidMakeSimpleBankFile = 9L,               /*!< MakeSimpleBankFile(). */
        dispidXferSynthData = 10L,                   /*!< XferSynthData(). */
        dispidDoDebugSomething = 11L,                /*!< DoDebugSomething(). */
        dispidReadAndSaveBankFile = 12L,             /*!< ReadAndSaveBankFile(). */
        dispidSaveBank = 13L,                        /*!< SaveBank(). */
        dispidNewSamplesFromFolder = 14L,            /*!< NewSamplesFromFolder(). */
        dispidNewInstrumentsFromFolder = 15L,        /*!< NewInstrumentsFromFolder(). */
        dispidNewSampleDescription = 16L,            /*!< NewSampleDescription(). */
        dispidDeleteSampleDescription = 17L,         /*!< DeleteSampleDescription(). */
        dispidInitSynthControl = 18L,                /*!< InitSynthControl(). */
        eventidUpdateSampleComments = 1L,            /*!< FireUpdateSampleComments(). */
        eventidUpdateSampleDescriptionComments = 2L, /*!< FireUpdateSampleDescriptionComments(). */
        eventidUpdateSampleData = 3L,                /*!< FireUpdateSampleData(). */
        eventidUpdateBankData = 4L,                  /*!< FireUpdateBankData(). */
        eventidUpdateInstrumentData = 5L,            /*!< FireUpdateInstrumentData(). */
        eventidUpdateSampleDescData = 6L,            /*!< FireUpdateSampleDescData(). */
        eventidUpdateInstrumentComments = 7L,        /*!< FireUpdateInstrumentComments(). */
        eventidInsertNewSample = 8L,                 /*!< FireInsertNewSample(). */
        eventidProtectWorkSheets = 9L,               /*!< FireProtectWorkSheets(). */
        eventidInsertNewInstrument = 10L,            /*!< FireInsertNewInstrument(). */
        eventidInsertNewSampleDescription = 11L,     /*!< FireInsertNewSampleDescription(). */
        eventidDeleteSampleDescription = 12L,        /*!< FireDeleteSampleDescription(). */
        eventidDeleteInstrument = 13L,               /*!< FireDeleteInstrument(). */
        eventidDeleteSample = 14L,                   /*!< FireDeleteSample(). */
        eventidUpdateHyperLinks = 15L,               /*!< FireUpdateHyperLinks(). */
    };

    /** The data XferSynthData() reads or writes, as the add-in numbers it. */
    enum XferAction {
        kActionGetBankData = 0,                /*!< Bank volume, identifier, and name. */
        kActionGetInstrumentData = 1,          /*!< Instrument settings and name. */
        kActionGetSampleDescData = 2,          /*!< Sample description settings and name. */
        kActionGetSampleData = 3,              /*!< Sample loop points, file, and users. */
        kActionGetBankSamplesIDs = 4,          /*!< Identifiers of the bank's samples. */
        kActionGetBankInstrumentIDs = 5,       /*!< Identifiers of the bank's instruments. */
        kActionGetInstrumentSampleDescIDs = 6, /*!< Identifiers of an instrument's descriptions. */
        kActionGetSampleWriteup = 7,           /*!< Text about a sample. */
        kActionGetInstrumentWriteup = 8,       /*!< Text about an instrument. */
        kActionSetBankData = 9,                /*!< Bank volume, identifier, and name. */
        kActionSetInstrumentData = 10,         /*!< Instrument settings and name. */
        kActionSetSampleDescData = 11,         /*!< Sample description settings and name. */
        kActionSetSampleData = 12,             /*!< Sample loop points and name. */
        kActionGetReferencedSample = 13,       /*!< The sample a description plays. */
    };

protected:
    /**
     * Stop the systems InitSynthControl() started.
     *
     * @ghidraAddress 0x10002646
     */
    ~CSyntheditCtrl();

    DECLARE_OLECREATE_EX(CSyntheditCtrl)
    DECLARE_OLETYPELIB(CSyntheditCtrl)
    DECLARE_PROPPAGEIDS(CSyntheditCtrl)
    DECLARE_OLECTLTYPE(CSyntheditCtrl)

    DECLARE_MESSAGE_MAP()

    /**
     * Create a bank, loading it from a file the user picks or prompting for its bank change
     * identifier. The prompt repeats until the identifier is not negative.
     *
     * @return The bank.
     * @ghidraAddress 0x10002ac7
     */
    afx_msg long NewBank();

    /**
     * Add a description that plays a sample to an instrument.
     *
     * @param iBankID The bank.
     * @param iSampleID The sample.
     * @param iInstrumentID The instrument.
     * @return The description.
     * @ghidraAddress 0x10002cf7
     */
    afx_msg long AttachSampleToInstrument(long iBankID, long iSampleID, long iInstrumentID);

    /**
     * Delete an instrument and its descriptions, and update the worksheets.
     *
     * @param iBankID The bank.
     * @param iInstrumentID The instrument.
     * @return The result of BankWin::DeleteInstrument().
     * @ghidraAddress 0x10002d59
     */
    afx_msg long DeleteInstrument(long iBankID, long iInstrumentID);

    /**
     * Delete a sample and the descriptions that play it, and update the worksheets.
     *
     * @param iBankID The bank.
     * @param iSampleID The sample.
     * @return The result of BankWin::DeleteSample().
     * @ghidraAddress 0x10003025
     */
    afx_msg long DeleteSample(long iBankID, long iSampleID);

    /**
     * Add an instrument with program zero.
     *
     * @param iBankID The bank.
     * @return The instrument.
     * @ghidraAddress 0x10003158
     */
    afx_msg long NewInstrument(long iBankID);

    /**
     * Add the samples of the WAV files the user picks.
     *
     * @param iBankID The bank.
     * @return The last sample, or -1 when none was added.
     * @ghidraAddress 0x100031bb
     */
    afx_msg long NewSample(long iBankID);

    /**
     * Close a bank.
     *
     * @param iBankID The bank.
     * @return The result of BankManager::RetireBank().
     * @ghidraAddress 0x100033da
     */
    afx_msg long RetireBank(long iBankID);

    /**
     * Do nothing.
     *
     * @return Zero.
     * @ghidraAddress 0x100033f5
     */
    afx_msg long ShutdownSynthCtrl();

    /**
     * Create a bank of six instruments of four samples each, which the user picks, save it, and
     * close it.
     *
     * @return Zero.
     * @ghidraAddress 0x1000541a
     */
    afx_msg long MakeSimpleBankFile();

    /**
     * Read or write the data of a bank, an instrument, a sample description, or a sample.
     *
     * @param iBankID The bank.
     * @param iAction An XferAction.
     * @param ioLongArray A one-dimensional array of `VT_I4` for the numbers.
     * @param ioStringArray A one-dimensional array of `VT_BSTR` for the text.
     * @return Zero. The result of the transfer is discarded.
     * @ghidraAddress 0x10003402
     */
    afx_msg long
    XferSynthData(long iBankID, long iAction, VARIANT FAR *ioLongArray, VARIANT FAR *ioStringArray);

    /**
     * Do nothing.
     *
     * @return Zero.
     * @ghidraAddress 0x1000550a
     */
    afx_msg long DoDebugSomething();

    /**
     * Create a bank, save it, and close it.
     *
     * @return Zero.
     * @ghidraAddress 0x10005517
     */
    afx_msg long ReadAndSaveBankFile();

    /**
     * Save a bank, prompting for the file when it has none or when requested.
     *
     * @param iBankID The bank.
     * @param iSaveAs Positive to prompt for the file even when the bank has one.
     * @return The result of BankWin::Save(), or -1 when nothing was saved.
     * @ghidraAddress 0x10002779
     */
    afx_msg long SaveBank(long iBankID, long iSaveAs);

    /**
     * Add the WAV files of a folder the user picks, and of its subfolders, as samples.
     *
     * @param iBankID The bank.
     * @return -1.
     * @ghidraAddress 0x1000554b
     */
    afx_msg long NewSamplesFromFolder(long iBankID);

    /**
     * Add an instrument for each folder, the one the user picks and its subfolders, that has WAV
     * files, playing those files.
     *
     * @param iBankID The bank.
     * @return -1.
     * @ghidraAddress 0x10005cdb
     */
    afx_msg long NewInstrumentsFromFolder(long iBankID);

    /**
     * Add descriptions to an instrument for a sample, or for the WAV files the user picks.
     *
     * @param iBankID The bank.
     * @param iInstrumentID The instrument.
     * @param iSampleIDOptional The sample, or a negative value to prompt for files.
     * @return Zero.
     * @ghidraAddress 0x10005e2d
     */
    afx_msg long NewSampleDescription(long iBankID, long iInstrumentID, long iSampleIDOptional);

    /**
     * Delete a description from an instrument.
     *
     * @param iBankID The bank.
     * @param iInstrumentID The instrument.
     * @param iSampleDescID The description.
     * @return Zero.
     * @ghidraAddress 0x10006554
     */
    afx_msg long DeleteSampleDescription(long iBankID, long iInstrumentID, long iSampleDescID);

    /**
     * Start the systems with the editor configuration.
     *
     * @param iSystemDirectory The directory of `synth_editor_config.txt`.
     * @return Zero.
     * @ghidraAddress 0x1000675f
     */
    afx_msg long InitSynthControl(LPCTSTR iSystemDirectory);

    DECLARE_DISPATCH_MAP()

    /**
     * Show the about box.
     *
     * @ghidraAddress 0x10002723
     */
    afx_msg void AboutBox();

    /** @ghidraAddress 0x10006ab0 */
    void FireUpdateSampleComments(long iBankID, VARIANT FAR *iSampleIDs) {
        FireEvent(
            eventidUpdateSampleComments, EVENT_PARAM(VTS_I4 VTS_PVARIANT), iBankID, iSampleIDs);
    }

    /** @ghidraAddress 0x10006cf0 */
    void
    FireUpdateSampleDescriptionComments(long iBankID, long iSampleID, VARIANT FAR *iSampleDescIDs) {
        FireEvent(eventidUpdateSampleDescriptionComments,
                  EVENT_PARAM(VTS_I4 VTS_I4 VTS_PVARIANT),
                  iBankID,
                  iSampleID,
                  iSampleDescIDs);
    }

    /** @ghidraAddress 0x10006ae0 */
    void FireUpdateSampleData(long iBankID, long iSampleID) {
        FireEvent(eventidUpdateSampleData, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iSampleID);
    }

    /** @ghidraAddress 0x10006e70 */
    void FireUpdateBankData(long iBankID) {
        FireEvent(eventidUpdateBankData, EVENT_PARAM(VTS_I4), iBankID);
    }

    /** @ghidraAddress 0x10006d80 */
    void FireUpdateInstrumentData(long iBankID, long iInstrumentID) {
        FireEvent(eventidUpdateInstrumentData, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iInstrumentID);
    }

    /** @ghidraAddress 0x10006d20 */
    void FireUpdateSampleDescData(long iBankID, long iInstrumentID, long iSampleDescID) {
        FireEvent(eventidUpdateSampleDescData,
                  EVENT_PARAM(VTS_I4 VTS_I4 VTS_I4),
                  iBankID,
                  iInstrumentID,
                  iSampleDescID);
    }

    /** @ghidraAddress 0x10006bc0 */
    void FireUpdateInstrumentComments(long iBankID, long iInstrumentID) {
        FireEvent(
            eventidUpdateInstrumentComments, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iInstrumentID);
    }

    /** @ghidraAddress 0x10006cc0 */
    void FireInsertNewSample(long iBankID, long iSampleID) {
        FireEvent(eventidInsertNewSample, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iSampleID);
    }

    /** @ghidraAddress 0x10006b10 */
    void FireProtectWorkSheets(long iBankID, long iYesNo) {
        FireEvent(eventidProtectWorkSheets, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iYesNo);
    }

    /** @ghidraAddress 0x10006c70 */
    void FireInsertNewInstrument(long iBankID, long iInstrumentID) {
        FireEvent(eventidInsertNewInstrument, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iInstrumentID);
    }

    /** @ghidraAddress 0x10006ec0 */
    void FireInsertNewSampleDescription(long iBankID, long iInstrumentID, long iSampleDescID) {
        FireEvent(eventidInsertNewSampleDescription,
                  EVENT_PARAM(VTS_I4 VTS_I4 VTS_I4),
                  iBankID,
                  iInstrumentID,
                  iSampleDescID);
    }

    /** @ghidraAddress 0x10006b40 */
    void FireDeleteSampleDescription(long iBankID, long iInstrumentID, long iSampleDescID) {
        FireEvent(eventidDeleteSampleDescription,
                  EVENT_PARAM(VTS_I4 VTS_I4 VTS_I4),
                  iBankID,
                  iInstrumentID,
                  iSampleDescID);
    }

    /** @ghidraAddress 0x10006b70 */
    void FireDeleteInstrument(long iBankID, long iInstrumentID) {
        FireEvent(eventidDeleteInstrument, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iInstrumentID);
    }

    /** @ghidraAddress 0x10006bf0 */
    void FireDeleteSample(long iBankID, long iSampleID) {
        FireEvent(eventidDeleteSample, EVENT_PARAM(VTS_I4 VTS_I4), iBankID, iSampleID);
    }

    /** @ghidraAddress 0x10006c20 */
    void FireUpdateHyperLinks(long iBankID) {
        FireEvent(eventidUpdateHyperLinks, EVENT_PARAM(VTS_I4), iBankID);
    }

    DECLARE_EVENT_MAP()

private:
    /**
     * Add a sample for a WAV file and report it when it is new.
     *
     * @param iBankID The bank.
     * @param filename The WAV file.
     * @param alreadyExists Receives whether the bank already had the file.
     * @return The sample.
     * @ghidraAddress 0x10003366
     */
    long NewSampleFromFile(long iBankID, const String &filename, bool *alreadyExists);

    /**
     * Add the WAV files of a folder, sorted, as samples, then visit its subfolders. A folder whose
     * path has `.AppleDouble` is skipped.
     *
     * @param folder The folder.
     * @param bankID The bank.
     * @param asInstrument Whether to add an instrument named after the folder that plays its
     * files.
     * @ghidraAddress 0x1000569d
     */
    void AddSamplesFromFolder(const String &folder, unsigned short bankID, bool asInstrument);

    /**
     * Transfer the data of a sample description: kActionGetSampleDescData,
     * kActionSetSampleDescData, or kActionGetReferencedSample.
     *
     * @param iBankID The bank.
     * @param iAction The XferAction.
     * @param longs The numbers.
     * @param strings The text.
     * @return Zero for kActionGetSampleDescData and kActionSetSampleDescData, otherwise -1.
     * @ghidraAddress 0x100036d2
     */
    long XferSampleDescData(long iBankID,
                            long iAction,
                            HxLongSafeArray &longs,
                            HxStringSafeArray &strings);

    /**
     * Transfer the data of an instrument: kActionGetInstrumentData,
     * kActionGetInstrumentSampleDescIDs, kActionGetInstrumentWriteup, or
     * kActionSetInstrumentData.
     *
     * @param iBankID The bank.
     * @param iAction The XferAction.
     * @param longs The numbers.
     * @param strings The text.
     * @return Zero for the first two actions, otherwise -1.
     * @ghidraAddress 0x1000431e
     */
    long XferInstrumentData(long iBankID,
                            long iAction,
                            HxLongSafeArray &longs,
                            HxStringSafeArray &strings);

    /**
     * Transfer the data of a sample: kActionGetSampleData or kActionSetSampleData.
     *
     * @param iBankID The bank.
     * @param iAction The XferAction.
     * @param longs The numbers.
     * @param strings The text.
     * @return Zero for kActionGetSampleData, otherwise -1.
     * @ghidraAddress 0x10004a06
     */
    long
    XferSampleData(long iBankID, long iAction, HxLongSafeArray &longs, HxStringSafeArray &strings);

    /**
     * Transfer the data of a bank: kActionGetBankData, kActionGetBankSamplesIDs,
     * kActionGetBankInstrumentIDs, kActionGetSampleWriteup, or kActionSetBankData.
     *
     * @param iBankID The bank.
     * @param iAction The XferAction.
     * @param longs The numbers.
     * @param strings The text.
     * @return Zero for the first three actions, otherwise -1.
     * @ghidraAddress 0x10004f56
     */
    long
    XferBankData(long iBankID, long iAction, HxLongSafeArray &longs, HxStringSafeArray &strings);
};
