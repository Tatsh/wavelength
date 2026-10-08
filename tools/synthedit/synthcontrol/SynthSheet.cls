Attribute VB_Name = "SynthSheet"
Attribute VB_Base = "0{FCFB3D2A-A0FA-1068-A738-08002B3371B5}"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_TemplateDerived = False
Attribute VB_Customizable = False
Option Explicit

Private mNumColumns As Integer
Private mColumns() As SynthColumn
Private mGlobalUtils As GlobalUtils

Private mSheetType As Long
Private mBankIDNumber As Long

Public WithEvents mWorkSheet As Excel.Worksheet
Attribute mWorkSheet.VB_VarHelpID = -1

Private mDataTypeColumn As New SynthColumn
Private mDataIDColumn As New SynthColumn
Dim mBankWorkbook As BankWorkbook

Private mRespondToChanges As Boolean

'stores the location of the data rows
'indexed of the sheet type enumerations.
'these will be long:long pairs of object ID keyed against row # (the sheet is implicit in the type.)
Private gRow2IDDictionaries(2) As Scripting.Dictionary
Private gID2RowDictionaries(2) As Scripting.Dictionary
Private gSampleDesc2Instrument As Scripting.Dictionary
Private gMenuSampleIndices As Scripting.Dictionary 'maps index to ID
Private mCurrentSortColumn As Long

Public Function GetIDForMenuIndex(iIndex As Long) As Long
    Dim val As Long
    val = gMenuSampleIndices.Item(iIndex)
    GetIDForMenuIndex = val
End Function

Public Function GetRow2IDDictionary() As Scripting.Dictionary
    Set GetRow2IDDictionary = gRow2IDDictionaries(mSheetType)
End Function

Public Function GetSampleDesc2InstrumentDictionary() As Scripting.Dictionary
    Set GetSampleDesc2InstrumentDictionary = gSampleDesc2Instrument
End Function

Public Function Init(iWkSht As Excel.Worksheet, iGU As GlobalUtils, iHid As Boolean, iTitle As String, iColumns() As SynthColumn, iGroups() As String, iSheetType As Long, iBankIDNumber As Long, iBankWorkbook As BankWorkbook)
    
    'set up some local variables...
    Set mWorkSheet = iWkSht
    Set mGlobalUtils = iGU
    mColumns = iColumns
    mNumColumns = UBound(mColumns)
    mSheetType = iSheetType
    Set gRow2IDDictionaries(mSheetType) = New Scripting.Dictionary
    Set gID2RowDictionaries(mSheetType) = New Scripting.Dictionary
    Set gSampleDesc2Instrument = New Scripting.Dictionary
    Set gMenuSampleIndices = New Scripting.Dictionary
    mBankIDNumber = iBankIDNumber
    Set mBankWorkbook = iBankWorkbook
    mRespondToChanges = False
    mCurrentSortColumn = 1
    
    'trim the extra pages off...
    Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mWorkSheet.Name = iTitle
    If iHid Then
        mWorkSheet.Visible = xlSheetHidden
    End If
    
    'setup basic colors and such...
    Call mGlobalUtils.SetupFirstRow(mWorkSheet)
    Call mGlobalUtils.ColorWkShtBG(mWorkSheet)
    
    'set up the columns...
    Dim counter As Integer
    For counter = 0 To mNumColumns - 1
        Call mGlobalUtils.SetupColumn(mWorkSheet, mColumns(counter), counter)
    Next counter
    
    
    'set up the auto filtering.
    mWorkSheet.columns.AutoFilter
    
    'make the sheet look right...
    Call mGlobalUtils.CleanUpColumns(mWorkSheet)
    
    'make the groups
    Dim i
    Dim numGroups As Integer
    numGroups = UBound(iGroups)
    For i = 0 To numGroups - 1
        Call mGlobalUtils.GroupRange(mWorkSheet, iGroups(i))
    Next i
    
    
    
    'update the synth data..
    UpdateSynthData
    
    
    'allow some special protection to use outlining.
    Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
    
    'now we start listening.
    mRespondToChanges = True
    
    'set the return value.
    Set Init = Nothing
    
End Function
Public Sub SortSamples(iColumn As Long, iAscending As Boolean)
    mRespondToChanges = False
    Dim rng As Range
    Dim sortOrder As Long
    If iAscending Then
        sortOrder = xlAscending
    Else
        sortOrder = xlDescending
    End If
    
    If iColumn = -1 Then
        iColumn = mCurrentSortColumn
    End If
    
    mCurrentSortColumn = iColumn
    
    Set rng = GetSamplesRange()
    If Not rng Is Nothing Then
        rng.Sort Key1:=rng.Range("A1").Offset(0, mCurrentSortColumn - 1), Order1:=sortOrder, Header:=xlNo, _
            OrderCustom:=1, MatchCase:=False, Orientation:=xlTopToBottom
        Call RescanIDs(gRow2IDDictionaries(mSheetType).count + 1)
        Call mBankWorkbook.mInstrumentSheet.UpdateHyperlinks
    End If
    mRespondToChanges = True
End Sub
Public Sub SortInstrument(iInstrumentID As Long, iColumn As Long, iAscending As Boolean)
    mRespondToChanges = False
    Dim rng As Range
    Dim sortOrder As Long
    If iAscending Then
        sortOrder = xlAscending
    Else
        sortOrder = xlDescending
    End If
    
    If iColumn = -1 Then
     iColumn = mCurrentSortColumn
    End If
    
    mCurrentSortColumn = iColumn
    
    Set rng = GetInstrumentSampleDescsRange(iInstrumentID)
    If Not rng Is Nothing Then
        rng.Sort Key1:=rng.Range("A1").Offset(0, mCurrentSortColumn - 1), Order1:=sortOrder, Header:=xlNo, _
            OrderCustom:=1, MatchCase:=False, Orientation:=xlTopToBottom
        Call RescanIDs(gRow2IDDictionaries(mSheetType).count + 1)
    End If
    mRespondToChanges = True
End Sub
Public Sub UpdateHyperlinks()
    Dim hyperLink As String
    Dim sampleDescLongArr As Variant
    Dim sampleDescStringArr As Variant
    Dim instrumentID As Long
    Dim reffedSample As Long
    
    Dim numItems As Long
    numItems = gRow2IDDictionaries(mSheetType).count
    
    Dim i As Long
    Dim dataType As Long
    Dim dataID As Long
    For i = 2 To numItems + 2
        dataType = mWorkSheet.Cells(i, kInstrumentDataTypeColumnNum + 1).Value
        dataID = mWorkSheet.Cells(i, kInstrumentDataIDColumnNum + 1).Value
        
        If dataType = kSampleDescDataType Then
            ReDim sampleDescLongArr(2) As Long
            ReDim sampleDescStringArr(0) As String
    
            instrumentID = gSampleDesc2Instrument.Item(dataID)
            
            sampleDescLongArr(0) = instrumentID
            sampleDescLongArr(1) = dataID
            
            Call mGlobalUtils.TheSynthControl.XferSynthData( _
                mBankIDNumber, _
                GlobalVariables.kActionGetReferencedSample, _
                sampleDescLongArr, _
                sampleDescStringArr)
                
            reffedSample = sampleDescLongArr(0)
            hyperLink = FindSampleLinkCell(reffedSample)
            
            mWorkSheet.Hyperlinks.Add Anchor:=mWorkSheet.Cells(i, kInstrumentInfoColumnNum + 1), _
                              Address:="", _
                              SubAddress:=hyperLink
        End If
    Next i
    
    
End Sub

Public Sub DeleteSample(iSampleID)
    mRespondToChanges = False
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iSampleID)
        Call mWorkSheet.Range("1:1").Offset(row - 1).Delete(xlUp)
        Dim numItems As Long
        numItems = gRow2IDDictionaries(mSheetType).count
        RescanIDs (numItems)
        
        'this will break all the hyper links.
    mRespondToChanges = True
End Sub


Public Sub DeleteInstrument(iInstrumentID As Long)
    mRespondToChanges = False
        Call gSampleDesc2Instrument.Remove(iInstrumentID)
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iInstrumentID)
        Call mWorkSheet.Range("1:1").Offset(row - 1).Delete(xlUp)
        Dim numItems As Long
        numItems = gRow2IDDictionaries(mSheetType).count
        RescanIDs (numItems)
    mRespondToChanges = True
End Sub

