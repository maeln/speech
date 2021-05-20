#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH
#include <windows.h>
#include <sapi.h>
#include <mmsystem.h>
#include <mmreg.h>

#ifdef __cplusplus
extern "C"
{
#endif
    int _fltused = 0; // it should be a single underscore since the double one is the mangled name
#ifdef __cplusplus
}
#endif

// gl
#include <gl/GL.h>
#include "glext.h"
#include "gldefs.h"

// Song
#include "4klang.h"

// shader
#include "draw_shader.h"
#include "postprocess_shader.h"
#include "tex.h"

#include "audio.h"

#define TRES 512
#define FIRST_GLYPH 1
#define GLYPHS 'z'

static void processMessages()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // Kill
            CoUninitialize();
            ShowCursor(true);
            ExitProcess(0);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

const wchar_t truc[] =
    L"<voice required=\"Language=409\">What the fuck did you just fucking say about me, you little bitch? I'll have you know I graduated top of my class in the Navy Seals, and I've been involved in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills. I am trained in gorilla warfare and I'm the top sniper in the entire US armed forces. You are nothing to me but just another target. I will wipe you the fuck out with precision the likes of which has never been seen before on this Earth, mark my fucking words. You think you can get away with saying that shit to me over the Internet? Think again, fucker. As we speak I am contacting my secret network of spies across the USA and your IP is being traced right now so you better prepare for the storm, maggot. The storm that wipes out the pathetic little thing you call your life. You're fucking dead, kid. I can be anywhere, anytime, and I can kill you in over seven hundred ways, and that's just with my bare hands. Not only am I extensively trained in unarmed combat, but I have access to the entire arsenal of the United States Marine Corps and I will use it to its full extent to wipe your miserable ass off the face of the continent, you little shit. If only you could have known what unholy retribution your little clever comment was about to bring down upon you, maybe you would have held your fucking tongue. But you couldn't, you didn't, and now you're paying the price, you goddamn idiot. I will shit fury all over you and you will drown in it. You're fucking dead, kiddo.</voice>";

const wchar_t window_class[] = L"static";

static const PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

static SAMPLE_TYPE k4SoundBuffer[MAX_SAMPLES * 2];

WAVEHDR generate4klang()
{
    WAVEHDR k4WaveHDR =
        {
            (LPSTR)k4SoundBuffer,
            MAX_SAMPLES * sizeof(SAMPLE_TYPE) * 2, // MAX_SAMPLES*sizeof(float)*2(stereo)
            0,
            0,
            0,
            0,
            0,
            0};
    _4klang_render(k4SoundBuffer);
    return k4WaveHDR;
}

WAVEHDR generateTTS()
{
    ttsWavFormat.wFormatTag = WAVE_FORMAT_PCM;
    ttsWavFormat.nChannels = 2;
    ttsWavFormat.wBitsPerSample = 16;
    ttsWavFormat.nBlockAlign = ttsWavFormat.nChannels * ttsWavFormat.wBitsPerSample / 8;
    ttsWavFormat.nSamplesPerSec = 44100;
    ttsWavFormat.nAvgBytesPerSec = ttsWavFormat.nSamplesPerSec * ttsWavFormat.nBlockAlign;
    ttsWavFormat.cbSize = 0;

    // init voice
    ISpVoice *pVoice = NULL;
    CLSID _CLSID_SpVoice;
    CLSIDFromString(L"{96749377-3391-11D2-9EE3-00C04F797396}", &_CLSID_SpVoice);
    IID _IID_ISpVoice;
    CLSIDFromString(L"{6C44DF74-72B9-4992-A1EC-EF996E0422D4}", &_IID_ISpVoice);
    HRESULT hr = CoCreateInstance(_CLSID_SpVoice, NULL, CLSCTX_ALL, _IID_ISpVoice, (void **)&pVoice);

    // init stream to output to
    IStream *pStream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    CLSID _CLSID_SpStream;
    CLSIDFromString(L"{715D9C59-4442-11D2-9605-00C04F8EE628}", &_CLSID_SpStream);
    IID _IID_ISpStream;
    CLSIDFromString(L"{12E3CCA9-7518-44C5-A5E7-BA5A79CB929E}", &_IID_ISpStream);
    ISpStream *pSpStream = nullptr;
    hr = CoCreateInstance(_CLSID_SpStream, NULL, CLSCTX_ALL, _IID_ISpStream, (void **)&pSpStream);

    GUID _SPDFID_WaveFormatEx;
    IIDFromString(L"{C31ADBAE-527F-4FF5-A230-F62BB61FF70C}", (IID *)&_SPDFID_WaveFormatEx);

    // Speak in the stream
    hr = pSpStream->SetBaseStream(pStream, _SPDFID_WaveFormatEx, &ttsWavFormat);
    hr = pVoice->SetOutput(pSpStream, TRUE);
    pVoice->Speak(truc, SPF_IS_XML, NULL);

    // Dump stream in a buffer
    STATSTG stat;
    pStream->Stat(&stat, 0);
    unsigned char *data = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stat.cbSize.LowPart);
    LARGE_INTEGER li = {0, 0};
    pStream->Seek(li, SEEK_SET, NULL);
    unsigned long c = 0;
    pStream->Read(data, stat.cbSize.LowPart, &c);

    // Initialize the WAV data for the copypasta
    WAVEHDR WaveHDR = {(LPSTR)data, c, 0, 0, 0, 0, 0, 0};
    pSpStream->Release();
    pStream->Release();
    pVoice->Release();
    return WaveHDR;
}

