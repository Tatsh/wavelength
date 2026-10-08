# synthedit

Reconstructed source of `synthedit_r.ocx`, the ActiveX control that the Excel 2000 add-in
`synthcontrol.xla` uses to edit the synthesiser banks of the game. The add-in calls the methods of
the control to create, load, save, and change banks, and the control raises events that bring the
add-in worksheets up to date.

## Compiler

The source targets Visual C++ 6.0 with MFC 4.2, the toolchain of the original control, and later
Microsoft compilers that still build MFC ActiveX controls.

The main build does not compile this directory.

## Building

Open `synthedit.dsw` in Visual C++ 6.0 and build the `synthedit` project. The workspace builds
`ENCVAG.DLL` first and writes it next to `synthedit.ocx`, and the build registers the control with
`regsvr32`. The lexer step runs `flex`, which must be on the `PATH`.

## Layout

- `os`, `utl`, and `synth` are the parts of the game engine that the control links, with the
  same file names as the engine source.
- `synthedit` is the control: the module (`synthedit.cpp`), the control class
  (`SyntheditCtl.cpp`), the property page (`SyntheditPpg.cpp`), the dialogues, and the wrappers of
  OLE Automation arrays.
- `utl/DataLex.l` is the flex source of the script lexer. Flex writes `utl/DataLex.cpp` from it.
- `encvag` is a stand-in for `ENCVAG.DLL`, the library that converts WAV data to VAG data. It
  exports the three functions of the original by the same ordinals, and it writes silent blocks.
  A bank saved with it has the correct layout but no audio.

## Resources

`synthedit/synthedit.odl` is the type library source, from which MIDL writes `synthedit.tlb`.
`synthedit/synthedit.rc` lists the dialogues, strings, and version information of the original
control. The icon (`synthedit/res/synthedit.ico`) and the toolbox bitmap
(`synthedit/res/SyntheditCtl.bmp`) are grey placeholders of the original sizes.
