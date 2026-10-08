#include "synthedit/SyntheditCtl.h"

#include <stdlib.h>

#include <algorithm>
#include <map>
#include <vector>

#include <afxdlgs.h>

#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "synth/BankManager.h"
#include "synthedit/FolderDialog.h"
#include "synthedit/IntDialog.h"
#include "synthedit/Resource.h"
#include "synthedit/SyntheditPpg.h"
#include "synthedit/synthedit.h"

namespace {

// The filter of the file dialogues.
const char *const kBankFilter = "Bank Files (*.bnk)|*.bnk||";
const char *const kWavFilter = ".wav Files (*.wav)|*.wav||";

// The flags of the dialogues that pick several WAV files.
const DWORD kMultiSelectFlags =
    OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

// The bank change identifier of a bank loaded from a file, which the file replaces.
const unsigned short kLoadedBankID = 0xffff;

// The size of the buffer NewSampleDescription() gives the file dialogue for the chosen names.
const int kFileNamesBufferSize = 10000;

// MakeSimpleBankFile() adds this many instruments of this many samples each.
const int kSimpleBankInstruments = 6;

// Ranges the sheets' values must be within.
const long kMaxMidiValue = 127;
const long kMinTranspose = -127;
const long kMinFineTranspose = -100;
const long kMaxFineTranspose = 100;
const long kMaxId = 99999;
const long kMaxBusMode = 3;

// The numbers and text exchanged for one sample description.
const int kNumSampleDescLongs = 18;
const int kNumSetSampleDescLongs = 19;
const int kNumSampleDescStrings = 2;

// The numbers and text exchanged for one instrument.
const int kNumInstrumentLongs = 5;
const int kNumInstrumentStrings = 2;

// The numbers and text exchanged for one sample.
const int kNumSampleLongs = 4;
const int kNumSampleStrings = 3;

// The numbers and text exchanged for one bank.
const int kNumBankLongs = 2;

// The configuration InitSynthControl() starts the systems with.
const char *const kEditorConfig = "synth_editor_config.txt";

// Copy a value when it is within a range.
template <class T>
bool StoreIfInRange(long value, long minValue, long maxValue, T *out) {
    bool outOfRange = false;
    if (value >= minValue && value <= maxValue) {
        *out = static_cast<T>(value);
    } else {
        outOfRange = true;
    }
    return outOfRange;
}

} // namespace

// 0x1003e840
static BankManager gBankManager;

IMPLEMENT_DYNCREATE(CSyntheditCtrl, COleControl)

BEGIN_MESSAGE_MAP(CSyntheditCtrl, COleControl)
ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSyntheditCtrl, COleControl)
DISP_FUNCTION(CSyntheditCtrl, "MakeSimpleBankFile", MakeSimpleBankFile, VT_I4, VTS_NONE)
DISP_FUNCTION(
    CSyntheditCtrl, "XferSynthData", XferSynthData, VT_I4, VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT)
DISP_FUNCTION(CSyntheditCtrl, "DoDebugSomething", DoDebugSomething, VT_I4, VTS_NONE)
DISP_FUNCTION(CSyntheditCtrl, "ReadAndSaveBankFile", ReadAndSaveBankFile, VT_I4, VTS_NONE)
DISP_FUNCTION(CSyntheditCtrl, "SaveBank", SaveBank, VT_I4, VTS_I4 VTS_I4)
DISP_FUNCTION(CSyntheditCtrl, "NewSamplesFromFolder", NewSamplesFromFolder, VT_I4, VTS_I4)
DISP_FUNCTION(CSyntheditCtrl, "NewInstrumentsFromFolder", NewInstrumentsFromFolder, VT_I4, VTS_I4)
DISP_FUNCTION(
    CSyntheditCtrl, "NewSampleDescription", NewSampleDescription, VT_I4, VTS_I4 VTS_I4 VTS_I4)
DISP_FUNCTION(
    CSyntheditCtrl, "DeleteSampleDescription", DeleteSampleDescription, VT_I4, VTS_I4 VTS_I4 VTS_I4)
