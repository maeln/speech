cl speech.cpp /nologo /O1 /Ob1 /Oi /Os /Oy /GF /GS- /Zp1 /Gz /FAs /Fp /c /utf-8
link speech.obj /DYNAMICBASE:NO  /MANIFEST:NO /INCREMENTAL:NO  /NODEFAULTLIB /TLBID:1 /MACHINE:X86 /SUBSYSTEM:WINDOWS /ENTRY:"entrypoint" "kernel32.lib" "ole32.lib" "winmm.lib" "user32.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" 
:: link speech.obj /DYNAMICBASE:NO  /MANIFEST:NO /INCREMENTAL:NO /TLBID:1 /MACHINE:X86 /SUBSYSTEM:WINDOWS /ENTRY:"entrypoint" "kernel32.lib" "ole32.lib" "winmm.lib" "msvcrt.lib"
