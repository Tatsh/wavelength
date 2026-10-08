Attribute VB_Name = "ThisWorkbook"
Attribute VB_Base = "0{00020819-0000-0000-C000-000000000046}"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Attribute VB_TemplateDerived = False
Attribute VB_Customizable = True
Option Explicit

Dim gGlobals As New GlobalUtils


Private Sub Workbook_AddinInstall()
    MsgBox "Addin Install"
End Sub

Private Sub Workbook_AddinUninstall()
    MsgBox "Addin Uninstall"
End Sub

Private Sub Workbook_BeforeClose(Cancel As Boolean)
    gGlobals.ShutdownGlobals
End Sub

Private Sub Workbook_Open()
    Call gGlobals.InitGlobals(Application)
End Sub