DISP_FUNCTION(CSyntheditCtrl, "InitSynthControl", InitSynthControl, VT_I4, VTS_BSTR)
DISP_FUNCTION_ID(CSyntheditCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

BEGIN_EVENT_MAP(CSyntheditCtrl, COleControl)
EVENT_CUSTOM("UpdateSampleComments", FireUpdateSampleComments, VTS_I4 VTS_PVARIANT)
EVENT_CUSTOM("UpdateSampleDescriptionComments",
             FireUpdateSampleDescriptionComments,
             VTS_I4 VTS_I4 VTS_PVARIANT)
EVENT_CUSTOM("UpdateSampleData", FireUpdateSampleData, VTS_I4 VTS_I4)
EVENT_CUSTOM("UpdateBankData", FireUpdateBankData, VTS_I4)
EVENT_CUSTOM("UpdateInstrumentData", FireUpdateInstrumentData, VTS_I4 VTS_I4)
EVENT_CUSTOM("UpdateSampleDescData", FireUpdateSampleDescData, VTS_I4 VTS_I4 VTS_I4)
EVENT_CUSTOM("UpdateInstrumentComments", FireUpdateInstrumentComments, VTS_I4 VTS_I4)
EVENT_CUSTOM("InsertNewSample", FireInsertNewSample, VTS_I4 VTS_I4)
EVENT_CUSTOM("ProtectWorkSheets", FireProtectWorkSheets, VTS_I4 VTS_I4)
EVENT_CUSTOM("InsertNewInstrument", FireInsertNewInstrument, VTS_I4 VTS_I4)
EVENT_CUSTOM("InsertNewSampleDescription", FireInsertNewSampleDescription, VTS_I4 VTS_I4 VTS_I4)
EVENT_CUSTOM("DeleteSampleDescription", FireDeleteSampleDescription, VTS_I4 VTS_I4 VTS_I4)
EVENT_CUSTOM("DeleteInstrument", FireDeleteInstrument, VTS_I4 VTS_I4)
EVENT_CUSTOM("DeleteSample", FireDeleteSample, VTS_I4 VTS_I4)
EVENT_CUSTOM("UpdateHyperLinks", FireUpdateHyperLinks, VTS_I4)
END_EVENT_MAP()

BEGIN_PROPPAGEIDS(CSyntheditCtrl, 1)
PROPPAGEID(CSyntheditPropPage::guid)
END_PROPPAGEIDS(CSyntheditCtrl)

IMPLEMENT_OLECREATE_EX(CSyntheditCtrl,
                       "SYNTHEDIT.SyntheditCtrl.1",
                       0x3d6cca5d,
                       0x9430,
                       0x4225,
                       0xba,
                       0x65,
                       0x92,
                       0xe3,
                       0xe1,
                       0x4d,
                       0x4b,
                       0x48)

IMPLEMENT_OLETYPELIB(CSyntheditCtrl, _tlid, _wVerMajor, _wVerMinor)

// 0x10031bc8
const IID BASED_CODE IID_DSynthedit = {
    0xcf0954d2, 0x7680, 0x4670, { 0x9d, 0x23, 0x14, 0xf0, 0xa1, 0x0a, 0xf4, 0xce }
};

// 0x10031bd8
const IID BASED_CODE IID_DSyntheditEvents = {
    0x97f09794, 0xb575, 0x479c, { 0x88, 0x63, 0x7a, 0xaf, 0xce, 0x71, 0x1f, 0x52 }
};

static const DWORD BASED_CODE _dwSyntheditOleMisc =
    OLEMISC_INVISIBLEATRUNTIME | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT | OLEMISC_CANTLINKINSIDE | OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CSyntheditCtrl, IDS_SYNTHEDIT, _dwSyntheditOleMisc)

// 0x10002588
BOOL CSyntheditCtrl::CSyntheditCtrlFactory::UpdateRegistry(BOOL bRegister) {
    if (bRegister) {
        return AfxOleRegisterControlClass(AfxGetInstanceHandle(),
                                          m_clsid,
                                          m_lpszProgID,
                                          IDS_SYNTHEDIT,
                                          IDB_SYNTHEDIT,
                                          afxRegApartmentThreading,
                                          _dwSyntheditOleMisc,
                                          _tlid,
                                          _wVerMajor,
                                          _wVerMinor);
    }
    return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

CSyntheditCtrl::CSyntheditCtrl() {
    InitializeIIDs(&IID_DSynthedit, &IID_DSyntheditEvents);
}

CSyntheditCtrl::~CSyntheditCtrl() {
    SystemTerminate();
}

void CSyntheditCtrl::OnDraw(CDC *pdc, const CRect &rcBounds, const CRect &rcInvalid) {
    pdc->FillRect(rcBounds, CBrush::FromHandle(static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH))));
    pdc->Ellipse(rcBounds);
}

void CSyntheditCtrl::DoPropExchange(CPropExchange *pPX) {
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
}

void CSyntheditCtrl::OnResetState() {
    COleControl::OnResetState();
}

void CSyntheditCtrl::AboutBox() {
    CDialog dlgAbout(IDD_ABOUTBOX_SYNTHEDIT);
    dlgAbout.DoModal();
}

long CSyntheditCtrl::SaveBank(long iBankID, long iSaveAs) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    ASSERT(bw);
    if (bw->GetFilename().size() > 0) {
        if (iSaveAs > 0) {
            // An open dialogue, not a save dialogue, is shown here.
            CFileDialog dialog(
                TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, kBankFilter);
            dialog.m_ofn.lpstrTitle = "you rock";
            if (dialog.DoModal() == IDOK) {
                bool forceAudioData = false;
                String filename(dialog.GetPathName());
                BankWin *bw = gBankManager.GetBankWin(iBankID);
                ASSERT(bw);
                retVal = bw->Save(filename, forceAudioData);
            }
        } else {
            BankWin *bw = gBankManager.GetBankWin(iBankID);
            ASSERT(bw);
            retVal = bw->Save("", false);
        }
    } else {
        CFileDialog dialog(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, kBankFilter);
        dialog.m_ofn.lpstrTitle = "Josh says 'Get to Work!'";
        if (dialog.DoModal() == IDOK) {
            bool forceAudioData = false;
            String filename(dialog.GetPathName());
            BankWin *bw = gBankManager.GetBankWin(iBankID);
            ASSERT(bw);
            retVal = bw->Save(filename, forceAudioData);
        }
    }
    return retVal;
}

