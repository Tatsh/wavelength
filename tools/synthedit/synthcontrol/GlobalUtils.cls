Attribute VB_Name = "GlobalUtils"
Attribute VB_Base = "0{FCFB3D2A-A0FA-1068-A738-08002B3371B5}"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_TemplateDerived = False
Attribute VB_Customizable = False
Option Explicit

'The App, current Workbook, and Current Sheet
Private WithEvents mApp As Excel.Application
Attribute mApp.VB_VarHelpID = -1

'The Synth
Public WithEvents TheSynthControl As Synthedit
Attribute TheSynthControl.VB_VarHelpID = -1


'The Buttons
Public mLinkADSRValues As Excel.XlFormControl
Attribute mLinkADSRValues.VB_VarHelpID = -1
Public WithEvents mMidiPortComboBox As Office.CommandBarComboBox
Attribute mMidiPortComboBox.VB_VarHelpID = -1
Const mMidiPortComboBoxName As String = "Midi Devices"
Const mCommandBarName As String = "Synth Midi Thru"

Private mBankWorkbooks As Scripting.Dictionary
'Private mNumBankWorkbooksSoFar As Integer

Public gCurrentBankWorkbook As BankWorkbook

Private Declare Function GetKeyState32 Lib "user32" Alias "GetKeyState" (ByVal vKey As Integer) As Integer

Const kFirstRow As String = "1:1"

Private Sub mApp_NewWorkbook(ByVal wb As Workbook)
    Dim bankID As Long
    bankID = TheSynthControl.NewBank
    'ReDim Preserve mBankWorkbooks(mNumBankWorkbooksSoFar + 1)
    Dim bwb As BankWorkbook
    Set bwb = New BankWorkbook
    Call bwb.Init(mApp, Me, wb, bankID)
    Call mBankWorkbooks.Add(bankID, bwb)
    'mNumBankWorkbooksSoFar = mNumBankWorkbooksSoFar + 1
End Sub



Public Sub InitGlobals(app As Excel.Application)
    Dim ad As AddIn
    Dim str As String
    For Each ad In Application.AddIns
        If ad.Name = "SynthControl.xla" Then
            str = ad.Path
        End If
    Next
    
    Set mApp = app
    'mNumBankWorkbooksSoFar = 0
    Set mBankWorkbooks = New Scripting.Dictionary
    Set TheSynthControl = New Synthedit
    Call TheSynthControl.InitSynthControl(str)
    Application.EnableAnimations = False
    Set MenuFunctions.mGlobalUtils = Me
    AddCommandBar
End Sub

Public Sub ShutdownGlobals()
    RemoveCommandBar
    TheSynthControl.ShutdownSynthCtrl
End Sub

Sub RemoveCommandBar()
    'Application.CommandBars(mCommandBarName).Delete
End Sub

Sub AddCommandBar()
    Call MenuFunctions.CreatePopUp(mApp)
End Sub

Public Sub SetupFirstRow(iWkSht As Excel.Worksheet)
    Dim fr As Range
    Set fr = iWkSht.Range(kFirstRow)
    With fr.Font
        .Name = "Arial CE"
        .FontStyle = "Bold"
        .size = 12
        .Strikethrough = False
        .Superscript = False
        .Subscript = False
        .OutlineFont = False
        .Shadow = False
        .Underline = xlUnderlineStyleNone
        .ColorIndex = 37
    End With
    
    'spin things so they face up.
    With fr
        .HorizontalAlignment = xlCenter
        .VerticalAlignment = xlBottom
        .WrapText = False
        .Orientation = 90
        .AddIndent = False
        .ShrinkToFit = False
        .MergeCells = False
    End With
    
