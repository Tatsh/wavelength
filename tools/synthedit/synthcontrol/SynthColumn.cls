Attribute VB_Name = "SynthColumn"
Attribute VB_Base = "0{FCFB3D2A-A0FA-1068-A738-08002B3371B5}"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_TemplateDerived = False
Attribute VB_Customizable = False
Option Explicit

Public mTitle As String
Public mProtected As Boolean
Public mHidden As Boolean

'the validation params may have to be more general
Public mValidated As Boolean
Public mLowerBound As Long
Public mUpperBound As Long
Public mColorIndex As Integer

Public Sub Init(title As String, pro As Boolean, hid As Boolean, val As Boolean, low As Long, hi As Long, Optional iColorIndex As Integer = 19)
    mTitle = title
    mProtected = pro
    mHidden = hid
    mValidated = val
    mLowerBound = low
    mUpperBound = hi
    mColorIndex = iColorIndex
End Sub