long CSyntheditCtrl::NewBank() {
    long retVal = -1;
    int dialogResult = IDOK;
    int loadAnswer = MessageBoxEx(
        NULL, "Would you like to load the new bank from a file?", "Load Bank", MB_YESNO, 0);
    if (loadAnswer == IDYES) {
        CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, kBankFilter);
        dialogResult = dialog.DoModal();
        if (dialogResult == IDOK) {
            String filename(dialog.GetPathName());
            retVal = gBankManager.NewBank(filename, kLoadedBankID);
        }
    }
    if (loadAnswer == IDNO || dialogResult == IDCANCEL || retVal == -1) {
        int bankChangeID = -1;
        do {
            CIntDialog dialog(NULL);
            dialog.SetTitle("Bank Change ID");
            bankChangeID = dialog.DoModal();
            if (bankChangeID >= 0) {
                retVal = gBankManager.NewBank("", bankChangeID);
            } else {
                MessageBoxEx(NULL,
                             "Bank change IDs must be greater than or equal to 0.",
                             "Bank Change ID",
                             MB_OK,
                             0);
            }
        } while (bankChangeID < 0);
    }
    ASSERT(retVal != -1);
    return retVal;
}

long CSyntheditCtrl::AttachSampleToInstrument(long iBankID, long iSampleID, long iInstrumentID) {
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    SampleWin *sw = GetFromMap(bw->GetSamples(), iSampleID);
    InstrumentWin *iw = GetFromMap(bw->GetInstruments(), iInstrumentID);
    long retVal = iw->AttachSample(sw);
    return retVal;
}

long CSyntheditCtrl::DeleteInstrument(long iBankID, long iInstrumentID) {
    std::vector<int> sampleDescIDs;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    InstrumentWin *iw = GetFromMap(bw->GetInstruments(), iInstrumentID);
    std::map<int, SampleDescWin *> &sampleDescs = iw->GetSampleDescs();
    std::map<int, SampleDescWin *>::iterator it;
    for (it = sampleDescs.begin(); it != sampleDescs.end(); ++it) {
        ASSERT(it->second);
        SampleDescWin *sdw = it->second;
        sampleDescIDs.push_back(sdw->GetID());
    }

    std::vector<int> sampleIDs;
    BankWin *bankWin = gBankManager.GetBankWin(iBankID);
    long retVal = bankWin->DeleteInstrument(iInstrumentID, sampleIDs);
    HxLongSafeArray sampleArray(sampleIDs);
    VARIANT sampleVariant;
    sampleArray.GetVariant(&sampleVariant);

    FireProtectWorkSheets(iBankID, FALSE);
    int numSampleDescs = sampleDescIDs.size();
    for (int i = 0; i < numSampleDescs; ++i) {
        FireDeleteSampleDescription(iBankID, iInstrumentID, sampleDescIDs[i]);
    }
    FireDeleteInstrument(iBankID, iInstrumentID);
    FireUpdateSampleComments(iBankID, &sampleVariant);
    int numSamples = sampleIDs.size();
    for (int j = 0; j < numSamples; ++j) {
        int sampleID = sampleIDs[j];
        FireUpdateSampleData(iBankID, sampleID);
    }
    FireProtectWorkSheets(iBankID, TRUE);
    return retVal;
}

long CSyntheditCtrl::DeleteSample(long iBankID, long iSampleID) {
    std::map<int, int> removed;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    long retVal = bw->DeleteSample(iSampleID, removed);
    FireProtectWorkSheets(iBankID, FALSE);
    FireDeleteSample(iBankID, iSampleID);
    std::map<int, int>::iterator it;
    for (it = removed.begin(); it != removed.end(); ++it) {
        int sampleDescID = it->first;
        int instrumentID = it->second;
        FireDeleteSampleDescription(iBankID, instrumentID, sampleDescID);
        FireUpdateInstrumentComments(iBankID, instrumentID);
    }
    FireUpdateHyperLinks(iBankID);
    FireProtectWorkSheets(iBankID, TRUE);
    return retVal;
}

long CSyntheditCtrl::NewInstrument(long iBankID) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    retVal = bw->NewInstrument(0);
    FireProtectWorkSheets(iBankID, FALSE);
    FireInsertNewInstrument(iBankID, retVal);
    FireProtectWorkSheets(iBankID, TRUE);
    return retVal;
}

long CSyntheditCtrl::NewSample(long iBankID) {
    CFileDialog dialog(TRUE, NULL, NULL, kMultiSelectFlags, kWavFilter);
    dialog.m_ofn.lpstrTitle = "Chris says hi.";
    long retVal = -1;
    if (dialog.DoModal() == IDOK) {
        POSITION pos = dialog.GetStartPosition();
        FireProtectWorkSheets(iBankID, FALSE);
        while (TRUE) {
            if (pos == NULL) {
                break;
            }
            retVal = -1;
            CString path = dialog.GetNextPathName(pos);
            String filename(path);
            BankWin *bw = gBankManager.GetBankWin(iBankID);
            bool alreadyExists;
            retVal = bw->NewSample(filename, &alreadyExists);
            if (!alreadyExists) {
                FireInsertNewSample(iBankID, retVal);
            }
        }
        FireProtectWorkSheets(iBankID, TRUE);
    }
    return retVal;
}

long CSyntheditCtrl::NewSampleFromFile(long iBankID, const String &filename, bool *alreadyExists) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    retVal = bw->NewSample(filename, alreadyExists);
    if (!*alreadyExists) {
        FireProtectWorkSheets(iBankID, FALSE);
        FireInsertNewSample(iBankID, retVal);
        FireProtectWorkSheets(iBankID, TRUE);
    }
    return retVal;
}

long CSyntheditCtrl::RetireBank(long iBankID) {
    return gBankManager.RetireBank(iBankID);
}

long CSyntheditCtrl::ShutdownSynthCtrl() {
    return 0;
}

