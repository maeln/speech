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

//#define TRES 512
#define FIRST_GLYPH 1
#define GLYPHS 126
#define FONT_BOTTOM_COMPENSATION 0.005
#define FONT_SIZE 40

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

const wchar_t loading_text[] = L"Generating a bunch of stuff ... Give it a few seconds";
const int loading_text_len = 54;

const wchar_t voice_start[] = L"<voice required=\"Language=409\">";
const wchar_t voice_end[] = L"</voice>";

const wchar_t msg1[] =
    L"Farewell Zenika !";
const int msg_1_len = 17;

const wchar_t msg2[] =
    L"Thanks to all of you!";
const int msg_2_len = 21;

const wchar_t msg3[] =
    L"And special thanks to the Nantes Team";
const int msg_3_len = 37;

const wchar_t msg4[] =
    L"So long and thanks for all the fish!";
const int msg_4_len = 36;

const wchar_t window_class[] = L"static";

static const PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

static SAMPLE_TYPE k4SoundBuffer[MAX_SAMPLES * 2];
static wchar_t textBuffer[256];

void text_2_tts_eng(const wchar_t msg[], int len)
{
    int n = 0;
    for (int i = 0; i < 32; ++i)
    {
        textBuffer[n] = voice_start[i];
        n++;
    }
    n--;

    for (int i = 0; i < len; ++i)
    {
        textBuffer[n] = msg[i];
        n++;
    }
    n--;

    for (int i = 0; i < 9; ++i)
    {
        textBuffer[n] = voice_end[i];
        n++;
    }
    n--;

    textBuffer[n] = '\0';
}

float pow(float x, int i)
{
    float p = x;
    for (int n = 1; n < i; ++n)
    {
        p *= p;
    }
    return p;
}

float easeInOutQuint(float x)
{
    return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
}

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

WAVEHDR generateTTS(const wchar_t msg[], int msg_len, int id)
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
    text_2_tts_eng(msg, msg_len);
    pVoice->Speak(textBuffer, SPF_IS_XML, NULL);

    // Dump stream in a buffer
    STATSTG stat;
    pStream->Stat(&stat, 0);
    unsigned char *data = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stat.cbSize.LowPart);
    LARGE_INTEGER li = {0, 0};
    pStream->Seek(li, SEEK_SET, NULL);
    unsigned long c = 0;
    pStream->Read(data, stat.cbSize.LowPart, &c);
    ttsBytes[id] = c;

    // Initialize the WAV data for the copypasta
    WAVEHDR WaveHDR = {(LPSTR)data, c, 0, 0, 0, 0, 0, 0};
    pSpStream->Release();
    pStream->Release();
    pVoice->Release();
    return WaveHDR;
}

const float time_ease = 1000;
const float midX = 0.0;
const float initX = -2.0;
const float endX = 1.1;
float mix(float x, float y, float a)
{
    return x * (1.0 - a) + y * a;
}

float calcXPos(float currTime, float breakTime, int id)
{
    float funit = FONT_SIZE / 800.0;
    if (breakTime == 0)
    {
        float r = currTime / time_ease;
        r = r > 1.0 ? 1.0 : r;
        float ease = easeInOutQuint(r);
        float target = midX;
        switch (id)
        {
        case 0:
            target -= (msg_1_len * funit) / 2.0;
            break;
        case 1:
            target -= (msg_2_len * funit) / 2.0;
            break;
        case 2:
            target -= (msg_3_len * funit) / 2.0;
            break;
        default:
            target -= (msg_4_len * funit) / 2.0;
            break;
        }
        return mix(initX, target, ease);
    }
    else
    {
        float r = breakTime / 500.0;
        r = r > 1.0 ? 1.0 : r;
        float ease = easeInOutQuint(r);
        float origin = midX;
        switch (id)
        {
        case 0:
            origin -= (msg_1_len * funit) / 2.0;
            break;
        case 1:
            origin -= (msg_2_len * funit) / 2.0;
            break;
        case 2:
            origin -= (msg_3_len * funit) / 2.0;
            break;
        default:
            origin -= (msg_4_len * funit) / 2.0;
            break;
        }
        return mix(origin, endX, ease);
    }
}