Public Sub DeleteSampleDesc(iInstrumentID As Long, iSampleDescID As Long)
    mRespondToChanges = False
        Call gSampleDesc2Instrument.Remove(iSampleDescID)
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iSampleDescID)
        Call mWorkSheet.Range("1:1").Offset(row - 1).Delete(xlUp)
        Dim numItems As Long
        numItems = gRow2IDDictionaries(mSheetType).count
        RescanIDs (numItems)
    mRespondToChanges = True
End Sub

Public Sub InsertNewSample(iSampleID As Long)
    Dim row As Long
    Dim rowVar As Variant
    Dim highest As Long
    highest = -1

    If gRow2IDDictionaries(mSheetType).count = 0 Then
        highest = 1
    Else
        For Each rowVar In gRow2IDDictionaries(mSheetType).Keys
            row = rowVar
            If row > highest Then
                highest = row
            End If
        Next rowVar
    End If
    
    'now, highest has the highest row.
    highest = highest + 1  'jump to the new row
    Call gRow2IDDictionaries(mSheetType).Add(highest, iSampleID)
    Call gID2RowDictionaries(mSheetType).Add(iSampleID, highest)
    
    'update the sample
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    Call mWorkSheet.Cells(highest, kSampleNameColumnNum + 1).AddComment
    UpdateSampleData (iSampleID)
    
    Dim ids As Variant
    ReDim ids(0) As Long
    ids(0) = iSampleID
    UpdateSampleComments (ids)
    
    'Call SortSamples(-1, True)
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
    
End Sub

Public Function GetSamplesRange() As Range
    Dim rng As Range
    Dim highest As Long
    Dim row As Long
    Dim rowVar As Variant
        
    If gRow2IDDictionaries(mSheetType).count = 0 Then
        highest = 2
    Else
        For Each rowVar In gRow2IDDictionaries(mSheetType).Keys
            row = rowVar
            If row > highest Then
                highest = row
            End If
        Next rowVar
    End If
    
    'the range of samples goes from 2->highest.
    Dim str As String
    str = "2:" & CStr(highest)
    Set rng = mWorkSheet.Range(str)
    
    Set GetSamplesRange = rng
End Function
Public Sub UpdateSampleDescData(iInstrumentID As Long, iSampleDescID As Long)
    
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
        
        Dim retVal As Long
        Dim sampleDescDataLongArr As Variant
        Dim sampleDescDataStringArr As Variant
        ReDim sampleDescDataLongArr(2) As Long
        ReDim sampleDescDataStringArr(0) As String
        
        'arguments.
        sampleDescDataLongArr(0) = iInstrumentID     'instrument #
        sampleDescDataLongArr(1) = iSampleDescID    'sample desc #
        
        
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                        mBankIDNumber, _
                        GlobalVariables.kActionGetSampleDescData, _
                        sampleDescDataLongArr, _
                        sampleDescDataStringArr)
                        
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iSampleDescID)
        
        
        'stick the data in
        mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value = sampleDescDataStringArr(0)
        mWorkSheet.Cells(row, kInstrumentInfoColumnNum + 1).Value = sampleDescDataStringArr(1)
        
        mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value = sampleDescDataLongArr(0)
        mWorkSheet.Cells(row, kInstrumentPanColumnNum + 1).Value = sampleDescDataLongArr(1)
        mWorkSheet.Cells(row, kInstrumentTransposeColumnNum + 1).Value = sampleDescDataLongArr(2)
        mWorkSheet.Cells(row, kInstrumentTransposeFineColumnNum + 1).Value = sampleDescDataLongArr(3)
        'no ID for sample descs.
        mWorkSheet.Cells(row, kInstrumentKeymapLowColumnNum + 1).Value = sampleDescDataLongArr(4)
        mWorkSheet.Cells(row, kInstrumentKeymapBaseColumnNum + 1).Value = sampleDescDataLongArr(5)
        mWorkSheet.Cells(row, kInstrumentKeymapHighColumnNum + 1).Value = sampleDescDataLongArr(6)
    
        
        mWorkSheet.Cells(row, kInstrumentAttackModeColumnNum + 1).Value = sampleDescDataLongArr(7)
        mWorkSheet.Cells(row, kInstrumentAttackRateColumnNum + 1).Value = sampleDescDataLongArr(8)
        mWorkSheet.Cells(row, kInstrumentDecayRateColumnNum + 1).Value = sampleDescDataLongArr(9)
        mWorkSheet.Cells(row, kInstrumentSustainLevelColumnNum + 1).Value = sampleDescDataLongArr(10)
        mWorkSheet.Cells(row, kInstrumentSustainModeColumnNum + 1).Value = sampleDescDataLongArr(11)
        mWorkSheet.Cells(row, kInstrumentSustainRateColumnNum + 1).Value = sampleDescDataLongArr(12)
        mWorkSheet.Cells(row, kInstrumentReleaseModeColumnNum + 1).Value = sampleDescDataLongArr(13)
        mWorkSheet.Cells(row, kInstrumentReleaseRateColumnNum + 1).Value = sampleDescDataLongArr(14)
        mWorkSheet.Cells(row, kInstrumentBusColumnNum + 1).Value = sampleDescDataLongArr(15)
        mWorkSheet.Cells(row, kInstrumentBusModeColumnNum + 1).Value = sampleDescDataLongArr(16)
    
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
    
End Sub
Public Sub UpdateInstrumentData(iInstrumentID As Long)
    
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
        Dim retVal As Long
        Dim instrumentDataLongArr As Variant
        Dim instrumentDataStringArr As Variant
        ReDim instrumentDataLongArr(1) As Long
        ReDim instrumentDataStringArr(0) As String
        
        'set the argument.
        instrumentDataLongArr(0) = iInstrumentID
    
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetInstrumentData, _
                                instrumentDataLongArr, _
                                instrumentDataStringArr)
        
    
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iInstrumentID)
        
        mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value = instrumentDataStringArr(0)
        mWorkSheet.Cells(row, kInstrumentInfoColumnNum + 1).Value = instrumentDataStringArr(1)
        mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value = instrumentDataLongArr(0)
        mWorkSheet.Cells(row, kInstrumentPanColumnNum + 1).Value = instrumentDataLongArr(1)
        mWorkSheet.Cells(row, kInstrumentTransposeColumnNum + 1).Value = instrumentDataLongArr(2)
        mWorkSheet.Cells(row, kInstrumentTransposeFineColumnNum + 1).Value = instrumentDataLongArr(3)
        mWorkSheet.Cells(row, kInstrumentIDColumnNum + 1).Value = instrumentDataLongArr(4)

    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
End Sub
Public Sub UpdateBankData()
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
        Dim retVal As Long
        Dim bankDataLongArr As Variant
        Dim bankDataStringArr As Variant
        ReDim bankDataLongArr(0) As Long
        ReDim bankDataStringArr(0) As String
        
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetBankData, _
                                bankDataLongArr, bankDataStringArr)
                                
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(mBankIDNumber)
                                
        mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value = bankDataStringArr(0)
        mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value = bankDataLongArr(0)
        mWorkSheet.Cells(row, kInstrumentIDColumnNum + 1).Value = bankDataLongArr(1)

    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