long CSyntheditCtrl::XferSynthData(long iBankID,
                                   long iAction,
                                   VARIANT FAR *ioLongArray,
                                   VARIANT FAR *ioStringArray) {
    if (ioLongArray->vt != (VT_ARRAY | VT_I4)) {
        ASSERT(false);
    }
    SAFEARRAY *longSafeArray = ioLongArray->parray;
    ASSERT(longSafeArray);
    if (SafeArrayGetDim(longSafeArray) != 1) {
        ASSERT(false);
    }
    HxLongSafeArray longs(longSafeArray);

    if (ioStringArray->vt != (VT_ARRAY | VT_BSTR)) {
        ASSERT(false);
    }
    SAFEARRAY *stringSafeArray = ioStringArray->parray;
    ASSERT(stringSafeArray);
    if (SafeArrayGetDim(stringSafeArray) != 1) {
        ASSERT(false);
    }
    HxStringSafeArray strings(stringSafeArray);

    long result = -1;
    switch (iAction) {
    case kActionGetBankData:
    case kActionGetBankSamplesIDs:
    case kActionGetBankInstrumentIDs:
    case kActionGetSampleWriteup:
    case kActionSetBankData:
        result = XferBankData(iBankID, iAction, longs, strings);
        break;
    case kActionGetSampleData:
    case kActionSetSampleData:
        result = XferSampleData(iBankID, iAction, longs, strings);
        break;
    case kActionGetInstrumentData:
    case kActionGetInstrumentSampleDescIDs:
    case kActionGetInstrumentWriteup:
    case kActionSetInstrumentData:
        result = XferInstrumentData(iBankID, iAction, longs, strings);
        break;
    case kActionGetSampleDescData:
    case kActionSetSampleDescData:
    case kActionGetReferencedSample:
        result = XferSampleDescData(iBankID, iAction, longs, strings);
        break;
    default:
        ASSERT(false);
        break;
    }
    // The arrays were never accessed, and the result is discarded.
    SafeArrayUnaccessData(longSafeArray);
    SafeArrayUnaccessData(stringSafeArray);
    return 0;
}

long CSyntheditCtrl::XferSampleDescData(long iBankID,
                                        long iAction,
                                        HxLongSafeArray &longs,
                                        HxStringSafeArray &strings) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    switch (iAction) {
    case kActionGetSampleDescData: {
        int instrumentID = longs.Get(0);
        int sampleDescID = longs.Get(1);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        SampleDescWin *sdw = GetFromMap(iw->GetSampleDescs(), sampleDescID);
        SampleDescription *desc = &sdw->GetSampleDescription();
        ASSERT(desc);
        strings.Resize(kNumSampleDescStrings);
        strings.Set(0, sdw->GetName());
        strings.Set(1, "Sample Description");

        int counter = 0;
        int numSampleDescLongs = kNumSampleDescLongs;
        longs.Resize(numSampleDescLongs);
        longs.Set(counter++, desc->mVolume);
        char pan = desc->mPan;
        if (desc->mSurround) {
            pan *= -1;
        }
        longs.Set(counter++, pan);
        longs.Set(counter++, desc->mTranspose);
        longs.Set(counter++, desc->mFineTranspose);
        longs.Set(counter++, desc->mLowKeymap);
        longs.Set(counter++, desc->mBaseKey);
        longs.Set(counter++, desc->mHighKeymap);
        longs.Set(counter++, desc->mAttackMode);
        longs.Set(counter++, desc->mAttackRate);
        longs.Set(counter++, desc->mDecayRate);
        longs.Set(counter++, desc->mSusLevel);
        longs.Set(counter++, desc->mSusMode);
        longs.Set(counter++, desc->mSusRate);
        longs.Set(counter++, desc->mReleaseMode);
        longs.Set(counter++, desc->mReleaseRate);
        longs.Set(counter++, desc->mBus);
        longs.Set(counter++, desc->mBusMode);
        longs.Set(counter++, sdw->GetSampleID());
        ASSERT(counter == numSampleDescLongs);
        retVal = 0;
        break;
    }
    case kActionSetSampleDescData: {
        int instrumentID = longs.Get(0);
        int sampleDescID = longs.Get(1);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        SampleDescWin *sdw = GetFromMap(iw->GetSampleDescs(), sampleDescID);
        SampleDescription *desc = &sdw->GetSampleDescription();
        ASSERT(desc);
        bool nameChanged = false;
        String name = strings.Get(0);
        if (name != sdw->GetName()) {
            sdw->SetName(name);
            nameChanged = true;
        }

        int counter = 2;
        bool outOfRange = false;
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mVolume);
        char pan;
        outOfRange |= StoreIfInRange(longs.Get(counter++), kMinTranspose, kMaxMidiValue, &pan);
        desc->mPan = abs(pan);
        // A negative pan marks the description surround; a positive one does not clear it.
        if (pan < 0) {
            desc->mSurround = 1;
        }
        outOfRange |=
            StoreIfInRange(longs.Get(counter++), kMinTranspose, kMaxMidiValue, &desc->mTranspose);
        outOfRange |= StoreIfInRange(
            longs.Get(counter++), kMinFineTranspose, kMaxFineTranspose, &desc->mFineTranspose);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mLowKeymap);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mBaseKey);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mHighKeymap);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mAttackMode);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mAttackRate);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mDecayRate);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mSusLevel);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mSusMode);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mSusRate);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mReleaseMode);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mReleaseRate);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxMidiValue, &desc->mBus);
        outOfRange |= StoreIfInRange(longs.Get(counter++), 0, kMaxBusMode, &desc->mBusMode);
        ASSERT(counter == kNumSetSampleDescLongs);

        FireProtectWorkSheets(iBankID, FALSE);
        if (outOfRange) {
            FireUpdateSampleDescData(iBankID, instrumentID, sampleDescID);
        }
        if (nameChanged) {
            HxLongSafeArray ids(NULL);
            ids.Resize(1);
            ids.Set(0, sampleDescID);
            VARIANT variant;
            ids.GetVariant(&variant);
            FireUpdateSampleDescriptionComments(iBankID, sdw->GetSampleID(), &variant);
            ids.Set(0, sdw->GetSampleID());
            ids.GetVariant(&variant);
            FireUpdateSampleComments(iBankID, &variant);
            FireUpdateInstrumentComments(iBankID, instrumentID);
        }
        FireProtectWorkSheets(iBankID, TRUE);
        retVal = 0;
        break;
    }
    case kActionGetReferencedSample: {
        int instrumentID = longs.Get(0);
        int sampleDescID = longs.Get(1);
        longs.Resize(1);
        strings.Resize(0);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        longs.Set(0, GetFromMap(iw->GetSampleDescs(), sampleDescID)->GetSampleID());
        break;
    }
    default:
        ASSERT(false);
        break;
    }
    return retVal;
}

