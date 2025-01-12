rem Runs from sciteinstaller\scite-win32\scite-win32\bin\Release
candle.exe -dinfoRtf=..\..\license.rtf ..\..\WiXDialog\WixUI_SCiTEAdvanced.wxs
lit.exe -out ..\..\WiXDialog\WixUI_SCiTEAdvanced.wixlib WixUI_SCiTEAdvanced.wixobj
..\..\..\sciteversion\bin\Release\sciteversion.exe ..\..\..\..\wscite\SciTE.exe