End Sub
Public Sub SetupColumn(wksht As Excel.Worksheet, sCol As SynthColumn, pos As Integer)
    Dim fr As Range
    Dim myCol As Range
    Dim myRng As Range
    
    Set fr = wksht.Range(kFirstRow)
    
    'this is a hack. how do I do columns[i]?
    Set myCol = wksht.Range("A:A").Offset(0, pos)
    Set myRng = Application.Intersect(myCol, fr)
    myRng.Value = sCol.mTitle
    myCol.Font.ColorIndex = sCol.mColorIndex
    
    'check if you need to validate input.
    If pos = 0 Then
        'validate input in the name columns.
        With myCol.Validation
            .Delete
            .Add Type:=xlValidateTextLength, AlertStyle:=xlValidAlertStop, _
            Operator:=xlGreater, Formula1:="0"
            .IgnoreBlank = False
            .InCellDropdown = True
            .InputTitle = ""
            .ErrorTitle = "Invalid Entry"
            .InputMessage = ""
            .ErrorMessage = "Text input must not be blank."
            .ShowInput = False
            .ShowError = True
        End With
    
    Else
       If sCol.mValidated Then
    
           Dim messageText As String
           messageText = "You must input a value between " & sCol.mLowerBound
           messageText = messageText & " and "
           messageText = messageText & sCol.mUpperBound
           messageText = messageText & "."
       
           With myCol.Validation
               .Delete
               .Add Type:=xlValidateWholeNumber, AlertStyle:=xlValidAlertStop, _
               Operator:=xlBetween, Formula1:=sCol.mLowerBound, Formula2:=sCol.mUpperBound
               .IgnoreBlank = True
               .InCellDropdown = True
               .InputTitle = ""
               .ErrorTitle = "Input value out of range."
               .InputMessage = ""
               .ErrorMessage = messageText
               .ShowInput = True
               .ShowError = True
           End With
       End If
    End If
    
    
    
    'protect the individual columns
    Call ProtectRange(myCol, sCol.mProtected)

    
    'always protect the first row...
    Call ProtectRange(fr, True)

End Sub
Sub ColorWkShtBG(wksht As Excel.Worksheet)

    With wksht.Cells
        .Borders(xlDiagonalDown).LineStyle = xlNone
        .Borders(xlDiagonalUp).LineStyle = xlNone
        .Borders(xlEdgeLeft).LineStyle = xlNone
        .Borders(xlEdgeTop).LineStyle = xlNone
        .Borders(xlEdgeBottom).LineStyle = xlNone
        .Borders(xlEdgeRight).LineStyle = xlNone
        With .Borders(xlInsideVertical)
            .LineStyle = xlContinuous
            .Weight = xlThin
            .ColorIndex = 12
        End With
        With .Borders(xlInsideHorizontal)
            .LineStyle = xlContinuous
            .Weight = xlThin
            .ColorIndex = 11
        End With
        With .Interior
            .ColorIndex = 1
            .Pattern = xlSolid
            .PatternColorIndex = xlAutomatic
        End With
        With .Font
            .ColorIndex = 4
        End With
    End With
End Sub

Sub CleanUpColumns(wksht As Excel.Worksheet)
    wksht.columns.AutoFit
End Sub

Sub ProtectWorksheet(wksht As Excel.Worksheet, yesNo As Boolean)

    If yesNo Then
        Call CleanUpColumns(wksht)
        wksht.EnableOutlining = True
        wksht.EnableAutoFilter = True
        wksht.EnableCalculation = True
        Call wksht.Protect("harmonix", True, True, True, True)
        Application.ScreenUpdating = True
    Else
        Application.ScreenUpdating = False
        Call wksht.Protect("harmonix", False, False, False, False)
    End If
End Sub

Sub ProtectRange(rng As Range, yesNo As Boolean)
    rng.Locked = yesNo
    rng.FormulaHidden = yesNo
End Sub

Public Sub RemoveExtraSheets(maxSheets As Integer, iWkBk As Excel.Workbook, iApp As Excel.Application)
    Dim count
    Dim i
    Dim wksht
    
    count = iWkBk.Worksheets.count
    If count > 3 Then
        iApp.DisplayAlerts = False
        For i = maxSheets + 1 To count
            Set wksht = iWkBk.Worksheets(i)
            wksht.Delete
        Next i
        iApp.DisplayAlerts = True
    End If
End Sub

Public Sub GroupRange(iWkSht As Excel.Worksheet, iRng As String)
    With iWkSht.Outline
        .AutomaticStyles = False
        .SummaryRow = xlAbove
        .SummaryColumn = xlLeft
    End With
    
    Dim rng As Range
    Set rng = iWkSht.Range(iRng)
    rng.columns.Group