long CSyntheditCtrl::XferInstrumentData(long iBankID,
                                        long iAction,
                                        HxLongSafeArray &longs,
                                        HxStringSafeArray &strings) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    switch (iAction) {
    case kActionGetInstrumentSampleDescIDs: {
        int instrumentID = longs.Get(0);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        int numSampleDescs = iw->GetSampleDescs().size();
        longs.Resize(numSampleDescs);
        strings.Resize(0);
        std::map<int, SampleDescWin *> &sampleDescs = iw->GetSampleDescs();
        std::map<int, SampleDescWin *>::iterator it;
        int counter = 0;
        for (it = sampleDescs.begin(); it != sampleDescs.end(); ++it) {
            int sampleDescID = it->first;
            longs.Set(counter++, sampleDescID);
        }
        ASSERT(counter == numSampleDescs);
        retVal = 0;
        break;
    }
    case kActionSetInstrumentData: {
        int instrumentID = longs.Get(0);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        Instrument *inst = &iw->GetInstrument();
        ASSERT(inst);
        bool outOfRange = false;
        outOfRange |= StoreIfInRange(longs.Get(1), 0, kMaxMidiValue, &inst->mVolume);
        outOfRange |= StoreIfInRange(longs.Get(2), 0, kMaxMidiValue, &inst->mPan);
        outOfRange |= StoreIfInRange(longs.Get(3), kMinTranspose, kMaxMidiValue, &inst->mTranspose);
        outOfRange |= StoreIfInRange(
            longs.Get(4), kMinFineTranspose, kMaxFineTranspose, &inst->mFineTranspose);
        outOfRange |= StoreIfInRange(longs.Get(5), 0, kMaxId, &inst->mProgram);
        String name = strings.Get(0);
        bool nameChanged = false;
        if (name != iw->GetName()) {
            nameChanged = true;
        }
        if (name.size() > 0) {
            iw->SetName(name);
        } else {
            outOfRange = true;
        }
        FireProtectWorkSheets(iBankID, FALSE);
        if (outOfRange) {
            FireUpdateInstrumentData(iBankID, instrumentID);
        }
        if (nameChanged) {
            FireUpdateInstrumentComments(iBankID, instrumentID);
        }
        FireProtectWorkSheets(iBankID, TRUE);
        break;
    }
    case kActionGetInstrumentData: {
        int instrumentID = longs.Get(0);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        Instrument *inst = &iw->GetInstrument();
        ASSERT(inst);
        strings.Resize(kNumInstrumentStrings);
        longs.Resize(kNumInstrumentLongs);
        strings.Set(0, iw->GetName());
        strings.Set(1, "Instrument");
        longs.Set(0, inst->mVolume);
        longs.Set(1, inst->mPan);
        longs.Set(2, inst->mTranspose);
        longs.Set(3, inst->mFineTranspose);
        longs.Set(4, inst->mProgram);
        retVal = 0;
        break;
    }
    case kActionGetInstrumentWriteup: {
        int instrumentID = longs.Get(0);
        InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
        std::vector<String> sampleDescNames;
        std::map<int, SampleDescWin *>::iterator it;
        for (it = iw->GetSampleDescs().begin(); it != iw->GetSampleDescs().end(); ++it) {
            SampleDescWin *sdWin = it->second;
            ASSERT(sdWin);
            sampleDescNames.push_back(sdWin->GetName());
        }
        longs.Resize(0);
        strings.Resize(1);
        String writeup(iw->GetName());
        writeup += "\n\n";
        if (sampleDescNames.size() == 0) {
            writeup += "This instrument does not use any samples.\n";
        } else {
            writeup += "This instrument contains the following::\n";
            int numNames = sampleDescNames.size();
            for (int i = 0; i < numNames; ++i) {
                writeup += sampleDescNames[i];
                writeup += "\n";
            }
        }
        strings.Set(0, writeup);
        break;
    }
    }
    return retVal;
}