static const char dummyText[] = "loading bitch";

static const float projectionMatrix[16] = {
    1.f, 0.f, 0.f, 0.f,
    0.f, 800.0 / 600.0, 0.f, 0.f, // 16/9 // 1.778f
    0.f, 0.f, -1.f, -1.f,
    0.f, 0.f, 0.f, 1.f};
static const float faceVerts[] = {
    1.f, -1.f,
    1.f, 1.f,
    -1.f, 1.f,
    -1.f, -1.f};
static const float faceCoords[] = {
    1.f, 0.f,
    1.f, 1.f,
    0.f, 1.f,
    0.f, 0.f};

int __stdcall WinMainCRTStartup()
{
    GLuint gShaderPresent;
    GLuint gShaderPostProcess;
    GLuint gShaderTex;

    GLuint fbAccumulator;
    GLuint texTexte;
    GLuint tex[2];

    int kScreenWidth = (SetProcessDPIAware(), GetSystemMetrics(SM_CXSCREEN));
    int kScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int kCanvasWidth = 800;
    int kCanvasHeight = 600;
    int kUniformResolution = 0;
    int kUniformTime = 1;

    CoInitialize(NULL);

    // window and stuff
    HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_VISIBLE | WS_POPUP, (kScreenWidth - 800) / 2, (kScreenHeight - 600) / 2, kCanvasWidth, kCanvasHeight, 0, 0, 0, 0));
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
    ShowCursor(false);

    // initalize opengl
    /*
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        3,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        1,
        WGL_CONTEXT_FLAGS_ARB,
        WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    };
    */

    HGLRC tempContext = wglCreateContext(hDC);
    wglMakeCurrent(hDC, tempContext);

    //HGLRC context = wglCreateContextAttribs(hDC, nullptr, attribs);
    //wglMakeCurrent(NULL, NULL);
    //wglDeleteContext(tempContext);
    //wglMakeCurrent(hDC, context);

    // Font & text
    HFONT font = CreateFont(32, // FIXME : ajuster une fois la police choisie
                            0, 0, 0, FW_MEDIUM,
                            FALSE, FALSE, FALSE, ANSI_CHARSET,
                            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, //PROOF_QUALITY,
                            FF_DONTCARE | DEFAULT_PITCH,
                            "Impact");
    SelectObject(hDC, font);
    unsigned int list = glGenLists(GLYPHS);
    wglUseFontBitmaps(hDC, FIRST_GLYPH, GLYPHS, list);

    glGenTextures(2, tex);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, kCanvasWidth, kCanvasHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, kCanvasWidth, kCanvasHeight);
    glEnable(GL_TEXTURE_2D);

    const unsigned int white = 0xffffffff;
    glColor3ubv((const GLubyte *)&white);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glRasterPos2f(0.0 - (13.0 * 32.0 / 800.0 / 2.0), 0.0);
    glCallLists(13, GL_BYTE, dummyText);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kCanvasWidth, kCanvasHeight);
    SwapBuffers(hDC);

    // Generate&render audio buffer for TTS and 4klang
    WAVEHDR WaveHDR = generateTTS();
    MMTIME MMTime = {TIME_BYTES, 0};
    HWAVEOUT k4WaveOut;
    WAVEHDR k4WaveHDR = generate4klang();

    // Play 4k song
    waveOutOpen(&k4WaveOut, WAVE_MAPPER, &k4WaveFMT, NULL, 0, CALLBACK_NULL);
    waveOutPrepareHeader(k4WaveOut, &k4WaveHDR, sizeof(k4WaveHDR));
    waveOutWrite(k4WaveOut, &k4WaveHDR, sizeof(k4WaveHDR));

    // Play the copypasta
    waveOutOpen(&ttsWaveOut, WAVE_MAPPER, &ttsWavFormat, NULL, 0, CALLBACK_NULL);
    waveOutPrepareHeader(ttsWaveOut, &WaveHDR, sizeof(WaveHDR));
    waveOutWrite(ttsWaveOut, &WaveHDR, sizeof(WaveHDR));

    // Make framebuffer
    glViewport(0, 0, kCanvasWidth, kScreenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glGenFramebuffers(1, &fbAccumulator);
    glBindFramebuffer(GL_FRAMEBUFFER, fbAccumulator);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kCanvasWidth, kCanvasHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[0], 0);
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    // make shader
    gShaderPresent = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &draw_frag);
    gShaderPostProcess = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &postprocess_frag);
    gShaderTex = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &tex_frag);

    // main loop : Play the WAV until finished.
    bool finished = false;
    int t;
    int to = timeGetTime();
    do
    {
        t = timeGetTime() - to;
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        //waveOutGetPosition(k4WaveOut, &MMTime, sizeof(MMTIME));

        glUseProgram(gShaderPresent);
        glBindFramebuffer(GL_FRAMEBUFFER, fbAccumulator);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glRecti(-1, -1, 1, 1);

        glUseProgram(gShaderPostProcess);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glBindTexture(GL_TEXTURE_2D, fbAccumulator);
        glRecti(-1, -1, 1, 1);

        /*
        glUseProgram(gShaderTex);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glBindTexture(GL_TEXTURE_2D, tex[1]);
        glRecti(-1, -1, 1, 1);
        */

        SwapBuffers(hDC);
        processMessages();
        // finished = !(MMTime.u.cb < c);
    } while (!finished && !GetAsyncKeyState(VK_ESCAPE));

    CoUninitialize();
    ShowCursor(true);
    ExitProcess(0);
    return 0;
}
