#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH
#include <windows.h>
#include <sapi.h>
#include "mmsystem.h"
#include "mmreg.h"

// cl speech.cpp /nologo /O1 /Ob1 /Oi /Os /Oy /GF /GS- /Zp1 /Gz /FAs /Fp /c /utf-8
// link speech.obj /DYNAMICBASE:NO  /MANIFEST:NO /INCREMENTAL:NO  /NODEFAULTLIB /TLBID:1 /MACHINE:X86 /SUBSYSTEM:WINDOWS /ENTRY:"entrypoint" "kernel32.lib" "ole32.lib" "winmm.lib"

const wchar_t truc[] =
    L"édjé je t'aime!";

void entrypoint(void)
{
    CoInitialize(NULL);

    ISpVoice *pVoice = NULL;
    CLSID _CLSID_SpVoice;
    CLSIDFromString(L"{96749377-3391-11D2-9EE3-00C04F797396}", &_CLSID_SpVoice);
    IID _IID_ISpVoice;
    CLSIDFromString(L"{6C44DF74-72B9-4992-A1EC-EF996E0422D4}", &_IID_ISpVoice);
    HRESULT hr = CoCreateInstance(_CLSID_SpVoice, NULL, CLSCTX_ALL, _IID_ISpVoice, (void **)&pVoice);

    IStream *pStream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    CLSID _CLSID_SpStream;
    CLSIDFromString(L"{715D9C59-4442-11D2-9605-00C04F8EE628}", &_CLSID_SpStream);
    IID _IID_ISpStream;
    CLSIDFromString(L"{12E3CCA9-7518-44C5-A5E7-BA5A79CB929E}", &_IID_ISpStream);
    ISpStream *pSpStream = nullptr;
    hr = CoCreateInstance(_CLSID_SpStream, NULL, CLSCTX_ALL, _IID_ISpStream, (void **)&pSpStream);
    WAVEFORMATEX fmt = {};
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample / 8;
    fmt.nSamplesPerSec = 44100;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
    fmt.cbSize = 0;

    GUID _SPDFID_WaveFormatEx;
    IIDFromString(L"{C31ADBAE-527F-4FF5-A230-F62BB61FF70C}", (IID *)&_SPDFID_WaveFormatEx);

    hr = pSpStream->SetBaseStream(pStream, _SPDFID_WaveFormatEx, &fmt);
    hr = pVoice->SetOutput(pSpStream, TRUE);
    pVoice->Speak(truc, SPF_IS_XML, NULL);

    STATSTG stat;
    pStream->Stat(&stat, 0);
    unsigned char *data = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stat.cbSize.LowPart);
    LARGE_INTEGER li = {0, 0};
    pStream->Seek(li, SEEK_SET, NULL);
    unsigned long c = 0;
    pStream->Read(data, stat.cbSize.LowPart, &c);

    HWAVEOUT hWaveOut;
    WAVEHDR WaveHDR = {(LPSTR)data, c, 0, 0, 0, 0, 0, 0};
    MMTIME MMTime = {TIME_BYTES, 0};
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &fmt, NULL, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
    waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));

    bool finished = false;
    long t;
    long to = timeGetTime();
    do
    {
        t = timeGetTime() - to;
        waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
        finished = !(MMTime.u.cb < c);
    } while (!finished);

    pSpStream->Release();
    pStream->Release();
    pVoice->Release();

    CoUninitialize();
    ExitProcess(0);
}
