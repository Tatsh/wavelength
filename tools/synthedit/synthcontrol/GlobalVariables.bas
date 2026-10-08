Attribute VB_Name = "GlobalVariables"
Option Explicit

'tell the sheets what kind they are.
Public Const kInstrumentSheetType As Long = 0
Public Const kSampleSheetType As Long = 1
Public Const kHiddenSheetType As Long = 2

'these are used in the spreadsheet to tell the rows apart.
Public Const kBankDataType As Long = 1
Public Const kInstrumentDataType As Long = 2
Public Const kSampleDescDataType As Long = 3
Public Const kSampleDataType As Long = 4



'these are used in the control XferSynthData call to tell
'the control what to do.
'/////////////////////////////////////
'get the actual data
Public Const kActionGetBankData As Long = 0
Public Const kActionGetInstrumentData As Long = 1
Public Const kActionGetSampleDescData As Long = 2
Public Const kActionGetSampleData As Long = 3

'get the object counts.
Public Const kActionGetBankSamplesIDs As Long = 4
Public Const kActionGetBankInstrumentIDs As Long = 5
Public Const kActionGetInstrumentSampleDescIDs As Long = 6

'get the object pop ups...
Public Const kActionGetSampleWriteup As Long = 7
Public Const kActionGetInstrumentWriteup As Long = 8

'set the object data
Public Const kActionSetBankData As Long = 9
Public Const kActionSetInstrumentData As Long = 10
Public Const kActionSetSampleDescData As Long = 11
Public Const kActionSetSampleData As Long = 12
Public Const kActionGetReferencedSample = 13

'//////////////////////////////////////


'sample sheet columns
Public Const kSampleNameColumnNum As Integer = 0
Public Const kSampleRateColumnNum As Integer = 1
Public Const kSampleSizeColumnNum As Integer = 2
Public Const kSampleLoopStartColumnNum As Integer = 3
Public Const kSampleLoopEndColumnNum As Integer = 4
Public Const kSampleUsedColumnNum As Integer = 5
Public Const kSampleFileNameColumnNum As Integer = 6
Public Const kSampleDataTypeColumnNum As Integer = 7
Public Const kSampleDataIDColumnNum As Integer = 8
Public Const kNumSampleSheetColumns = 9

'instrument sheet columns
Public Const kInstrumentNameColumnNum As Integer = 0
Public Const kInstrumentInfoColumnNum As Integer = 1
Public Const kInstrumentBasicColumnNum As Integer = 2
Public Const kInstrumentVolumeColumnNum As Integer = 3
Public Const kInstrumentPanColumnNum As Integer = 4
Public Const kInstrumentTransposeColumnNum As Integer = 5
Public Const kInstrumentTransposeFineColumnNum As Integer = 6
Public Const kInstrumentIDColumnNum As Integer = 7
Public Const kInstrumentKeymapColumnNum As Integer = 8
Public Const kInstrumentKeymapLowColumnNum As Integer = 9
Public Const kInstrumentKeymapLowSpacerColumnNum As Integer = 10
Public Const kInstrumentKeymapBaseColumnNum As Integer = 11
Public Const kInstrumentKeymapBaseSpacerColumnNum As Integer = 12
Public Const kInstrumentKeymapHighColumnNum As Integer = 13
Public Const kInstrumentKeymapHighSpacerColumnNum As Integer = 14
Public Const kInstrumentADSRColumnNum As Integer = 15
Public Const kInstrumentAttackModeColumnNum As Integer = 16
Public Const kInstrumentAttackRateColumnNum As Integer = 17
Public Const kInstrumentDecayRateColumnNum As Integer = 18
Public Const kInstrumentSustainLevelColumnNum As Integer = 19
Public Const kInstrumentSustainModeColumnNum As Integer = 20
Public Const kInstrumentSustainRateColumnNum As Integer = 21
Public Const kInstrumentReleaseModeColumnNum As Integer = 22
Public Const kInstrumentReleaseRateColumnNum As Integer = 23
Public Const kInstrumentBusGroupColumnNum As Integer = 24
Public Const kInstrumentBusColumnNum As Integer = 25
Public Const kInstrumentBusModeColumnNum As Integer = 26

Public Const kInstrumentDataTypeColumnNum As Integer = 27
Public Const kInstrumentDataIDColumnNum As Integer = 28

Public Const kNumInstrumentSheetColumns = 29


Public Const kInstrumentSheetGroup0 As String = "D:H"
Public Const kInstrumentSheetGroup1 As String = "J:O"
Public Const kInstrumentSheetGroup2 As String = "Q:X"
Public Const kInstrumentSheetGroup3 As String = "Z:AA"

