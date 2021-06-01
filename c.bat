cl -nologo -GL -O1 -Os -Gm- -GR- -EHa- -Oi -GS- -Gs99999999 speech.cpp -link -stack:0x5000000,0x5000000 -subsystem:windows -nodefaultlib "kernel32.lib" "ole32.lib" "winmm.lib" "user32.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" "song.obj" 

:: -utf-8 -MD  -Fd"Release\speech.pdb" -Fa"Release\speech.asm" -Fo"Release\speech.obj"

:: link /OUT:"Release\speech.exe" /NODEFAULTLIB /MACHINE:X86 /SUBSYSTEM:WINDOWS /DEBUG:FULL /PDB:"Release\speech.pdb" /STACK:0x100000,0x100000 "kernel32.lib" "ole32.lib" "winmm.lib" "user32.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" "song.obj" Release\speech.obj 


:: Crinkler.exe "Release\speech.obj" /OUT:"Release\speech.exe" /LTCG:incremental /TLBID:1 /DYNAMICBASE /NXCOMPAT /NODEFAULTLIB /MACHINE:X86 /SAFESEH /SUBSYSTEM:WINDOWS /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed "kernel32.lib" /DEBUG:FULL /PDB:"Release\vc142.pdb" "ole32.lib" "winmm.lib" "user32.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /CRINKLER  /RANGE:opengl32 /HASHTRIES:300 /COMPMODE:FAST /ORDERTRIES:1000 /REPORT:out.html /PROGRESSGUI /HASHSIZE:100 /UNALIGNCODE
:: link speech.obj /DYNAMICBASE:NO  /MANIFEST:NO /INCREMENTAL:NO /TLBID:1 /MACHINE:X86 /SUBSYSTEM:WINDOWS /ENTRY:"WinMainCRTStartup" "kernel32.lib" "ole32.lib" "winmm.lib" "msvcrt.lib"
:: "libcmt.lib" "libvcruntime.lib" "libucrt.lib"