long CSyntheditCtrl::XferSampleData(long iBankID,
                                    long iAction,
                                    HxLongSafeArray &longs,
                                    HxStringSafeArray &strings) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    switch (iAction) {
    case kActionGetSampleData: {
        int sampleID = longs.Get(0);
        ASSERT(sampleID >= 0);
        SampleWin *sw = GetFromMap(bw->GetSamples(), sampleID);
        strings.Resize(kNumSampleStrings);
        longs.Resize(kNumSampleLongs);
        strings.Set(0, sw->GetName());
        std::vector<String> users;
        std::vector<int> sampleDescIDs;
        bw->GetSampleUsers(users, sampleDescIDs, sampleID);
        strings.Set(1, users.size() > 0 ? "Yes" : "No");
        strings.Set(2, sw->GetAudioIO().GetFilename());
        longs.Set(0, sw->GetAudioIO().GetSampleRate());
        longs.Set(1, sw->GetAudioIO().GetVagSize());
        longs.Set(2, sw->GetSample().mLoopStart);
        longs.Set(3, sw->GetSample().mLoopStop);
        retVal = 0;
        break;
    }
    case kActionSetSampleData: {
        int sampleID = longs.Get(0);
        int loopStart = longs.Get(1);
        int loopStop = longs.Get(2);
        SampleWin *sw = GetFromMap(bw->GetSamples(), sampleID);
        ASSERT(sw);
        String name = strings.Get(0);
        sw->SetName(name);
        sw->GetSample().mLoopStart = loopStart;
        sw->GetSample().mLoopStop = loopStop;
        bw->mAudioDataDirty = true;
        FireProtectWorkSheets(iBankID, FALSE);
        {
            HxLongSafeArray sampleIDs(NULL);
            sampleIDs.Resize(1);
            sampleIDs.Set(0, sampleID);
            VARIANT sampleVariant;
            sampleIDs.GetVariant(&sampleVariant);
            FireUpdateSampleComments(iBankID, &sampleVariant);
            std::vector<String> users;
            std::vector<int> sampleDescIDs;
            bw->GetSampleUsers(users, sampleDescIDs, sampleID);
            int numSampleDescs = sampleDescIDs.size();
            HxLongSafeArray sampleDescArray(NULL);
            sampleDescArray.Resize(numSampleDescs);
            for (int i = 0; i < numSampleDescs; ++i) {
                sampleDescArray.Set(i, sampleDescIDs[i]);
            }
            VARIANT sampleDescVariant;
            sampleDescArray.GetVariant(&sampleDescVariant);
            FireUpdateSampleDescriptionComments(iBankID, sampleID, &sampleDescVariant);
            if (name.size() == 0) {
                FireUpdateSampleData(iBankID, sampleID);
            }
        }
        FireProtectWorkSheets(iBankID, FALSE); // The binary never protects the sheets again.
        break;
    }
    default:
        ASSERT(false);
        break;
    }
    return retVal;
}

long CSyntheditCtrl::XferBankData(long iBankID,
                                  long iAction,
                                  HxLongSafeArray &longs,
                                  HxStringSafeArray &strings) {
    long retVal = -1;
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    ASSERT(bw);
    switch (iAction) {
    case kActionSetBankData: {
        Bank *bank = &bw->GetBank();
        ASSERT(bank);
        bool outOfRange = false;
        outOfRange |= StoreIfInRange(longs.Get(0), 0, kMaxMidiValue, &bank->mVolume);
        outOfRange |= StoreIfInRange(longs.Get(1), 0, kMaxId, &bank->mId);
        String name = strings.Get(0);
        if (name.size() > 0) {
            bw->SetName(name);
        } else {
            outOfRange = true;
        }
        if (outOfRange) {
            FireProtectWorkSheets(iBankID, FALSE);
            FireUpdateBankData(iBankID);
            FireProtectWorkSheets(iBankID, TRUE);
        }
        break;
    }
    case kActionGetBankData: {
        Bank *bank = &bw->GetBank();
        ASSERT(bank);
        longs.Resize(kNumBankLongs);
        strings.Resize(1);
        strings.Set(0, bw->GetName());
        longs.Set(0, bank->mVolume);
        longs.Set(1, bank->mId);
        retVal = 0;
        break;
    }
    case kActionGetBankSamplesIDs: {
        int numSamples = bw->GetSamples().size();
        longs.Resize(numSamples);
        strings.Resize(0);
        std::map<int, SampleWin *> &samples = bw->GetSamples();
        std::map<int, SampleWin *>::iterator it;
        int counter = 0;
        for (it = samples.begin(); it != samples.end(); ++it) {
            int sampleID = it->first;
            longs.Set(counter++, sampleID);
        }
        retVal = 0;
        ASSERT(counter == numSamples);
        break;
    }
    case kActionGetBankInstrumentIDs: {
        int numInstruments = bw->GetInstruments().size();
        longs.Resize(numInstruments);
        strings.Resize(0);
        std::map<int, InstrumentWin *> &instruments = bw->GetInstruments();
        std::map<int, InstrumentWin *>::iterator it;
        int counter = 0;
        for (it = instruments.begin(); it != instruments.end(); ++it) {
            int instrumentID = it->first;
            longs.Set(counter++, instrumentID);
        }
        retVal = 0;
        ASSERT(counter == numInstruments);
        break;
    }
    case kActionGetSampleWriteup: {
        int sampleID = longs.Get(0);
        longs.Resize(0);
        strings.Resize(1);
        String info = bw->GetSampleInfo(static_cast<unsigned short>(sampleID));
        strings.Set(0, info);
        break;
    }
    default:
        ASSERT(false);
        break;
    }
    return retVal;
}

