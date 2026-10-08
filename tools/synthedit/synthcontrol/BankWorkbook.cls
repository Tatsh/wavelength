Attribute VB_Name = "BankWorkbook"
Attribute VB_Base = "0{FCFB3D2A-A0FA-1068-A738-08002B3371B5}"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_TemplateDerived = False
Attribute VB_Customizable = False
Option Explicit

Public mSampleSheet As New SynthSheet
Public mInstrumentSheet As New SynthSheet
Public mHiddenSheet As New SynthSheet
Dim mGlobalUtils As GlobalUtils

Private WithEvents mApp As Excel.Application
Attribute mApp.VB_VarHelpID = -1
Public WithEvents mWorkbook As Excel.Workbook
Attribute mWorkbook.VB_VarHelpID = -1

Public mBankIDNumber As Long

Private gTitlePadding As String


Private Const mGroupColor As Integer = 37
Private closingRetireBank As Boolean



Public Function Init(iApp As Excel.Application, iGU As GlobalUtils, iWb As Workbook, iID As Long)
    Set mApp = iApp
    Set mWorkbook = iWb
    Set mGlobalUtils = iGU
    mBankIDNumber = iID
    
    gTitlePadding = "    "
    
    closingRetireBank = False
    
    'remove extra sheets
    Call mGlobalUtils.RemoveExtraSheets(3, mWorkbook, mApp)
    
    'make the sheets like we want them...
    SetupHiddenSheet
    SetupSampleSheet
    SetupInstrumentSheet
    
    'make the nice window split.
    'ActiveWindow.SplitColumn = 1
    'ActiveWindow.SplitRow = 1
    
    Set Init = Nothing

End Function
Private Sub SetupHiddenSheet()
    
    Dim groups(0) As String
    Dim columns(1) As SynthColumn
    Dim i As Integer
    i = 0
    
    Set columns(i) = New SynthColumn
    Call columns(i).Init("Bogus", False, False, False, 0, 0)
    i = i + 1
    
    If Not i = 1 Then
        MsgBox "You have the wrong number of columns for the Hidden Sheet."
    Else
        Call mSampleSheet.Init(mWorkbook.Worksheets(3), mGlobalUtils, True, "HiddenSheet", columns, groups, GlobalVariables.kHiddenSheetType, mBankIDNumber, Me)
    End If
    
End Sub
Private Sub SetupSampleSheet()

    'make sure that this local variable doesn't bite
    'you in the ass by being garbage collected out from underneath
    'the SynthSheet.
    Dim groups(0) As String
    Dim columns(GlobalVariables.kNumSampleSheetColumns) As SynthColumn
    
    Set columns(GlobalVariables.kSampleNameColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleNameColumnNum).Init(CStr(gTitlePadding) & "Name", False, False, False, 0, 0)
    
    Set columns(GlobalVariables.kSampleRateColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleRateColumnNum).Init(CStr(gTitlePadding) & "Sample Rate", True, False, False, 0, 0)
    
    Set columns(GlobalVariables.kSampleSizeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleSizeColumnNum).Init(CStr(gTitlePadding) & "Sample Size", True, False, True, 0, 127)
    
    'this is validated to 3 million (3 megs.) SPU is two megs, so this is fine.
    Set columns(GlobalVariables.kSampleLoopStartColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleLoopStartColumnNum).Init(CStr(gTitlePadding) & "Loop (start)", False, False, True, 0, 3000000)
    
    Set columns(GlobalVariables.kSampleLoopEndColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleLoopEndColumnNum).Init(CStr(gTitlePadding) & "Loop (end)", False, False, True, 0, 3000000)
    
    Set columns(GlobalVariables.kSampleUsedColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleUsedColumnNum).Init(CStr(gTitlePadding) & "Used?", True, False, True, 0, 0)

    Set columns(GlobalVariables.kSampleFileNameColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleFileNameColumnNum).Init(CStr(gTitlePadding) & "File Name", True, False, True, 0, 0)

    Set columns(GlobalVariables.kSampleDataTypeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleDataTypeColumnNum).Init(CStr(gTitlePadding) & "DataType", True, True, False, 0, 0, 5)
    
    Set columns(GlobalVariables.kSampleDataIDColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kSampleDataIDColumnNum).Init(CStr(gTitlePadding) & "DataID", True, True, False, 0, 0, 5)
    
    Call mSampleSheet.Init(mWorkbook.Worksheets(2), mGlobalUtils, False, "Samples", columns, groups, GlobalVariables.kSampleSheetType, mBankIDNumber, Me)
    
