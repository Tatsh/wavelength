Attribute VB_Name = "MenuFunctions"
' macros written 2000-04-27 by Ole P. Erlandsen, ope@erlandsendata.no

Option Explicit
' see also Sheet1/Sheet2/ThisWorkbook/UserForm1 for more code
 
Private Const kInstrumentAdditionMenu As String = "InstrumentAddition"
Private Const kInstrumentDeletionMenu As String = "InstrumentDeletion"
Private Const kSampleAdditionMenu As String = "SampleAddition"
Private Const kSampleDeletionMenu As String = "SampleDeletion"

Public InstrumentAdditionCommandBar As CommandBar
Public InstrumentDeletionCommandBar As CommandBar
Public SampleAdditionCommandBar As CommandBar
Public SampleDeletionCommandBar As CommandBar

Public gDropDownList As CommandBarComboBox
Public mGlobalUtils As GlobalUtils


Sub DeletePopUp(app As Excel.Application) ' deletes the custom popup menu
    On Error Resume Next
    'Call app.CommandBars(PopUpCommandBarName).Delete
    On Error GoTo 0
End Sub

Sub CreatePopUp(app As Excel.Application) ' creates the custom popup menu
    Dim cb As CommandBar
    'Call DeletePopUp(app)
    
    
    'InstrumentAdditionMenu
    Set InstrumentAdditionCommandBar = app.CommandBars.Add(kInstrumentAdditionMenu, msoBarPopup, False, True)
    With InstrumentAdditionCommandBar
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentAddition"
          .FaceId = 0
          .Caption = "Add Instrument"
          .Tag = "AddInstrument"
        End With
        
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentAddition"
          .FaceId = 0
          .Caption = "Import Folder As Instruments"
          .Tag = "ImportFolderAsInstruments"
        End With
        
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentAddition"
          .FaceId = 0
          .Caption = "Sort Instrument Ascending"
          .Tag = "SortInstrumentAscending"
        End With
        
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentAddition"
          .FaceId = 0
          .Caption = "Sort Instrument Descending"
          .Tag = "SortInstrumentDescending"
        End With
        
        ' create the new submenu
        Dim SubMenu As CommandBarPopup
        Set SubMenu = .Controls.Add(Type:=msoControlPopup, Temporary:=True)
        With SubMenu ' add the menu caption
            .BeginGroup = True
            .Caption = "Add Sample Description(s)"
            .Tag = "MySubMenuTag"
        
            With .Controls.Add(Type:=msoControlButton, Temporary:=True)
                .Caption = "From Audio File(s)"
                .OnAction = "HandleInstrumentAddition"
                .FaceId = 0
                .Tag = "AddSampleDescFromAudioFile"
            End With
            

            Set gDropDownList = .Controls.Add(Type:=msoControlDropdown, Temporary:=True)
            With gDropDownList
                .AddItem "Hello"
                .AddItem "Goodbye."
                .OnAction = "HandleInstrumentAddition"
                .Tag = "gDropDownList"
            End With
            
            
            
        End With
        
    End With
    
    
    
    'InstrumentDeletionMenu
    Set InstrumentDeletionCommandBar = app.CommandBars.Add(kInstrumentDeletionMenu, msoBarPopup, False, True)
    With InstrumentDeletionCommandBar
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentDeletion"
          .FaceId = 0
          .Caption = "Delete Instrument"
          .Tag = "DeleteInstrument"
        End With
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleInstrumentDeletion"
          .FaceId = 0
          .Caption = "Delete Sample Description"
          .Tag = "DeleteSampleDescription"
        End With
    End With
    
    'SampleDeletionMenu
    Set SampleDeletionCommandBar = app.CommandBars.Add(kSampleDeletionMenu, msoBarPopup, False, True)
    With SampleDeletionCommandBar
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleSampleDeletion"
          .FaceId = 0
          .Caption = "Delete Sample"
          .Tag = "DeleteSample"
        End With
    End With
    
    
    'SampleAdditionMenu
    Set SampleAdditionCommandBar = app.CommandBars.Add(kSampleAdditionMenu, msoBarPopup, False, True)
    With SampleAdditionCommandBar
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleSampleAddition"
          .FaceId = 0
          .Caption = "Add Sample"
          .Tag = "AddSample"
        End With
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleSampleAddition"
          .FaceId = 0
          .Caption = "Import Folder As Samples"
          .Tag = "ImportFolderAsSamples"
        End With
        
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleSampleAddition"
          .FaceId = 0
          .Caption = "Sort Samples Ascending"
          .Tag = "SortSamplesAscending"
        End With
        
        With .Controls.Add(Type:=msoControlButton)
          .OnAction = "HandleSampleAddition"
          .FaceId = 0
          .Caption = "Sort Samples Descending"
          .Tag = "SortSamplesDescending"
        End With
        
    End With
    
    
    Set cb = Nothing