long CSyntheditCtrl::MakeSimpleBankFile() {
    long bankID = NewBank();
    for (int i = 0; i < kSimpleBankInstruments; ++i) {
        long instrumentID = NewInstrument(bankID);
        long sampleID = NewSample(bankID);
        AttachSampleToInstrument(bankID, sampleID, instrumentID);
        sampleID = NewSample(bankID);
        AttachSampleToInstrument(bankID, sampleID, instrumentID);
        sampleID = NewSample(bankID);
        AttachSampleToInstrument(bankID, sampleID, instrumentID);
        sampleID = NewSample(bankID);
        AttachSampleToInstrument(bankID, sampleID, instrumentID);
    }
    SaveBank(bankID, 0);
    RetireBank(bankID);
    return 0;
}

long CSyntheditCtrl::DoDebugSomething() {
    return 0;
}

long CSyntheditCtrl::ReadAndSaveBankFile() {
    long bankID = NewBank();
    SaveBank(bankID, 0);
    RetireBank(bankID);
    return 0;
}

long CSyntheditCtrl::NewSamplesFromFolder(long iBankID) {
    // 0x1003e89c
    static CString sLastFolder("C:\\");
    CFolderDialog dialog(sLastFolder, 0, NULL);
    if (dialog.DoModal() == IDOK) {
        CString folder = dialog.GetSelectedFolder();
        sLastFolder = folder;
        String path(folder);
        AddSamplesFromFolder(path, static_cast<unsigned short>(iBankID), false);
    }
    return -1;
}

void CSyntheditCtrl::AddSamplesFromFolder(const String &folder,
                                          unsigned short bankID,
                                          bool asInstrument) {
    // 0x10037388
    static const char *sAppleDoubleFolder = ".AppleDouble";
    if (folder.Find(sAppleDoubleFolder) != String::npos) {
        return;
    }
    String pattern(folder);
    pattern += "\\";
    pattern += "*";
    WIN32_FIND_DATA findData;
    HANDLE find = FindFirstFile(pattern, &findData);
    std::vector<String> subfolders;
    std::vector<String> wavFiles;
    String folderName(FileGetBase(folder));
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }

    BOOL found = TRUE;
    while (found) {
        String name(findData.cFileName);
        if (name != folderName && name != "." && name != ".." && name != sAppleDoubleFolder) {
            String path(folder);
            path += "\\";
            path += name;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                subfolders.push_back(String(path));
            } else {
                String extension(FileGetExt(path));
                if (extension == "wav") {
                    wavFiles.push_back(path);
                }
            }
        }
        found = FindNextFile(find, &findData);
    }
    FindClose(find);

    if (wavFiles.size() > 0) {
        long instrumentID = -1;
        if (asInstrument) {
            instrumentID = NewInstrument(bankID);
            BankWin *bw = gBankManager.GetBankWin(bankID);
            InstrumentWin *iw = GetFromMap(bw->GetInstruments(), instrumentID);
            iw->SetName(folderName);
        }
        std::sort(wavFiles.begin(), wavFiles.end());
        int numFiles = wavFiles.size();
        for (int i = 0; i < numFiles; ++i) {
            bool alreadyExists;
            long sampleID = NewSampleFromFile(bankID, wavFiles[i], &alreadyExists);
            if (asInstrument) {
                long sampleDescID = AttachSampleToInstrument(bankID, sampleID, instrumentID);
                FireProtectWorkSheets(bankID, FALSE);
                FireUpdateSampleData(bankID, sampleID);
                {
                    HxLongSafeArray sampleIDs(NULL);
                    sampleIDs.Resize(1);
                    sampleIDs.Set(0, sampleID);
                    VARIANT variant;
                    sampleIDs.GetVariant(&variant);
                    FireUpdateSampleComments(bankID, &variant);
                    FireInsertNewSampleDescription(bankID, instrumentID, sampleDescID);
                }
                FireProtectWorkSheets(bankID, TRUE);
            }
        }
        if (asInstrument) {
            FireProtectWorkSheets(bankID, FALSE);
            FireUpdateInstrumentComments(bankID, instrumentID);
            FireUpdateInstrumentData(bankID, instrumentID);
            FireProtectWorkSheets(bankID, TRUE);
        }
    }

    int numSubfolders = subfolders.size();
    for (int j = 0; j < numSubfolders; ++j) {
        AddSamplesFromFolder(subfolders[j], bankID, asInstrument);
    }
}

long CSyntheditCtrl::NewInstrumentsFromFolder(long iBankID) {
    // 0x1003e8a0
    static CString sLastFolder("C:\\");
    CFolderDialog dialog(sLastFolder, 0, NULL);
    if (dialog.DoModal() == IDOK) {
        CString folder = dialog.GetSelectedFolder();
        sLastFolder = folder;
        String path(folder);
        AddSamplesFromFolder(path, static_cast<unsigned short>(iBankID), true);
    }
    return -1;
}