End Sub
Public Sub UpdateSampleData(iSampleID As Long)

    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
        Dim sampleLongData As Variant
        Dim sampleStringData As Variant
        ReDim sampleLongData(1) As Long
        ReDim sampleStringData(0) As String
        sampleLongData(0) = iSampleID  'sending the sample id argument.
        
        
        Dim retVal As Long
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetSampleData, _
                                        sampleLongData, _
                                        sampleStringData)
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iSampleID)
                
        'the data starts on row '2' because the top row is a title row.
        mWorkSheet.Cells(row, kSampleNameColumnNum + 1).Value = sampleStringData(0)
        mWorkSheet.Cells(row, kSampleRateColumnNum + 1).Value = sampleLongData(0)
        mWorkSheet.Cells(row, kSampleSizeColumnNum + 1).Value = sampleLongData(1)
        mWorkSheet.Cells(row, kSampleLoopStartColumnNum + 1).Value = sampleLongData(2)
        mWorkSheet.Cells(row, kSampleLoopEndColumnNum + 1).Value = sampleLongData(3)
        mWorkSheet.Cells(row, kSampleUsedColumnNum + 1).Value = sampleStringData(1)
        mWorkSheet.Cells(row, kSampleFileNameColumnNum + 1).Value = sampleStringData(2)
        'my super-secret data columns
        mWorkSheet.Cells(row, kSampleDataTypeColumnNum + 1).Value = kSampleDataType
        mWorkSheet.Cells(row, kSampleDataIDColumnNum + 1).Value = iSampleID
    
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
End Sub
Public Sub UpdateSampleDescriptionComments(iSampleID As Long, iSampleDescIDs As Variant)
    Dim retVal As Long
    Dim id As Long
    Dim row As Long
    Dim size As Long
    Dim i As Long
    
    'get the instrument comment
    Dim instrumentCommentLongData As Variant
    Dim instrumentCommentStringData As Variant
               
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
    
        size = UBound(iSampleDescIDs)
        
        For i = 0 To size
            id = iSampleDescIDs(i)
            row = gID2RowDictionaries(mSheetType).Item(id)
            
            ReDim instrumentCommentLongData(1) As Long
            ReDim instrumentCommentStringData(0) As String
            instrumentCommentLongData(0) = iSampleID
            
            retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                    mBankIDNumber, _
                                    GlobalVariables.kActionGetSampleWriteup, _
                                    instrumentCommentLongData, _
                                    instrumentCommentStringData)
                                    
            'comment the sample.
            Dim instCommStr As String
            instCommStr = instrumentCommentStringData(0)
            'the +1 is for 1-based indexing.
            Dim cellRng As Range
            Set cellRng = mWorkSheet.Cells(row, kInstrumentInfoColumnNum + 1)
            Call CommentRange(cellRng, instCommStr)
    
                                
        Next i
        
        Call mGlobalUtils.CleanUpColumns(mWorkSheet)
    
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
    
    
End Sub
Public Sub UpdateInstrumentComments(iInstrumentID As Long)
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
        'get the instrument comment
        Dim retVal As Long
        Dim instrumentCommentLongData As Variant
        Dim instrumentCommentStringData As Variant
        ReDim instrumentCommentLongData(1) As Long
        ReDim instrumentCommentStringData(0) As String
        instrumentCommentLongData(0) = iInstrumentID
        
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetInstrumentWriteup, _
                                instrumentCommentLongData, _
                                instrumentCommentStringData)
                    
        'comment the instrument.
        Dim row As Long
        row = gID2RowDictionaries(mSheetType).Item(iInstrumentID)
        Dim instCommStr As String
        instCommStr = instrumentCommentStringData(0)
        Call CommentRange(mWorkSheet.Cells(row, kInstrumentInfoColumnNum + 1), instCommStr)
        
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
End Sub
Public Sub UpdateSampleComments(iSampleIDs As Variant)
    Dim retVal As Long
    Dim id As Long
    Dim row As Long
    Dim size As Long
    Dim i As Long
    
    'get the instrument comment
    Dim instrumentCommentLongData As Variant
    Dim instrumentCommentStringData As Variant
               
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
    mRespondToChanges = False
    
        size = UBound(iSampleIDs)
        
        For i = 0 To size
            id = iSampleIDs(i)
            row = gID2RowDictionaries(mSheetType).Item(id)
            
            ReDim instrumentCommentLongData(1) As Long
            ReDim instrumentCommentStringData(0) As String
            instrumentCommentLongData(0) = id
            
            retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                    mBankIDNumber, _
                                    GlobalVariables.kActionGetSampleWriteup, _
                                    instrumentCommentLongData, _
                                    instrumentCommentStringData)
                                    
            'comment the sample.
            Dim instCommStr As String
            instCommStr = instrumentCommentStringData(0)
            'the +1 is for 1-based indexing.
            Dim cellRng As Range
            Set cellRng = mWorkSheet.Cells(row, kSampleNameColumnNum + 1)
            Call CommentRange(cellRng, instCommStr)
    
                                
        Next i
        
        'Call mGlobalUtils.CleanUpColumns(mWorkSheet, kNumSampleSheetColumns)
    
    mRespondToChanges = True
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
    
    
End Sub
Private Sub SendSampleData(iRow As Long)
    Dim sampleLongData As Variant
    Dim sampleStringData As Variant
    ReDim sampleLongData(3) As Long
    ReDim sampleStringData(1) As String
    Dim retVal As Long
    Dim sampleName As String
    
    'we get the sampleID value from the 'row' key.
    Dim sampleID As Long
    sampleID = gRow2IDDictionaries(mSheetType).Item(iRow)
                    
    'get the sample name, it's the only data we're updating.\
    'the +1 is for 1-based indexing. yuck.
    sampleName = mWorkSheet.Cells(iRow, kSampleNameColumnNum + 1).Value
    
    'put the data into the arrays.
    sampleLongData(0) = sampleID
    sampleLongData(1) = mWorkSheet.Cells(iRow, kSampleLoopStartColumnNum + 1).Value
    sampleLongData(2) = mWorkSheet.Cells(iRow, kSampleLoopEndColumnNum + 1).Value
    sampleStringData(0) = sampleName
    
    'ship that bad boy off.
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionSetSampleData, _
                            sampleLongData, _
                            sampleStringData)
                            
    Call mGlobalUtils.CleanUpColumns(mWorkSheet)
    
End Sub
Private Sub SendInstrumentData(row As Long)
    Dim objectType As Long
    Dim instrumentID As Long

    objectType = mWorkSheet.Cells(row, kInstrumentDataTypeColumnNum + 1).Value
    
    If objectType = kBankDataType Then
        Dim bankLongData As Variant
        Dim bankStringData As Variant
        ReDim bankLongData(2) As Long    'volume & id.
        ReDim bankStringData(1) As String 'bank name.
        
        bankStringData(0) = mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value
        bankLongData(0) = mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value
        bankLongData(1) = mWorkSheet.Cells(row, kInstrumentIDColumnNum + 1).Value
        
        Call mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionSetBankData, _
                                bankLongData, _
                                bankStringData)
    End If
    
    If objectType = kInstrumentDataType Then
        instrumentID = gRow2IDDictionaries(mSheetType).Item(row)
        
        Dim instrumentLongData As Variant
        Dim instrumentStringData As Variant
        ReDim instrumentLongData(6) As Long    'id+volume,pan,trans,fine trans
        ReDim instrumentStringData(1) As String 'instrument name.
        
        'args...
        instrumentLongData(0) = instrumentID
        
        'instrument params...
        instrumentStringData(0) = mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value
        
        instrumentLongData(1) = mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value
        instrumentLongData(2) = mWorkSheet.Cells(row, kInstrumentPanColumnNum + 1).Value
        instrumentLongData(3) = mWorkSheet.Cells(row, kInstrumentTransposeColumnNum + 1).Value
        instrumentLongData(4) = mWorkSheet.Cells(row, kInstrumentTransposeFineColumnNum + 1).Value
        instrumentLongData(5) = mWorkSheet.Cells(row, kInstrumentIDColumnNum + 1).Value
        
        Call mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionSetInstrumentData, _
                                instrumentLongData, _
                                instrumentStringData)
    End If
    
    If objectType = kSampleDescDataType Then
        Dim sampleDescID As Long
        sampleDescID = gRow2IDDictionaries(mSheetType).Item(row)
        instrumentID = gSampleDesc2Instrument.Item(sampleDescID)
        
        Dim sampleDescLongData As Variant
        Dim sampleDescStringData As Variant
        ReDim sampleDescLongData(19) As Long    '2 args + 17 params.
        ReDim sampleDescStringData(1) As String 'sampledesc name.
        
        'args...
        sampleDescLongData(0) = instrumentID
        sampleDescLongData(1) = sampleDescID
        
        'sample desc params...
        sampleDescStringData(0) = mWorkSheet.Cells(row, kInstrumentNameColumnNum + 1).Value
        
        sampleDescLongData(2) = mWorkSheet.Cells(row, kInstrumentVolumeColumnNum + 1).Value
        sampleDescLongData(3) = mWorkSheet.Cells(row, kInstrumentPanColumnNum + 1).Value
        sampleDescLongData(4) = mWorkSheet.Cells(row, kInstrumentTransposeColumnNum + 1).Value
        sampleDescLongData(5) = mWorkSheet.Cells(row, kInstrumentTransposeFineColumnNum + 1).Value
        sampleDescLongData(6) = mWorkSheet.Cells(row, kInstrumentKeymapLowColumnNum + 1).Value
        sampleDescLongData(7) = mWorkSheet.Cells(row, kInstrumentKeymapBaseColumnNum + 1).Value
        sampleDescLongData(8) = mWorkSheet.Cells(row, kInstrumentKeymapHighColumnNum + 1).Value
        sampleDescLongData(9) = mWorkSheet.Cells(row, kInstrumentAttackModeColumnNum + 1).Value
        sampleDescLongData(10) = mWorkSheet.Cells(row, kInstrumentAttackRateColumnNum + 1).Value
        sampleDescLongData(11) = mWorkSheet.Cells(row, kInstrumentDecayRateColumnNum + 1).Value
        sampleDescLongData(12) = mWorkSheet.Cells(row, kInstrumentSustainLevelColumnNum + 1).Value
        sampleDescLongData(13) = mWorkSheet.Cells(row, kInstrumentSustainModeColumnNum + 1).Value
        sampleDescLongData(14) = mWorkSheet.Cells(row, kInstrumentSustainRateColumnNum + 1).Value
        sampleDescLongData(15) = mWorkSheet.Cells(row, kInstrumentReleaseModeColumnNum + 1).Value
        sampleDescLongData(16) = mWorkSheet.Cells(row, kInstrumentReleaseRateColumnNum + 1).Value
        sampleDescLongData(17) = mWorkSheet.Cells(row, kInstrumentBusColumnNum + 1).Value
        sampleDescLongData(18) = mWorkSheet.Cells(row, kInstrumentBusModeColumnNum + 1).Value
        
        Call mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionSetSampleDescData, _
                                sampleDescLongData, _
                                sampleDescStringData)
    End If
    
    