End Sub

Public Sub TestVariant()
    Call TheSynthControl.DoDebugSomething
End Sub


Private Function GetBankWorkbook(iID As Long) As BankWorkbook
    Dim retVal As BankWorkbook
    Set retVal = mBankWorkbooks.Item(iID)
    Set GetBankWorkbook = retVal
End Function



Private Sub TheSynthControl_DeleteInstrument(ByVal iBankID As Long, ByVal iInstrumentID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.DeleteInstrument(iInstrumentID)
End Sub

Private Sub TheSynthControl_DeleteSample(ByVal iBankID As Long, ByVal iSampleID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mSampleSheet.DeleteSample(iSampleID)
End Sub

Private Sub TheSynthControl_DeleteSampleDescription(ByVal iBankID As Long, ByVal iInstrumentID As Long, ByVal iSampleDescID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.DeleteSampleDesc(iInstrumentID, iSampleDescID)
End Sub

Private Sub TheSynthControl_InsertNewInstrument(ByVal iBankID As Long, ByVal iInstrumentID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.InsertNewInstrument(iInstrumentID)
End Sub

Private Sub TheSynthControl_InsertNewSample(ByVal iBankID As Long, ByVal iSampleID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mSampleSheet.InsertNewSample(iSampleID)
End Sub



Private Sub TheSynthControl_InsertNewSampleDescription(ByVal iBankID As Long, ByVal iInstrumentID As Long, ByVal iSampleDescID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.InsertNewSampleDescription(iSampleDescID, iInstrumentID)
End Sub

Private Sub TheSynthControl_ProtectWorkSheets(ByVal iBankID As Long, ByVal iYesNo As Long)
    Dim yesNo As Boolean
    If iYesNo = 0 Then
        yesNo = False
    Else
        yesNo = True
    End If
    
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call ProtectWorksheet(bankWB.mSampleSheet.mWorkSheet, yesNo)
    Call ProtectWorksheet(bankWB.mInstrumentSheet.mWorkSheet, yesNo)
    
End Sub

Private Sub TheSynthControl_UpdateBankData(ByVal iBankID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.UpdateBankData
End Sub

Private Sub TheSynthControl_UpdateHyperLinks(ByVal iBankID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.UpdateHyperlinks
End Sub

Private Sub TheSynthControl_UpdateInstrumentComments(ByVal iBankID As Long, ByVal iInstrumentID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.UpdateInstrumentComments(iInstrumentID)
End Sub

Private Sub TheSynthControl_UpdateInstrumentData(ByVal iBankID As Long, ByVal iInstrumentID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.UpdateInstrumentData(iInstrumentID)
End Sub

Private Sub TheSynthControl_UpdateSampleComments(ByVal iBankID As Long, iSampleIDs As Variant)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mSampleSheet.UpdateSampleComments(iSampleIDs)
    
End Sub


Private Sub TheSynthControl_UpdateSampleData(ByVal iBankID As Long, ByVal iSampleID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mSampleSheet.UpdateSampleData(iSampleID)
End Sub



Private Sub TheSynthControl_UpdateSampleDescData(ByVal iBankID As Long, ByVal iInstrumentID As Long, ByVal iSampleDescID As Long)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    
    Call bankWB.mInstrumentSheet.UpdateSampleDescData(iInstrumentID, iSampleDescID)
End Sub

Private Sub TheSynthControl_UpdateSampleDescriptionComments(ByVal iBankID As Long, ByVal iSampleID As Long, iSampleDescIDs As Variant)
    Dim bankWB As BankWorkbook
    Set bankWB = GetBankWorkbook(iBankID)
    Call bankWB.mInstrumentSheet.UpdateSampleDescriptionComments(iSampleID, iSampleDescIDs)
End Sub


Public Sub RemoveBankWorkbook(iBankIDNumber As Long)
    mBankWorkbooks.Remove (iBankIDNumber)
End Sub



Public Function ShiftDown()
   'This returns TRUE if shift is down
   ShiftDown = (GetKeyState32(16) < 0)
End Function

Public Function CtrlDown()
   'This returns TRUE if ctrl is down
   CtrlDown = (GetKeyState32(17) < 0)
End Function