long CSyntheditCtrl::NewSampleDescription(long iBankID,
                                          long iInstrumentID,
                                          long iSampleIDOptional) {
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    InstrumentWin *iw = GetFromMap(bw->GetInstruments(), iInstrumentID);
    if (iSampleIDOptional < 0) {
        CFileDialog dialog(TRUE, NULL, NULL, kMultiSelectFlags, kWavFilter);
        dialog.m_ofn.lpstrTitle = "Kasson says hi.";
        char fileNames[kFileNamesBufferSize];
        memset(fileNames, 0, sizeof(fileNames));
        dialog.m_ofn.lpstrFile = fileNames;
        dialog.m_ofn.nMaxFile = sizeof(fileNames);
        long sampleID = -1;
        if (dialog.DoModal() == IDOK) {
            POSITION pos = dialog.GetStartPosition();
            std::vector<CString> paths;
            while (pos != NULL) {
                paths.push_back(dialog.GetNextPathName(pos));
            }
            std::sort(paths.begin(), paths.end());
            FireProtectWorkSheets(iBankID, FALSE);
            int numPaths = paths.size();
            for (int i = 0; i < numPaths; ++i) {
                sampleID = -1;
                CString path(paths[i]);
                String filename(path);
                BankWin *bankWin = gBankManager.GetBankWin(iBankID);
                bool alreadyExists;
                sampleID = bankWin->NewSample(filename, &alreadyExists);
                if (!alreadyExists) {
                    FireInsertNewSample(iBankID, sampleID);
                }
                long sampleDescID = AttachSampleToInstrument(iBankID, sampleID, iInstrumentID);
                FireInsertNewSampleDescription(iBankID, iInstrumentID, sampleDescID);
                FireUpdateInstrumentComments(iBankID, iInstrumentID);
                HxLongSafeArray sampleIDs(NULL);
                sampleIDs.Resize(1);
                sampleIDs.Set(0, sampleID);
                VARIANT sampleVariant;
                sampleIDs.GetVariant(&sampleVariant);
                FireUpdateSampleComments(iBankID, &sampleVariant);
                FireUpdateSampleData(iBankID, sampleID);
                std::vector<String> users;
                std::vector<int> sampleDescIDs;
                bw->GetSampleUsers(users, sampleDescIDs, sampleID);
                HxLongSafeArray sampleDescArray(sampleDescIDs);
                VARIANT sampleDescVariant;
                sampleDescArray.GetVariant(&sampleDescVariant);
                FireUpdateSampleDescriptionComments(iBankID, sampleID, &sampleDescVariant);
            }
            FireProtectWorkSheets(iBankID, TRUE);
        } else if (CommDlgExtendedError() != 0) {
            MessageBoxEx(NULL,
                         "Error loading *WAY TOO MANY FILES!* Talk to Denny.",
                         "C'mon now, seriously...",
                         MB_OK,
                         0);
        }
    } else {
        FireProtectWorkSheets(iBankID, FALSE);
        BankWin *bankWin = gBankManager.GetBankWin(iBankID);
        InstrumentWin *instWin = GetFromMap(bankWin->GetInstruments(), iInstrumentID);
        SampleWin *sw = GetFromMap(bankWin->GetSamples(), iSampleIDOptional);
        long sampleDescID = instWin->AttachSample(sw);
        FireInsertNewSampleDescription(iBankID, iInstrumentID, sampleDescID);
        FireUpdateInstrumentComments(iBankID, iInstrumentID);
        {
            HxLongSafeArray sampleIDs(NULL);
            sampleIDs.Resize(1);
            sampleIDs.Set(0, iSampleIDOptional);
            VARIANT sampleVariant;
            sampleIDs.GetVariant(&sampleVariant);
            FireUpdateSampleComments(iBankID, &sampleVariant);
            FireUpdateSampleData(iBankID, iSampleIDOptional);
            std::vector<String> users;
            std::vector<int> sampleDescIDs;
            bw->GetSampleUsers(users, sampleDescIDs, iSampleIDOptional);
            HxLongSafeArray sampleDescArray(sampleDescIDs);
            VARIANT sampleDescVariant;
            sampleDescArray.GetVariant(&sampleDescVariant);
            FireUpdateSampleDescriptionComments(iBankID, iSampleIDOptional, &sampleDescVariant);
        }
        FireProtectWorkSheets(iBankID, TRUE);
    }
    return 0;
}

long CSyntheditCtrl::DeleteSampleDescription(long iBankID, long iInstrumentID, long iSampleDescID) {
    BankWin *bw = gBankManager.GetBankWin(iBankID);
    InstrumentWin *iw = GetFromMap(bw->GetInstruments(), iInstrumentID);
    SampleDescWin *sdw = GetFromMap(iw->GetSampleDescs(), iSampleDescID);
    int sampleID = sdw->GetSampleID();
    FireProtectWorkSheets(iBankID, FALSE);
    iw->DeleteSampleDescription(iSampleDescID);
    FireDeleteSampleDescription(iBankID, iInstrumentID, iSampleDescID);
    FireUpdateInstrumentComments(iBankID, iInstrumentID);
    HxLongSafeArray sampleIDs(NULL);
    sampleIDs.Resize(1);
    sampleIDs.Set(0, sampleID);
    VARIANT sampleVariant;
    sampleIDs.GetVariant(&sampleVariant);
    FireUpdateSampleData(iBankID, sampleID);
    FireUpdateSampleComments(iBankID, &sampleVariant);
    std::vector<int> sampleDescIDs;
    std::vector<String> users;
    bw->GetSampleUsers(users, sampleDescIDs, sampleID);
    HxLongSafeArray sampleDescArray(sampleDescIDs);
    VARIANT sampleDescVariant;
    sampleDescArray.GetVariant(&sampleDescVariant);
    FireUpdateSampleDescriptionComments(iBankID, sampleID, &sampleDescVariant);
    FireProtectWorkSheets(iBankID, TRUE);
    return 0;
}

long CSyntheditCtrl::InitSynthControl(LPCTSTR iSystemDirectory) {
    SystemInit(FormatString("%s/foo", iSystemDirectory), kEditorConfig);
    return 0;
}