End Sub
Private Sub RescanIDs(iNumRowsFromTop As Long)
    gRow2IDDictionaries(mSheetType).RemoveAll
    gID2RowDictionaries(mSheetType).RemoveAll
    
    Dim column As Long
    If mSheetType = kSampleSheetType Then
        column = kSampleDataIDColumnNum
    ElseIf mSheetType = kInstrumentSheetType Then
        column = kInstrumentDataIDColumnNum
    End If
    
    Dim id As Long
    
    Dim running As Boolean
    running = True
    
    Dim i As Long
    
    For i = 2 To iNumRowsFromTop + 2
        
        If mWorkSheet.Cells(i, column + 1).Value = Empty Then
            'do nothing
        Else
            id = mWorkSheet.Cells(i, column + 1).Value
            Call gRow2IDDictionaries(mSheetType).Add(i, id)
            Call gID2RowDictionaries(mSheetType).Add(id, i)
        End If

    Next i

End Sub
Private Sub mWorkSheet_BeforeRightClick(ByVal Target As Range, Cancel As Boolean)
    Cancel = True
    
    Dim onDataCells As Boolean
    onDataCells = False
    
    
    If gRow2IDDictionaries(mSheetType).Exists(Target.row) And Not Target.row = 2 Then
        onDataCells = True
    End If
    
    'set up the drop down menu.
    Dim sampleSheet As Worksheet
    Dim sampleDict As Scripting.Dictionary
    Dim rowNumVariant As Variant
    Dim rowNum As Long
    
    If mSheetType = kInstrumentSheetType Then
        If mGlobalUtils.CtrlDown Then
            'check if we're on a sample desc row.
            Dim onSampleDesc As Boolean
            If mWorkSheet.Cells(Target.row, kInstrumentDataTypeColumnNum + 1).Value = kSampleDescDataType Then
                onSampleDesc = True
            Else
                onSampleDesc = False
            End If
            
            MenuFunctions.InstrumentDeletionCommandBar.Controls(1).Enabled = onDataCells
            MenuFunctions.InstrumentDeletionCommandBar.Controls(2).Enabled = onDataCells And onSampleDesc
            
            
            
            Call MenuFunctions.InstrumentDeletionCommandBar.ShowPopup
        Else
        
            Dim sampleName As String
            Set sampleSheet = mBankWorkbook.mSampleSheet.mWorkSheet
            Set sampleDict = mBankWorkbook.mSampleSheet.GetRow2IDDictionary
            
            Call MenuFunctions.gDropDownList.Clear
            
            gMenuSampleIndices.RemoveAll
            Dim counter As Long
            counter = 0
            Dim id As Long
            For Each rowNumVariant In sampleDict.Keys
                rowNum = rowNumVariant
                id = sampleSheet.Cells(rowNum, GlobalVariables.kSampleDataIDColumnNum + 1).Value
                Call gMenuSampleIndices.Add(counter, id)
                sampleName = sampleSheet.Cells(rowNum, GlobalVariables.kSampleNameColumnNum + 1).Value
                MenuFunctions.gDropDownList.AddItem (sampleName)
                counter = counter + 1
            Next rowNumVariant
        
            MenuFunctions.InstrumentAdditionCommandBar.Controls(3).Enabled = onDataCells
            MenuFunctions.InstrumentAdditionCommandBar.Controls(4).Enabled = onDataCells
            MenuFunctions.InstrumentAdditionCommandBar.Controls(5).Enabled = onDataCells
            'Call MenuFunctions.DisplayCustomPopUp(mWorkSheet.Application, MenuFunctions.kInstrumentAdditionMenu)
            Call MenuFunctions.InstrumentAdditionCommandBar.ShowPopup
            
        End If
    End If
    
    
    If mSheetType = kSampleSheetType Then
        'Call MenuFunctions.DisplayCustomPopUp(mWorkSheet.Application, MenuFunctions.kSampleAdditionMenu)
        'Dim ctrl As CommandBarControl
        'Set ctrl = MenuFunctions.SampleAdditionCommandBar.Controls(1)  'AddSample
        'ctrl.Enabled = onDataCells
        
        If mGlobalUtils.CtrlDown Then
            Call MenuFunctions.SampleDeletionCommandBar.ShowPopup
        Else
            Call MenuFunctions.SampleAdditionCommandBar.ShowPopup
        End If
    End If
        
    
End Sub

Private Sub mWorkSheet_Change(ByVal Target As Range)
    Dim val As String
    Dim r As Range
    
    Application.Cursor = xlWait
    
    If mRespondToChanges Then
        
        For Each r In Target.Rows
            Dim i As Long
            i = r.row
            
            'only let through the rows that we know about.
            If gRow2IDDictionaries(mSheetType).Exists(i) Then
                Select Case mSheetType
                    Case GlobalVariables.kHiddenSheetType
                    
                    Case GlobalVariables.kSampleSheetType
                        Call SendSampleData(i)
                    
                    Case GlobalVariables.kInstrumentSheetType
                        Call SendInstrumentData(i)
                        
                End Select
            End If
            
        Next r
    End If
    
    Application.Cursor = xlDefault
    
End Sub
Private Function FindSampleLinkCell(sampleID As Long) As String

    'this must take place in the sample sheet object.
    'Dim row As Long
    'row = gDictionaries(mSheetType).Item(sampleID)
    'Dim cell As String
    'cell = "Samples!A" & CStr(row)
    'FindSampleLinkCell = cell
    
    Dim cell As String
    cell = "GARBAGE"
    Dim i As Integer
    
    Dim longArr As Variant
    Dim stringArr As Variant
    ReDim longArr(0) As Long
    ReDim stringArr(0) As String
    Dim retVal As Long
    
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                             mBankIDNumber, _
                            GlobalVariables.kActionGetBankSamplesIDs, _
                            longArr, _
                            stringArr)
   
    'now, longArr should have all the ids for all of the samples
    'for this bank. it should be numSamples long (set by the control.)
    Dim numSamples As Long
    numSamples = UBound(longArr)
   
    Dim cellValue As Integer
    For i = 0 To numSamples
        cellValue = mBankWorkbook.mSampleSheet.mWorkSheet.Range("A2").Offset(i, kSampleDataIDColumnNum).Value
        If cellValue = sampleID Then
            cell = "Samples!A" & CStr(i + 2)
            Exit For
        End If
    Next i
    
    FindSampleLinkCell = cell
End Function
Private Sub CommentRange(iRng As Range, iStr As String)
    iRng.Comment.Text iStr
    iRng.Comment.Shape.TextFrame.AutoSize = True
End Sub