int __stdcall WinMainCRTStartup()
{
    GLuint gShaderPresent;
    GLuint gShaderPostProcess;
    GLuint gShaderTex;

    GLuint fbAccumulator;
    GLuint fbTextSDF;
    GLuint accumulatorTex;
    GLuint sdfTex;

    int kScreenWidth = (SetProcessDPIAware(), GetSystemMetrics(SM_CXSCREEN));
    int kScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int kCanvasWidth = 800;
    int kCanvasHeight = 600;
    int kUniformResolution = 0;
    int kUniformTime = 1;

    CoInitialize(NULL);

    // window and stuff
    HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_VISIBLE | WS_POPUP, (kScreenWidth - kCanvasWidth) / 2, (kScreenHeight - kCanvasHeight) / 2, kCanvasWidth, kCanvasHeight, 0, 0, 0, 0));
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
    SwapBuffers(hDC);

    glViewport(0, 0, kCanvasWidth, kScreenHeight);
    glClearColor(0, 0, 0, 0);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &accumulatorTex);
    glGenTextures(1, &sdfTex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // make shader
    gShaderPresent = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &draw_frag);
    gShaderPostProcess = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &postprocess_frag);
    gShaderTex = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &tex_frag);

    // get sampler uniforms location
    GLint kPostProcSampler = glGetUniformLocation(gShaderPostProcess, VAR_ACCTEX);
    GLint kTextSampler = glGetUniformLocation(gShaderTex, VAR_SDFTEX);

    // Create the framebuffers
    glClear(GL_COLOR_BUFFER_BIT);
    glGenFramebuffers(1, &fbAccumulator);
    glBindFramebuffer(GL_FRAMEBUFFER, fbAccumulator);
    glBindTexture(GL_TEXTURE_2D, accumulatorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kCanvasWidth, kCanvasHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, accumulatorTex, 0);
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    glClear(GL_COLOR_BUFFER_BIT);
    glGenFramebuffers(1, &fbTextSDF);
    glBindFramebuffer(GL_FRAMEBUFFER, fbTextSDF);
    glBindTexture(GL_TEXTURE_2D, sdfTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kCanvasWidth, kCanvasHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sdfTex, 0);
    GLenum sdfDrawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, sdfDrawBuffers);

    // Font & text
    HFONT font = CreateFont(FONT_SIZE, // FIXME : ajuster une fois la police choisie
                            0, 0, 0, FW_MEDIUM,
                            FALSE, FALSE, FALSE, ANSI_CHARSET,
                            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, //PROOF_QUALITY,
                            FF_DONTCARE | DEFAULT_PITCH,
                            "Impact");
    SelectObject(hDC, font);
    unsigned int list = glGenLists(GLYPHS);
    wglUseFontBitmaps(hDC, FIRST_GLYPH, GLYPHS, list);

    glBindFramebuffer(GL_FRAMEBUFFER, fbTextSDF);
    const unsigned int white = 0xffffffff;
    glColor3ubv((const GLubyte *)&white);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, 0);

    glRasterPos2f(-0.9, -0.2);
    glCallLists(54 * 2, GL_UNSIGNED_BYTE, loading_text);

    float unitX32 = 32.0 / kCanvasWidth * 2.0;
    float unitY32 = 32.0 / kCanvasHeight;
    int maxUnitWidth = kCanvasWidth / 32.0;

    /*
    for (int i = 0; i < 128; ++i)
    {
        glPushMatrix();
        float xPos = (float)(i % 15);
        float yPos = (float)(i / 15);
        glRasterPos2f(-1.0 + unitX32 * (i % maxUnitWidth), -1.0 + (FONT_BOTTOM_COMPENSATION) + unitY32 * (i / maxUnitWidth));
        glCallLists(1, GL_UNSIGNED_BYTE, &textBuffer[i]);
        glPopMatrix();
    }
    */

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gShaderTex);
    glUniform4f(
        kUniformResolution,
        (float)kCanvasWidth,
        (float)kCanvasHeight,
        (float)kCanvasWidth / (float)kCanvasHeight,
        (float)kCanvasHeight / (float)kCanvasWidth);
    glUniform1i(kUniformTime, 0);
    glUniform1i(kTextSampler, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sdfTex);
    glRecti(-1, -1, 1, 1);

    SwapBuffers(hDC);

    // Generate & render audio buffer for TTS and 4klang
    int now_playing = 0;
    WAVEHDR tts_msgs[4];
    tts_msgs[0] = generateTTS(msg1, msg_1_len, 0);
    tts_msgs[1] = generateTTS(msg2, msg_2_len, 1);
    tts_msgs[2] = generateTTS(msg3, msg_3_len, 2);
    tts_msgs[3] = generateTTS(msg4, msg_4_len, 3);
    MMTIME MMTime = {TIME_BYTES, 0};

    WAVEHDR k4WaveHDR = generate4klang();
    HWAVEOUT k4WaveOut;

    // prepare 4k song
    waveOutOpen(&k4WaveOut, WAVE_MAPPER, &k4WaveFMT, NULL, 0, CALLBACK_NULL);
    waveOutPrepareHeader(k4WaveOut, &k4WaveHDR, sizeof(k4WaveHDR));

    // play song
    waveOutWrite(k4WaveOut, &k4WaveHDR, sizeof(k4WaveHDR));

    // main loop : Play the WAV until finished.
    bool playing = false;
    int t;
    int to = timeGetTime();
    int break_time = 0;
    int dt = 0;
    int animationTimer = 0;
    int ttsDelay = 0;
    bool rstDelay = false;
    do
    {
        int start = timeGetTime();
        t = timeGetTime() - to;
        animationTimer += dt;
        ttsDelay += dt;

        if (ttsDelay > 500 && !playing)
        {
            waveOutOpen(&ttsWaveOut, WAVE_MAPPER, &ttsWavFormat, NULL, 0, CALLBACK_NULL);
            waveOutPrepareHeader(ttsWaveOut, &tts_msgs[now_playing], sizeof(tts_msgs[now_playing]));
            waveOutWrite(ttsWaveOut, &tts_msgs[now_playing], sizeof(tts_msgs[now_playing]));
            playing = true;
            rstDelay = true;
        }

        waveOutGetPosition(ttsWaveOut, &MMTime, sizeof(MMTIME));
        if (now_playing < 4 && MMTime.u.cb >= ttsBytes[now_playing])
        {
            if (break_time > 1000)
            {
                if (rstDelay)
                {
                    playing = false;
                    break_time = 0;
                    animationTimer = 0;
                    now_playing++;

                    ttsDelay = 0;
                    rstDelay = false;
                }
            }
            else
            {
                break_time += dt;
            }
        }

        // Draw the text
        glBindFramebuffer(GL_FRAMEBUFFER, fbTextSDF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3ubv((const GLubyte *)&white);
        glBindTexture(GL_TEXTURE_2D, 0);

        glRasterPos2f(calcXPos(animationTimer, break_time, now_playing), -0.47);
        if (now_playing == 0)
        {
            glCallLists(msg_1_len * 2, GL_UNSIGNED_BYTE, msg1);
        }
        else if (now_playing == 1)
        {
            glCallLists(msg_2_len * 2, GL_UNSIGNED_BYTE, msg2);
        }
        else if (now_playing == 2)
        {
            glCallLists(msg_3_len * 2, GL_UNSIGNED_BYTE, msg3);
        }
        else if (now_playing == 3)
        {
            glCallLists(msg_4_len * 2, GL_UNSIGNED_BYTE, msg4);
        }

        // Draw the main scene
        glBindFramebuffer(GL_FRAMEBUFFER, fbAccumulator);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(gShaderPresent);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glRecti(-1, -1, 1, 1);

        // Supperpose the text
        glUseProgram(gShaderTex);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glUniform1i(kTextSampler, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sdfTex);
        glRecti(-1, -1, 1, 1);

        // Post process
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(gShaderPostProcess);
        glUniform4f(
            kUniformResolution,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kUniformTime, t);
        glUniform1i(kPostProcSampler, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumulatorTex);
        glRecti(-1, -1, 1, 1);

        SwapBuffers(hDC);
        processMessages();
        dt = timeGetTime() - start;
    } while (!GetAsyncKeyState(VK_ESCAPE));

    CoUninitialize();
    ShowCursor(true);
    ExitProcess(0);
    return 0;
}