End Sub

Private Sub SetupInstrumentSheet()
    
    'make sure that this local variable doesn't bite
    'you in the ass by being garbage collected out from underneath
    'the SynthSheet.
    
    Dim columns(GlobalVariables.kNumInstrumentSheetColumns) As SynthColumn
    
    Set columns(GlobalVariables.kInstrumentNameColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentNameColumnNum).Init(CStr(gTitlePadding) & "Name", False, False, False, 0, 0)
    
    Set columns(GlobalVariables.kInstrumentBasicColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentBasicColumnNum).Init(CStr(gTitlePadding) & "Basic", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentVolumeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentVolumeColumnNum).Init(CStr(gTitlePadding) & "Volume", False, False, True, 0, 127)
    
    Set columns(GlobalVariables.kInstrumentPanColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentPanColumnNum).Init(CStr(gTitlePadding) & "Pan", False, False, True, -127, 127)
    
    Set columns(GlobalVariables.kInstrumentTransposeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentTransposeColumnNum).Init(CStr(gTitlePadding) & "Transpose", False, False, True, -127, 127)
    
    Set columns(GlobalVariables.kInstrumentTransposeFineColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentTransposeFineColumnNum).Init(CStr(gTitlePadding) & "Transpose (fine)", False, False, True, -100, 100)
    
    Set columns(GlobalVariables.kInstrumentIDColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentIDColumnNum).Init(CStr(gTitlePadding) & "ID", False, False, True, 0, 65535)
    
    Set columns(GlobalVariables.kInstrumentInfoColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentInfoColumnNum).Init(CStr(gTitlePadding) & "Info", False, True, False, 0, 0)
    
    Set columns(GlobalVariables.kInstrumentKeymapColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapColumnNum).Init(CStr(gTitlePadding) & "Keymap", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentKeymapLowColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapLowColumnNum).Init(CStr(gTitlePadding) & "Keymap (low)", False, False, True, 0, 127)
    
    'this shows the note representation of the previous column
    Set columns(GlobalVariables.kInstrumentKeymapLowSpacerColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapLowSpacerColumnNum).Init(CStr(gTitlePadding) & "", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentKeymapBaseColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapBaseColumnNum).Init(CStr(gTitlePadding) & "Keymap (base)", False, False, True, 0, 127)
    
    'this shows the note representation of the previous column
    Set columns(GlobalVariables.kInstrumentKeymapBaseSpacerColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapBaseSpacerColumnNum).Init(CStr(gTitlePadding) & "", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentKeymapHighColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapHighColumnNum).Init(CStr(gTitlePadding) & "Keymap (high)", False, False, True, 0, 127)
    
    'this shows the note representation of the previous column
    Set columns(GlobalVariables.kInstrumentKeymapHighSpacerColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentKeymapHighSpacerColumnNum).Init(CStr(gTitlePadding) & "", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentADSRColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentADSRColumnNum).Init(CStr(gTitlePadding) & "ADSR", True, False, False, 0, 0, mGroupColor)
    
    'this should be from a custom list (linear, exp,etc...)
    Set columns(GlobalVariables.kInstrumentAttackModeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentAttackModeColumnNum).Init(CStr(gTitlePadding) & "Attack (mode)", False, False, True, 0, 1)
    
    Set columns(GlobalVariables.kInstrumentAttackRateColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentAttackRateColumnNum).Init(CStr(gTitlePadding) & "Attack (rate)", False, False, True, 0, 127)
    
    Set columns(GlobalVariables.kInstrumentDecayRateColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentDecayRateColumnNum).Init(CStr(gTitlePadding) & "Decay (rate)", False, False, True, 0, 15)
    
    Set columns(GlobalVariables.kInstrumentSustainLevelColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentSustainLevelColumnNum).Init(CStr(gTitlePadding) & "Sustain (level)", False, False, True, 0, 15)
        
    Set columns(GlobalVariables.kInstrumentSustainModeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentSustainModeColumnNum).Init(CStr(gTitlePadding) & "Sustain (mode)", False, False, True, 0, 3)
    
    Set columns(GlobalVariables.kInstrumentSustainRateColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentSustainRateColumnNum).Init(CStr(gTitlePadding) & "Sustain (rate)", False, False, True, 0, 127)
    
    Set columns(GlobalVariables.kInstrumentReleaseModeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentReleaseModeColumnNum).Init(CStr(gTitlePadding) & "Release (mode)", False, False, True, 0, 1)
    
    Set columns(GlobalVariables.kInstrumentReleaseRateColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentReleaseRateColumnNum).Init(CStr(gTitlePadding) & "Release (rate)", False, False, True, 0, 127)
    
    Set columns(GlobalVariables.kInstrumentBusGroupColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentBusGroupColumnNum).Init(CStr(gTitlePadding) & "FX", True, False, False, 0, 0, mGroupColor)
    
    Set columns(GlobalVariables.kInstrumentBusColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentBusColumnNum).Init(CStr(gTitlePadding) & "Bus", False, False, True, 0, 127)
    
    Set columns(GlobalVariables.kInstrumentBusModeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentBusModeColumnNum).Init(CStr(gTitlePadding) & "Bus Mode", False, False, True, 0, 3)
    
    
    Set columns(GlobalVariables.kInstrumentDataTypeColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentDataTypeColumnNum).Init(CStr(gTitlePadding) & "DataType", True, True, False, 0, 0, 5)
    
    Set columns(GlobalVariables.kInstrumentDataIDColumnNum) = New SynthColumn
    Call columns(GlobalVariables.kInstrumentDataIDColumnNum).Init(CStr(gTitlePadding) & "DataID", True, True, False, 0, 0, 5)
    
    
    'pick your groups
    Dim groups(4) As String
    groups(0) = GlobalVariables.kInstrumentSheetGroup0
    groups(1) = GlobalVariables.kInstrumentSheetGroup1
    groups(2) = GlobalVariables.kInstrumentSheetGroup2
    groups(3) = GlobalVariables.kInstrumentSheetGroup3
    

    Call mInstrumentSheet.Init(mWorkbook.Worksheets(1), mGlobalUtils, False, "Instruments", columns, groups, GlobalVariables.kInstrumentSheetType, mBankIDNumber, Me)

    
End Sub



Private Sub mWorkbook_Activate()
    Set mGlobalUtils.gCurrentBankWorkbook = Me
End Sub

Private Sub mWorkbook_BeforeClose(Cancel As Boolean)
    closingRetireBank = True
End Sub

Private Sub mWorkbook_BeforeSave(ByVal SaveAsUI As Boolean, Cancel As Boolean)
    If SaveAsUI Then
        Call mGlobalUtils.TheSynthControl.SaveBank(mBankIDNumber, 0)
    Else
        Call mGlobalUtils.TheSynthControl.SaveBank(mBankIDNumber, 0)
    End If
    
    Cancel = True
End Sub

Private Sub mWorkbook_Deactivate()
    If closingRetireBank Then
        Call mGlobalUtils.TheSynthControl.RetireBank(mBankIDNumber)
        Call mGlobalUtils.RemoveBankWorkbook(mBankIDNumber)
    End If
End Sub