Private Sub UpdateSampleSheet()
    Dim longArr As Variant
    Dim stringArr As Variant
    ReDim longArr(0) As Long
    ReDim stringArr(0) As String
    Dim retVal As Long
    
    mRespondToChanges = False
    
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetBankSamplesIDs, _
                                longArr, _
                                stringArr)
        
        'now, longArr should have all the ids for all of the samples
        'for this bank. it should be numSamples long (set by the control.)
        Dim numSamples As Long
        numSamples = UBound(longArr)
        
        'now, loop over every sample and fill it in...
        If numSamples > -1 Then
        
            Dim sampleLongData As Variant
            Dim sampleStringData As Variant
            
            Dim i As Integer
            For i = 0 To numSamples
            
                ReDim sampleLongData(1) As Long
                ReDim sampleStringData(0) As String
            
                sampleLongData(0) = longArr(i)  'sending the sample id argument.
                
                retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetSampleData, _
                                        sampleLongData, _
                                        sampleStringData)
                
                'the data starts on row '2' because the top row is a title row.
                mWorkSheet.Range("A2").Offset(i, kSampleNameColumnNum).Value = sampleStringData(0)
                mWorkSheet.Range("A2").Offset(i, kSampleRateColumnNum).Value = sampleLongData(0)
                mWorkSheet.Range("A2").Offset(i, kSampleSizeColumnNum).Value = sampleLongData(1)
                mWorkSheet.Range("A2").Offset(i, kSampleLoopStartColumnNum).Value = sampleLongData(2)
                mWorkSheet.Range("A2").Offset(i, kSampleLoopEndColumnNum).Value = sampleLongData(3)
                mWorkSheet.Range("A2").Offset(i, kSampleUsedColumnNum).Value = sampleStringData(1)
                mWorkSheet.Range("A2").Offset(i, kSampleFileNameColumnNum).Value = sampleStringData(2)
                'my super-secret data columns
                mWorkSheet.Range("A2").Offset(i, kSampleDataTypeColumnNum).Value = kSampleDataType
                mWorkSheet.Range("A2").Offset(i, kSampleDataIDColumnNum).Value = longArr(i)
                
                'store the sampleID keyed against the row
                Dim sampRng As Range
                Set sampRng = mWorkSheet.Range("A2").Offset(i, 0)
                Call gRow2IDDictionaries(mSheetType).Add(sampRng.row, longArr(i))
                Call gID2RowDictionaries(mSheetType).Add(longArr(i), sampRng.row)
    
                
                
                
                'add the comment.
                Dim sampleCommentLongData As Variant
                Dim sampleCommentStringData As Variant
                ReDim sampleCommentLongData(1) As Long
                ReDim sampleCommentStringData(0) As String
                sampleCommentLongData(0) = longArr(i)
                
                retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetSampleWriteup, _
                                        sampleCommentLongData, _
                                        sampleCommentStringData)
                            
                'comment the sample.
                Dim str As String
                str = sampleCommentStringData(0)
                mWorkSheet.Range("A2").Offset(i, kSampleNameColumnNum).AddComment
                Call CommentRange(mWorkSheet.Range("A2").Offset(i, kSampleNameColumnNum), str)
                
            Next i
            
        End If
        
        'make the sheet look right...
        Call mGlobalUtils.CleanUpColumns(mWorkSheet)
    
    mRespondToChanges = True
    
End Sub
Public Sub InsertNewSampleDescription(iSampleDescID As Long, iInstrumentID As Long)
    Dim nextRow As Long
    nextRow = -1
    Dim retVal As Long
    Dim key As Long
    Dim keyVar As Variant
    Dim row As Long
    For Each keyVar In gSampleDesc2Instrument.Keys
        key = keyVar
        If gSampleDesc2Instrument(key) = iInstrumentID Then
            row = gID2RowDictionaries(mSheetType).Item(key)
            If row >= nextRow Then
                nextRow = row + 1
            End If
        End If
    Next keyVar
    
    'ungroup the instrument so that we can group it later
    Call GroupInstrument(iInstrumentID, False)
    
    Dim sampleDescDataLongArr As Variant
    Dim sampleDescDataStringArr As Variant
    ReDim sampleDescDataLongArr(2) As Long
    ReDim sampleDescDataStringArr(0) As String
    
    mRespondToChanges = False
    
    'arguments.
    sampleDescDataLongArr(0) = iInstrumentID           'instrument #
    sampleDescDataLongArr(1) = iSampleDescID           'sample desc #
    
    
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                    mBankIDNumber, _
                    GlobalVariables.kActionGetSampleDescData, _
                    sampleDescDataLongArr, _
                    sampleDescDataStringArr)
                    
    'make your row, lay the data in, and color the row.
    'remember to increment rowOffset for each sampleDesc row you create.
    'this is always instrumentIndex+rowOffset+1 because we keep incrementing
    'rowOffset. The +1 puts it right after the old one.
    
    'make the new row.
    mWorkSheet.Rows(nextRow).Insert Shift:=xlDown
    Call RescanIDs(gRow2IDDictionaries(mSheetType).count + 1)
    
    'stick the data in
    mWorkSheet.Cells(nextRow, kInstrumentNameColumnNum + 1).Value = sampleDescDataStringArr(0)
    mWorkSheet.Cells(nextRow, kInstrumentInfoColumnNum + 1).Value = sampleDescDataStringArr(1)
    
    mWorkSheet.Cells(nextRow, kInstrumentVolumeColumnNum + 1).Value = sampleDescDataLongArr(0)
    mWorkSheet.Cells(nextRow, kInstrumentPanColumnNum + 1).Value = sampleDescDataLongArr(1)
    mWorkSheet.Cells(nextRow, kInstrumentTransposeColumnNum + 1).Value = sampleDescDataLongArr(2)
    mWorkSheet.Cells(nextRow, kInstrumentTransposeFineColumnNum + 1).Value = sampleDescDataLongArr(3)
    'no ID for sample descs.
    mWorkSheet.Cells(nextRow, kInstrumentKeymapLowColumnNum + 1).Value = sampleDescDataLongArr(4)
    mWorkSheet.Cells(nextRow, kInstrumentKeymapBaseColumnNum + 1).Value = sampleDescDataLongArr(5)
    mWorkSheet.Cells(nextRow, kInstrumentKeymapHighColumnNum + 1).Value = sampleDescDataLongArr(6)

    
    mWorkSheet.Cells(nextRow, kInstrumentAttackModeColumnNum + 1).Value = sampleDescDataLongArr(7)
    mWorkSheet.Cells(nextRow, kInstrumentAttackRateColumnNum + 1).Value = sampleDescDataLongArr(8)
    mWorkSheet.Cells(nextRow, kInstrumentDecayRateColumnNum + 1).Value = sampleDescDataLongArr(9)
    mWorkSheet.Cells(nextRow, kInstrumentSustainLevelColumnNum + 1).Value = sampleDescDataLongArr(10)
    mWorkSheet.Cells(nextRow, kInstrumentSustainModeColumnNum + 1).Value = sampleDescDataLongArr(11)
    mWorkSheet.Cells(nextRow, kInstrumentSustainRateColumnNum + 1).Value = sampleDescDataLongArr(12)
    mWorkSheet.Cells(nextRow, kInstrumentReleaseModeColumnNum + 1).Value = sampleDescDataLongArr(13)
    mWorkSheet.Cells(nextRow, kInstrumentReleaseRateColumnNum + 1).Value = sampleDescDataLongArr(14)
    mWorkSheet.Cells(nextRow, kInstrumentBusColumnNum + 1).Value = sampleDescDataLongArr(15)
    mWorkSheet.Cells(nextRow, kInstrumentBusModeColumnNum + 1).Value = sampleDescDataLongArr(16)
    
    'get the sample comment.
    Dim reffedSampleID As Long
    reffedSampleID = sampleDescDataLongArr(17)
    
    'my super-secret columns
    mWorkSheet.Cells(nextRow, kInstrumentDataTypeColumnNum + 1).Value = kSampleDescDataType
    mWorkSheet.Cells(nextRow, kInstrumentDataIDColumnNum + 1).Value = iSampleDescID
    
    
    'cache the rows against the ids and vice versa...
    Call gRow2IDDictionaries(mSheetType).Add(nextRow, iSampleDescID)
    Call gID2RowDictionaries(mSheetType).Add(iSampleDescID, nextRow)
    Call gSampleDesc2Instrument.Add(iSampleDescID, iInstrumentID)

    
    
    'add the comment.
    Dim sampleCommentLongData As Variant
    Dim sampleCommentStringData As Variant
    ReDim sampleCommentLongData(1) As Long
    ReDim sampleCommentStringData(0) As String
    sampleCommentLongData(0) = reffedSampleID
    
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionGetSampleWriteup, _
                            sampleCommentLongData, _
                            sampleCommentStringData)
                
    'comment the sample.
    Dim commStr As String
    commStr = sampleCommentStringData(0)
    mWorkSheet.Cells(nextRow, kInstrumentInfoColumnNum + 1).AddComment
    Call CommentRange(mWorkSheet.Cells(nextRow, kInstrumentInfoColumnNum + 1), commStr)

    
    Dim hyperLink As String
    'Dim sampleToLink As Long
    'sampleToLink = sampleDescDataLongArr(17)
    hyperLink = FindSampleLinkCell(reffedSampleID)
    mWorkSheet.Hyperlinks.Add Anchor:=mWorkSheet.Cells(nextRow, kInstrumentInfoColumnNum + 1), _
                              Address:="", _
                              SubAddress:=hyperLink
                              

    'set the row color.
    With mWorkSheet.Rows(nextRow).Font
        .Name = "Arial"
        .FontStyle = "Regular"
        .size = 10
        .Strikethrough = False
        .Superscript = False
        .Subscript = False
        .OutlineFont = False
        .Shadow = False
        .Underline = xlUnderlineStyleNone
        .ColorIndex = 2
    End With
    
    'regroup with the new item.
    Call GroupInstrument(iInstrumentID, True)
    
    'Call SortInstrument(iInstrumentID, -1, True)

    mRespondToChanges = True
    
