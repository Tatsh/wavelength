' Build synthcontrol.xla from the VBA source in this directory.
'
' Run with `cscript //nologo build-xla.vbs` on Windows with Excel and the synthedit control
' registered. Excel must trust access to the VBA project object model (Trust Center, Macro
' Settings).

Option Explicit

Const vbext_ct_StdModule = 1
Const vbext_ct_ClassModule = 2
Const xlAddIn = 18

Dim fso, here, excel, oldSheetCount, wb, project, sheet

Set fso = CreateObject("Scripting.FileSystemObject")
here = fso.GetParentFolderName(WScript.ScriptFullName)

Set excel = CreateObject("Excel.Application")
excel.DisplayAlerts = False
oldSheetCount = excel.SheetsInNewWorkbook
excel.SheetsInNewWorkbook = 1
Set wb = excel.Workbooks.Add
excel.SheetsInNewWorkbook = oldSheetCount

Set sheet = wb.Worksheets(1)
sheet.Name = "SynthInfoWkSht"
Set project = wb.VBProject
project.VBComponents(sheet.CodeName).Name = "SynthInfoWkSht"

' Version 0.0 selects the newest registered version of each library.
project.References.AddFromGuid "{2DF8D04C-5BFA-101B-BDE5-00AA0044DE52}", 0, 0
project.References.AddFromGuid "{420B2830-E718-11CF-893D-00A0C9054228}", 0, 0
project.References.AddFromGuid "{B26CEE76-EB96-4863-82AC-A24D7F64CB93}", 1, 0

SetCode project.VBComponents("ThisWorkbook"), "ThisWorkbook.cls"
SetCode project.VBComponents("SynthInfoWkSht"), "SynthInfoWkSht.cls"
AddComponent vbext_ct_StdModule, "GlobalVariables", "GlobalVariables.bas"
AddComponent vbext_ct_StdModule, "MenuFunctions", "MenuFunctions.bas"
AddComponent vbext_ct_ClassModule, "GlobalUtils", "GlobalUtils.cls"
AddComponent vbext_ct_ClassModule, "SynthColumn", "SynthColumn.cls"
AddComponent vbext_ct_ClassModule, "BankWorkbook", "BankWorkbook.cls"
AddComponent vbext_ct_ClassModule, "SynthSheet", "SynthSheet.cls"

wb.BuiltinDocumentProperties("Author") = "Denny Bromley"
wb.BuiltinDocumentProperties("Company") = "Harmonix Music Systems, Inc."
wb.IsAddin = True
wb.SaveAs fso.BuildPath(here, "synthcontrol.xla"), xlAddIn
wb.Close False
excel.Quit
WScript.Echo "Wrote " & fso.BuildPath(here, "synthcontrol.xla")

' The VBA editor writes the attribute lines itself, so they are dropped from the code.
Function ReadCode(fileName)
    Dim stream, line, code
    Set stream = fso.OpenTextFile(fso.BuildPath(here, fileName), 1)
    code = ""
    Do While Not stream.AtEndOfStream
        line = stream.ReadLine
        If Left(line, 10) <> "Attribute " Then
            code = code & line & vbCrLf
        End If
    Loop
    stream.Close
    ReadCode = code
End Function

Sub SetCode(component, fileName)
    With component.CodeModule
        If .CountOfLines > 0 Then
            .DeleteLines 1, .CountOfLines
        End If
        .AddFromString ReadCode(fileName)
    End With
End Sub

Sub AddComponent(componentType, name, fileName)
    Dim component
    Set component = project.VBComponents.Add(componentType)
    component.Name = name
    SetCode component, fileName
End Sub