End Sub

'Sub DisplayCustomPopUp(app As Excel.Application, iPopupName As String) ' displays the custom popup menu
'    Call app.CommandBars(iPopupName).ShowPopup
'End Sub
Sub HandleInstrumentAddition()
    Dim Ctrl As CommandBarControl
    Set Ctrl = Application.CommandBars.ActionControl
    
    Dim row As Long
    Dim objectType As Long
    Dim id As Long
    Dim instrumentID As Long
    Dim ctrlCol As Long
    
    If Ctrl.Tag = "AddInstrument" Then
        row = ActiveCell.row
        id = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.mWorkSheet.Cells(row, kInstrumentDataIDColumnNum + 1)
        instrumentID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetSampleDesc2InstrumentDictionary.Item(id)
        
        Call mGlobalUtils.TheSynthControl.NewInstrument(mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber)
    
     
    ElseIf Ctrl.Tag = "SortInstrumentAscending" Or Ctrl.Tag = "SortInstrumentDescending" Then
        row = ActiveCell.row
        id = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.mWorkSheet.Cells(row, kInstrumentDataIDColumnNum + 1)
        instrumentID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetSampleDesc2InstrumentDictionary.Item(id)
        ctrlCol = ActiveCell.column
        Call mGlobalUtils.ProtectWorksheet(mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.mWorkSheet, False)
        If Ctrl.Tag = "SortInstrumentAscending" Then
            Call mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.SortInstrument(instrumentID, ctrlCol, True)
        ElseIf Ctrl.Tag = "SortInstrumentDescending" Then
            Call mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.SortInstrument(instrumentID, ctrlCol, False)
        End If
        Call mGlobalUtils.ProtectWorksheet(mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.mWorkSheet, True)
        
    ElseIf Ctrl.Tag = "AddSampleDescFromAudioFile" Then
        row = ActiveCell.row
        id = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.mWorkSheet.Cells(row, kInstrumentDataIDColumnNum + 1)
        instrumentID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetSampleDesc2InstrumentDictionary.Item(id)
        
        Call mGlobalUtils.TheSynthControl.NewSampleDescription(mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber, instrumentID, -1)
    
    ElseIf Ctrl.Tag = "ImportFolderAsInstruments" Then
        Call mGlobalUtils.TheSynthControl.NewInstrumentsFromFolder(mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber)
    
    ElseIf Ctrl.Tag = gDropDownList.Tag Then
        Dim index As Long
        Dim sampleID As Long
        Dim bankID As Long
        Dim sampleDescID As Long
        index = gDropDownList.ListIndex
        
        row = ActiveCell.row
        sampleDescID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetRow2IDDictionary().Item(row)
        instrumentID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetSampleDesc2InstrumentDictionary().Item(sampleDescID)
        bankID = mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber
        sampleID = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetIDForMenuIndex(index - 1)
        Call mGlobalUtils.TheSynthControl.NewSampleDescription(bankID, instrumentID, sampleID)
    End If
    
    
    
End Sub