End Sub
Public Sub InsertNewInstrument(iInstrumentID As Long)
    Dim row As Long
    Dim rowVar As Variant
    Dim highest As Long
    highest = -1
    Dim retVal As Long

    If gRow2IDDictionaries(mSheetType).count = 0 Then
        highest = 2
    Else
        For Each rowVar In gRow2IDDictionaries(mSheetType).Keys
            row = rowVar
            If row > highest Then
                highest = row
            End If
        Next rowVar
    End If
    
    highest = highest + 1
    
    Dim instrumentDataLongArr As Variant
    Dim instrumentDataStringArr As Variant
    ReDim instrumentDataLongArr(1) As Long
    ReDim instrumentDataStringArr(0) As String
                     
    'set the argument.
    instrumentDataLongArr(0) = iInstrumentID

    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionGetInstrumentData, _
                            instrumentDataLongArr, _
                            instrumentDataStringArr)
    

    'the data starts on row '3' because the top row is a title row and the
    'next row is the bank row.
    mWorkSheet.Cells(highest, kInstrumentNameColumnNum + 1).Value = instrumentDataStringArr(0)
    mWorkSheet.Cells(highest, kInstrumentInfoColumnNum + 1).Value = instrumentDataStringArr(1)
    mWorkSheet.Cells(highest, kInstrumentVolumeColumnNum + 1).Value = instrumentDataLongArr(0)
    mWorkSheet.Cells(highest, kInstrumentPanColumnNum + 1).Value = instrumentDataLongArr(1)
    mWorkSheet.Cells(highest, kInstrumentTransposeColumnNum + 1).Value = instrumentDataLongArr(2)
    mWorkSheet.Cells(highest, kInstrumentTransposeFineColumnNum + 1).Value = instrumentDataLongArr(3)
    mWorkSheet.Cells(highest, kInstrumentIDColumnNum + 1).Value = instrumentDataLongArr(4)
    'my super-secret columns
    mWorkSheet.Cells(highest, kInstrumentDataTypeColumnNum + 1).Value = kInstrumentDataType
    mWorkSheet.Cells(highest, kInstrumentDataIDColumnNum + 1).Value = iInstrumentID

    'cache the rows and ids against each other.
    Call gRow2IDDictionaries(mSheetType).Add(highest, iInstrumentID)
    Call gID2RowDictionaries(mSheetType).Add(iInstrumentID, highest)
    Call gSampleDesc2Instrument.Add(iInstrumentID, iInstrumentID)
            
    'get the instrument comment
    Dim instrumentCommentLongData As Variant
    Dim instrumentCommentStringData As Variant
    ReDim instrumentCommentLongData(1) As Long
    ReDim instrumentCommentStringData(0) As String
    instrumentCommentLongData(0) = iInstrumentID
    
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionGetInstrumentWriteup, _
                            instrumentCommentLongData, _
                            instrumentCommentStringData)
                
    'comment the instrument.
    Dim instCommStr As String
    instCommStr = instrumentCommentStringData(0)
    mWorkSheet.Cells(highest, kInstrumentInfoColumnNum + 1).AddComment
    Call CommentRange(mWorkSheet.Cells(highest, kInstrumentInfoColumnNum + 1), instCommStr)

    
    
    'set the row color.
    With mWorkSheet.Rows(highest).Font
        .Name = "Arial"
        .FontStyle = "Regular"
        .size = 11
        .Strikethrough = False
        .Superscript = False
        .Subscript = False
        .OutlineFont = False
        .Shadow = False
        .Underline = xlUnderlineStyleNone
        .ColorIndex = 35
    End With
    
    
    'for each instrument, you have to show it's sample descriptions...
    Dim sampleDescIDLongArr As Variant
    Dim sampleDescIDStringArr As Variant
    ReDim sampleDescIDLongArr(1) As Long
    ReDim sampleDescIDStringArr(0) As String
    
    'get the number of sample descriptions in this instrument.
    sampleDescIDLongArr(0) = iInstrumentID
    retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionGetInstrumentSampleDescIDs, _
                            sampleDescIDLongArr, _
                            sampleDescIDStringArr)
                            
    Dim numSampleDescs As Long
    numSampleDescs = UBound(sampleDescIDLongArr)
    
    If numSampleDescs > -1 Then
        Dim sampleDescIndex As Long
        Dim sampleDescDataLongArr As Variant
        Dim sampleDescDataStringArr As Variant

        'this is where our group starts.
        Dim firstSampleDescRow As Long
        firstSampleDescRow = highest + 1
        
        For sampleDescIndex = 0 To numSampleDescs
            ReDim sampleDescDataLongArr(2) As Long
            ReDim sampleDescDataStringArr(0) As String
            
            'arguments.
            sampleDescDataLongArr(0) = iInstrumentID                            'instrument #
            sampleDescDataLongArr(1) = sampleDescIDLongArr(sampleDescIndex)     'sample desc #
            
            
            retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                            mBankIDNumber, _
                            GlobalVariables.kActionGetSampleDescData, _
                            sampleDescDataLongArr, _
                            sampleDescDataStringArr)
                            
            'make your row, lay the data in, and color the row.
            'remember to increment rowOffset for each sampleDesc row you create.
            'this is always instrumentIndex+rowOffset+1 because we keep incrementing
            'rowOffset. The +1 puts it right after the old one.
            
            'make the new row.
            mWorkSheet.Rows(firstSampleDescRow).Insert Shift:=xlDown
            
            'stick the data in
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentNameColumnNum + 1).Value = sampleDescDataStringArr(0)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentInfoColumnNum + 1).Value = sampleDescDataStringArr(1)
            
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentVolumeColumnNum + 1).Value = sampleDescDataLongArr(0)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentPanColumnNum + 1).Value = sampleDescDataLongArr(1)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentTransposeColumnNum + 1).Value = sampleDescDataLongArr(2)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentTransposeFineColumnNum + 1).Value = sampleDescDataLongArr(3)
            'no ID for sample descs.
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentKeymapLowColumnNum + 1).Value = sampleDescDataLongArr(4)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentKeymapBaseColumnNum + 1).Value = sampleDescDataLongArr(5)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentKeymapHighColumnNum + 1).Value = sampleDescDataLongArr(6)

            
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentAttackModeColumnNum + 1).Value = sampleDescDataLongArr(7)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentAttackRateColumnNum + 1).Value = sampleDescDataLongArr(8)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentDecayRateColumnNum + 1).Value = sampleDescDataLongArr(9)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentSustainLevelColumnNum + 1).Value = sampleDescDataLongArr(10)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentSustainModeColumnNum + 1).Value = sampleDescDataLongArr(11)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentSustainRateColumnNum + 1).Value = sampleDescDataLongArr(12)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentReleaseModeColumnNum + 1).Value = sampleDescDataLongArr(13)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentReleaseRateColumnNum + 1).Value = sampleDescDataLongArr(14)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentBusColumnNum + 1).Value = sampleDescDataLongArr(15)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentBusModeColumnNum + 1).Value = sampleDescDataLongArr(16)
            'my super-secret columns
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentDataTypeColumnNum + 1).Value = kSampleDescDataType
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentDataIDColumnNum + 1).Value = sampleDescIDLongArr(sampleDescIndex)
            
            'get the sample comment.
            Dim reffedSampleID As Long
            reffedSampleID = sampleDescDataLongArr(17)
            
            'cache the rows against the ids and vice versa...
            Call gRow2IDDictionaries(mSheetType).Add(mWorkSheet.Cells(firstSampleDescRow, kInstrumentDataIDColumnNum + 1).row, sampleDescIDLongArr(sampleDescIndex))
            Call gID2RowDictionaries(mSheetType).Add(sampleDescIDLongArr(sampleDescIndex), mWorkSheet.Cells(firstSampleDescRow, kInstrumentDataIDColumnNum + 1).row)
            Call gSampleDesc2Instrument.Add(sampleDescIDLongArr(sampleDescIndex), iInstrumentID)
        
            
            
            'add the comment.
            Dim sampleCommentLongData As Variant
            Dim sampleCommentStringData As Variant
            ReDim sampleCommentLongData(1) As Long
            ReDim sampleCommentStringData(0) As String
            sampleCommentLongData(0) = reffedSampleID
            
            retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                    mBankIDNumber, _
                                    GlobalVariables.kActionGetSampleWriteup, _
                                    sampleCommentLongData, _
                                    sampleCommentStringData)
                        
            'comment the sample.
            Dim commStr As String
            commStr = sampleCommentStringData(0)
            mWorkSheet.Cells(firstSampleDescRow, kInstrumentInfoColumnNum + 1).AddComment
            Call CommentRange(mWorkSheet.Cells(firstSampleDescRow, kInstrumentInfoColumnNum + 1), commStr)
    
            
            Dim hyperLink As String
            'Dim sampleToLink As Long
            'sampleToLink = sampleDescDataLongArr(17)
            hyperLink = FindSampleLinkCell(reffedSampleID)
            mWorkSheet.Hyperlinks.Add Anchor:=mWorkSheet.Cells(firstSampleDescRow, kInstrumentInfoColumnNum + 1), _
                                      Address:="", _
                                      SubAddress:=hyperLink
                                      

            'set the row color.
            With mWorkSheet.Rows(firstSampleDescRow).Font
                .Name = "Arial"
                .FontStyle = "Regular"
                .size = 10
                .Strikethrough = False
                .Superscript = False
                .Subscript = False
                .OutlineFont = False
                .Shadow = False
                .Underline = xlUnderlineStyleNone
                .ColorIndex = 2
            End With

            

                    
            'increment the row offset.
            firstSampleDescRow = firstSampleDescRow + 1
                            
        
        Next sampleDescIndex
        
        'make the group for this instrument
        Dim instrumentRow As Long
        instrumentRow = gID2RowDictionaries(mSheetType).Item(iInstrumentID)
        
        'firstSampleDescRow = firstSampleDescRow + 3
        'Dim lastSampleDescRow As Long
        'lastSampleDescRow = firstSampleDescRow + numSampleDescs
        Dim rng As String
        rng = CStr(instrumentRow + 1) & ":" & CStr(firstSampleDescRow - 1)
        mWorkSheet.Range(rng).Rows.Group
        
        
        
    End If
    
    
    
End Sub


Private Sub UpdateInstrumentSheet()
    Dim retVal As Long
    
    Dim instrumentIndex As Integer
    Dim sampleDescIndex As Integer
    
    'update the bank data.
    Dim bankDataLongArr As Variant
    Dim bankDataStringArr As Variant
    ReDim bankDataLongArr(0) As Long
    ReDim bankDataStringArr(0) As String
    
    mRespondToChanges = False
    
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetBankData, _
                                bankDataLongArr, bankDataStringArr)
                                
        mWorkSheet.Range("A2").Offset(0, kInstrumentNameColumnNum).Value = bankDataStringArr(0)
        mWorkSheet.Range("A2").Offset(0, kInstrumentVolumeColumnNum).Value = bankDataLongArr(0)
        mWorkSheet.Range("A2").Offset(0, kInstrumentIDColumnNum).Value = bankDataLongArr(1)
        'my super-secret colums.
        mWorkSheet.Range("A2").Offset(0, kInstrumentDataTypeColumnNum).Value = kBankDataType
        mWorkSheet.Range("A2").Offset(0, kInstrumentDataIDColumnNum).Value = mBankIDNumber
        
        
        'cache the rows and ids against each other.
        Call gRow2IDDictionaries(mSheetType).Add(mWorkSheet.Range("A2").Offset(0, kInstrumentDataIDColumnNum).row, mBankIDNumber)
        Call gID2RowDictionaries(mSheetType).Add(mBankIDNumber, mWorkSheet.Range("A2").Offset(0, kInstrumentDataIDColumnNum).row)
                 
        
        'set the font for the bank row.
        With mWorkSheet.Range("2:2").Font
            .Name = "Arial"
            .FontStyle = "Bold"
            .size = 12
            .Strikethrough = False
            .Superscript = False
            .Subscript = False
            .OutlineFont = False
            .Shadow = False
            .Underline = xlUnderlineStyleNone
            .ColorIndex = 8
        End With
        
        'get the number of instruments in this bank...
        Dim instrumentIDLongArr As Variant
        Dim instrumentIDStringArr As Variant
        ReDim instrumentIDLongArr(0) As Long
        ReDim instrumentIDStringArr(0) As String
        
        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                mBankIDNumber, _
                                GlobalVariables.kActionGetBankInstrumentIDs, _
                                instrumentIDLongArr, instrumentIDStringArr)
        
        Dim numInstruments As Long
        numInstruments = UBound(instrumentIDLongArr)
        
        If numInstruments > -1 Then
        
            Dim instrumentDataLongArr As Variant
            Dim instrumentDataStringArr As Variant
            
            'this fixes the fact that we're creating rows out from underneath
            'the other instruments.
            Dim rowOffset As Integer
            rowOffset = 0
    
    
            For instrumentIndex = 0 To numInstruments
            
                ReDim instrumentDataLongArr(1) As Long
                ReDim instrumentDataStringArr(0) As String
                
                
                
                
                'set the argument.
                instrumentDataLongArr(0) = instrumentIDLongArr(instrumentIndex)
            
                retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetInstrumentData, _
                                        instrumentDataLongArr, _
                                        instrumentDataStringArr)
                
    
                'the data starts on row '3' because the top row is a title row and the
                'next row is the bank row.
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentNameColumnNum).Value = instrumentDataStringArr(0)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentInfoColumnNum).Value = instrumentDataStringArr(1)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentVolumeColumnNum).Value = instrumentDataLongArr(0)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentPanColumnNum).Value = instrumentDataLongArr(1)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentTransposeColumnNum).Value = instrumentDataLongArr(2)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentTransposeFineColumnNum).Value = instrumentDataLongArr(3)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentIDColumnNum).Value = instrumentDataLongArr(4)
                'my super-secret columns
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentDataTypeColumnNum).Value = kInstrumentDataType
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentDataIDColumnNum).Value = instrumentIDLongArr(instrumentIndex)
    
                'cache the rows and ids against each other.
                Call gRow2IDDictionaries(mSheetType).Add(mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentDataIDColumnNum).row, instrumentIDLongArr(instrumentIndex))
                Call gID2RowDictionaries(mSheetType).Add(instrumentIDLongArr(instrumentIndex), mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentDataIDColumnNum).row)
                Call gSampleDesc2Instrument.Add(instrumentIDLongArr(instrumentIndex), instrumentIDLongArr(instrumentIndex))
                        
                'get the instrument comment
                Dim instrumentCommentLongData As Variant
                Dim instrumentCommentStringData As Variant
                ReDim instrumentCommentLongData(1) As Long
                ReDim instrumentCommentStringData(0) As String
                instrumentCommentLongData(0) = instrumentIDLongArr(instrumentIndex)
                
                retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetInstrumentWriteup, _
                                        instrumentCommentLongData, _
                                        instrumentCommentStringData)
                            
                'comment the instrument.
                Dim instCommStr As String
                instCommStr = instrumentCommentStringData(0)
                mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentInfoColumnNum).AddComment
                Call CommentRange(mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset, kInstrumentInfoColumnNum), instCommStr)
        
                
                
                'set the row color.
                With mWorkSheet.Range("3:3").Offset(instrumentIndex + rowOffset).Font
                    .Name = "Arial"
                    .FontStyle = "Regular"
                    .size = 11
                    .Strikethrough = False
                    .Superscript = False
                    .Subscript = False
                    .OutlineFont = False
                    .Shadow = False
                    .Underline = xlUnderlineStyleNone
                    .ColorIndex = 35
                End With
                
                
                'for each instrument, you have to show it's sample descriptions...
                Dim sampleDescIDLongArr As Variant
                Dim sampleDescIDStringArr As Variant
                ReDim sampleDescIDLongArr(1) As Long
                ReDim sampleDescIDStringArr(0) As String
                
                'get the number of sample descriptions in this instrument.
                sampleDescIDLongArr(0) = instrumentIDLongArr(instrumentIndex)
                retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetInstrumentSampleDescIDs, _
                                        sampleDescIDLongArr, _
                                        sampleDescIDStringArr)
                                        
                Dim numSampleDescs As Long
                numSampleDescs = UBound(sampleDescIDLongArr)
                
                If numSampleDescs > -1 Then
                    Dim sampleDescDataLongArr As Variant
                    Dim sampleDescDataStringArr As Variant
    
                    'this is where our group starts.
                    Dim firstSampleDescRow As Long
                    firstSampleDescRow = instrumentIndex + rowOffset + 1
                    
                    For sampleDescIndex = 0 To numSampleDescs
                        ReDim sampleDescDataLongArr(2) As Long
                        ReDim sampleDescDataStringArr(0) As String
                        
                        'arguments.
                        sampleDescDataLongArr(0) = instrumentIDLongArr(instrumentIndex) 'instrument #
                        sampleDescDataLongArr(1) = sampleDescIDLongArr(sampleDescIndex)    'sample desc #
                        
                        
                        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                        mBankIDNumber, _
                                        GlobalVariables.kActionGetSampleDescData, _
                                        sampleDescDataLongArr, _
                                        sampleDescDataStringArr)
                                        
                        'make your row, lay the data in, and color the row.
                        'remember to increment rowOffset for each sampleDesc row you create.
                        'this is always instrumentIndex+rowOffset+1 because we keep incrementing
                        'rowOffset. The +1 puts it right after the old one.
                        
                        'make the new row.
                        mWorkSheet.Range("3:3").Offset(instrumentIndex + rowOffset + 1).Insert Shift:=xlDown
                        
                        'stick the data in
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentNameColumnNum).Value = sampleDescDataStringArr(0)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentInfoColumnNum).Value = sampleDescDataStringArr(1)
                        
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentVolumeColumnNum).Value = sampleDescDataLongArr(0)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentPanColumnNum).Value = sampleDescDataLongArr(1)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentTransposeColumnNum).Value = sampleDescDataLongArr(2)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentTransposeFineColumnNum).Value = sampleDescDataLongArr(3)
                        'no ID for sample descs.
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentKeymapLowColumnNum).Value = sampleDescDataLongArr(4)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentKeymapBaseColumnNum).Value = sampleDescDataLongArr(5)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentKeymapHighColumnNum).Value = sampleDescDataLongArr(6)
    
                        
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentAttackModeColumnNum).Value = sampleDescDataLongArr(7)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentAttackRateColumnNum).Value = sampleDescDataLongArr(8)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentDecayRateColumnNum).Value = sampleDescDataLongArr(9)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentSustainLevelColumnNum).Value = sampleDescDataLongArr(10)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentSustainModeColumnNum).Value = sampleDescDataLongArr(11)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentSustainRateColumnNum).Value = sampleDescDataLongArr(12)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentReleaseModeColumnNum).Value = sampleDescDataLongArr(13)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentReleaseRateColumnNum).Value = sampleDescDataLongArr(14)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentBusColumnNum).Value = sampleDescDataLongArr(15)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentBusModeColumnNum).Value = sampleDescDataLongArr(16)
                        
                        'get the sample comment.
                        Dim reffedSampleID As Long
                        reffedSampleID = sampleDescDataLongArr(17)
                        
                        'my super-secret columns
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentDataTypeColumnNum).Value = kSampleDescDataType
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentDataIDColumnNum).Value = sampleDescIDLongArr(sampleDescIndex)
                        
                        'cache the rows against the ids and vice versa...
                        Call gRow2IDDictionaries(mSheetType).Add(mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentDataIDColumnNum).row, sampleDescIDLongArr(sampleDescIndex))
                        Call gID2RowDictionaries(mSheetType).Add(sampleDescIDLongArr(sampleDescIndex), mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentDataIDColumnNum).row)
                        Call gSampleDesc2Instrument.Add(sampleDescIDLongArr(sampleDescIndex), instrumentIDLongArr(instrumentIndex))
                    
                        
                        
                        'add the comment.
                        Dim sampleCommentLongData As Variant
                        Dim sampleCommentStringData As Variant
                        ReDim sampleCommentLongData(1) As Long
                        ReDim sampleCommentStringData(0) As String
                        sampleCommentLongData(0) = reffedSampleID
                        
                        retVal = mGlobalUtils.TheSynthControl.XferSynthData( _
                                                mBankIDNumber, _
                                                GlobalVariables.kActionGetSampleWriteup, _
                                                sampleCommentLongData, _
                                                sampleCommentStringData)
                                    
                        'comment the sample.
                        Dim commStr As String
                        commStr = sampleCommentStringData(0)
                        mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentInfoColumnNum).AddComment
                        Call CommentRange(mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentInfoColumnNum), commStr)
                
                        
                        'add the hyperlink.
                        'ActiveSheet.Hyperlinks.Add Anchor:=Selection, Address:="", SubAddress:="Samples!A2"
                        'Dim hyperLinkShtName As String
                        'hyperLinkShtName = ActiveWorkbook.Sheets(2).Name
                        
                        Dim hyperLink As String
                        'Dim sampleToLink As Long
                        'sampleToLink = sampleDescDataLongArr(17)
                        hyperLink = FindSampleLinkCell(reffedSampleID)
                        mWorkSheet.Hyperlinks.Add Anchor:=mWorkSheet.Range("A3").Offset(instrumentIndex + rowOffset + 1, kInstrumentInfoColumnNum), _
                                                  Address:="", _
                                                  SubAddress:=hyperLink
                                                  
    
                        'set the row color.
                        With mWorkSheet.Range("3:3").Offset(instrumentIndex + rowOffset + 1).Font
                            .Name = "Arial"
                            .FontStyle = "Regular"
                            .size = 10
                            .Strikethrough = False
                            .Superscript = False
                            .Subscript = False
                            .OutlineFont = False
                            .Shadow = False
                            .Underline = xlUnderlineStyleNone
                            .ColorIndex = 2
                        End With
    
                        
    
                                
                        'increment the row offset.
                        rowOffset = rowOffset + 1
                                        
                    
                    Next sampleDescIndex
                    
                    'make the group for this instrument
                    'firstSampleDescRow = firstSampleDescRow + 3
                    'Dim lastSampleDescRow As Long
                    'lastSampleDescRow = firstSampleDescRow + numSampleDescs
                    'Dim rng As String
                    'rng = CStr(firstSampleDescRow) & ":" & CStr(lastSampleDescRow)
                    'mWorkSheet.Range(rng).Rows.Group 'offset by the third row.
                    Dim id As Long
                    id = instrumentIDLongArr(instrumentIndex)
                    Call GroupInstrument(id, True)
                    
                    
                End If
            
            Next instrumentIndex
            
        End If
                            
                            
        Call mGlobalUtils.CleanUpColumns(mWorkSheet)
    
    mRespondToChanges = True
            