Sub HandleInstrumentDeletion()
    Dim Ctrl As CommandBarControl
    Set Ctrl = Application.CommandBars.ActionControl
    Dim row As Long
    row = ActiveCell.row
    Dim instrumentID As Long
    Dim sampleDescID As Long
    Dim bankID As Long
    
    Dim row2IDDict As Scripting.Dictionary
    Dim sampleDescToInstrumentDict As Scripting.Dictionary
    Set row2IDDict = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetRow2IDDictionary
    Set sampleDescToInstrumentDict = mGlobalUtils.gCurrentBankWorkbook.mInstrumentSheet.GetSampleDesc2InstrumentDictionary
    bankID = mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber
        
    If Ctrl.Tag = "DeleteSampleDescription" Then
        If MsgBox("Are you sure that you want to delete these sample descriptions?", vbYesNo, "Delete?!!?") = 7 Then
            Exit Sub
        End If
        
        sampleDescID = row2IDDict.Item(row)
        instrumentID = sampleDescToInstrumentDict.Item(sampleDescID)
        Call mGlobalUtils.TheSynthControl.DeleteSampleDescription(bankID, instrumentID, sampleDescID)
    End If
    
    If Ctrl.Tag = "DeleteInstrument" Then
        If MsgBox("Are you sure that you want to delete this instrument?", vbYesNo, "Delete?!!?") = 7 Then
            Exit Sub
        End If
        
        sampleDescID = row2IDDict.Item(row)
        instrumentID = sampleDescToInstrumentDict.Item(sampleDescID)
        Call mGlobalUtils.TheSynthControl.DeleteInstrument(bankID, instrumentID)
    End If
End Sub


Sub HandleSampleAddition()
    Dim Ctrl As CommandBarControl
    Set Ctrl = Application.CommandBars.ActionControl
    Dim row As Long
    Dim ctrlCol As Long
    
    If Ctrl.Tag = "AddSample" Then
        Call mGlobalUtils.TheSynthControl.NewSample(mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber)
    End If
    
    If Ctrl.Tag = "ImportFolderAsSamples" Then
        Call mGlobalUtils.TheSynthControl.NewSamplesFromFolder(mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber)
    End If
    
     If Ctrl.Tag = "SortSamplesAscending" Or Ctrl.Tag = "SortSamplesDescending" Then
        row = ActiveCell.row
        ctrlCol = ActiveCell.column
        Call mGlobalUtils.ProtectWorksheet(mGlobalUtils.gCurrentBankWorkbook.mSampleSheet.mWorkSheet, False)
        If Ctrl.Tag = "SortSamplesAscending" Then
            Call mGlobalUtils.gCurrentBankWorkbook.mSampleSheet.SortSamples(ctrlCol, True)
        ElseIf Ctrl.Tag = "SortSamplesDescending" Then
            Call mGlobalUtils.gCurrentBankWorkbook.mSampleSheet.SortSamples(ctrlCol, False)
        End If
        Call mGlobalUtils.ProtectWorksheet(mGlobalUtils.gCurrentBankWorkbook.mSampleSheet.mWorkSheet, True)
    End If
    
End Sub

Sub HandleSampleDeletion()
    Dim Ctrl As CommandBarControl
    Set Ctrl = Application.CommandBars.ActionControl
    Dim row As Long
    Dim rowVar As Range
    Dim counterVar As Variant
    
    Dim row2IDDict As Scripting.Dictionary
    Set row2IDDict = mGlobalUtils.gCurrentBankWorkbook.mSampleSheet.GetRow2IDDictionary
    
    Dim sampleID As Long
    
    Dim bankID As Long
    bankID = mGlobalUtils.gCurrentBankWorkbook.mBankIDNumber
    
    
    Dim idArr() As Long
    ReDim idArr(0) As Long
    
    If Ctrl.Tag = "DeleteSample" Then
        If MsgBox("Are you sure that you want to delete these samples?", vbYesNo, "Delete?!!?") = 7 Then
            Exit Sub
        End If
        
        Dim counter As Long
        counter = 0
        
        
        For Each rowVar In Selection.Rows
            row = rowVar.row
            If row2IDDict.Exists(row) Then
                ReDim Preserve idArr(UBound(idArr) + 1)
                sampleID = row2IDDict.Item(row)
                idArr(counter) = sampleID
                counter = counter + 1
            End If
        Next rowVar
    
        
        Dim i As Long
        For i = 0 To counter - 1
            sampleID = idArr(i)
            Call mGlobalUtils.TheSynthControl.DeleteSample(bankID, sampleID)
        Next i
       
        
    End If
End Sub


Sub Foo()
Dim ad As AddIn
    For Each ad In Application.AddIns
        MsgBox ad.Name
        MsgBox ad.Installed
    Next
End Sub