End Sub
Public Sub GroupInstrument(iInstrumentID As Long, iYesNo As Boolean)
   Dim rng As Range
   Set rng = GetInstrumentSampleDescsRange(iInstrumentID)
    
    If Not rng Is Nothing Then
        If iYesNo Then
            Call rng.Rows.Group
        Else
            Call rng.Rows.Ungroup
        End If
    End If

End Sub

Public Function GetInstrumentSampleDescsRange(iInstrumentID As Long) As Range
    Dim low As Long
    Dim hi As Long
    
    low = 99999 'don't have more than 99999 rows.
    hi = -1
    
    Dim foundSomeSamples As Boolean
    foundSomeSamples = False
    Dim rng As String
    Dim idVar As Variant
    Dim id As Long
    Dim row As Long
    Dim instrumentID As Long
    
    For Each idVar In gSampleDesc2Instrument.Keys
        id = idVar
        
        If gSampleDesc2Instrument.Item(id) = iInstrumentID Then
            foundSomeSamples = True
            row = gID2RowDictionaries(mSheetType).Item(id)
            
            If row > hi Then
                hi = row
            End If
            
            If row < low Then
                low = row
            End If
        End If
        
    Next idVar
    
    Dim retVal As Range
    
    If foundSomeSamples Then
        If Not low = hi Then
            rng = CStr(low + 1) & ":" & CStr(hi) 'the +1 is for the instrument itself.
            Set retVal = mWorkSheet.Range(rng)
        End If
   End If
   
   Set GetInstrumentSampleDescsRange = retVal
    
End Function

Public Sub UpdateSynthData()
    
    
    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, False)
                    
    Select Case mSheetType
        Case GlobalVariables.kHiddenSheetType
        
        Case GlobalVariables.kSampleSheetType
            UpdateSampleSheet
        
        Case GlobalVariables.kInstrumentSheetType
            UpdateInstrumentSheet
    End Select

    'Call mGlobalUtils.ProtectWorksheet(mWorkSheet, True)
  
End Sub

