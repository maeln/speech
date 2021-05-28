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

// a shader that display the content of a texture that is the same size has the viewport.
#define VAR_IRESOLUTION "i"
#define VAR_ITIME "v"
#define VAR_SDFTEX "l"
const char *tex_frag =
    "#version 430\n"
    "layout(location=0)uniform vec4 i;"
    "uniform sampler2D l;"
    "void main()"
    "{"
    "vec4 i=texelFetch(l,ivec2(gl_FragCoord.xy),0);"
    "i.rgb = vec3(1.0,0.0,0.0);"
    "gl_FragColor=i;"
    "}";

//#define TRES 512
#define FIRST_GLYPH 1
#define GLYPHS 'z'
#define FONT_BOTTOM_COMPENSATION 0.005

const wchar_t window_class[] = L"static";

static const PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};

static const char dummyText[] = "abcdefghijklmnopqrstuvwxyz1234567890,.<>?!/";
static const char other[] = "lolilolpdtr";

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

// CMD to compile this mess :
// cl -nologo -Gm- -GR- -EHa- -Oi -GS- -Gs99999999 text.cpp -link -stack:0x5000000,0x5000000 -subsystem:windows -nodefaultlib "kernel32.lib" "ole32.lib" "winmm.lib" "user32.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"

int __stdcall WinMainCRTStartup()
{
    // Shader that display the texture
    GLuint gShaderTex;
    // Framebuffer in which we draw the text
    GLuint fbText;
    // Texture that back the framebuffer
    GLuint texText;

    // window stuff
    SetProcessDPIAware();
    int kScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int kScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int kCanvasWidth = 800;
    int kCanvasHeight = 600;

    HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_VISIBLE | WS_POPUP, (kScreenWidth - 800) / 2, (kScreenHeight - 600) / 2, kCanvasWidth, kCanvasHeight, 0, 0, 0, 0));
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);

    HGLRC tempContext = wglCreateContext(hDC);
    wglMakeCurrent(hDC, tempContext);
    SwapBuffers(hDC);

    // Init the classic opengl stuff
    glViewport(0, 0, kCanvasWidth, kScreenHeight);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Generate the texture for the framebuffer
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texText);

    // Compile the shader and get the location of the sampler
    gShaderTex = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &tex_frag);
    GLint kTextSampler = glGetUniformLocation(gShaderTex, VAR_SDFTEX);

    // Create the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glGenFramebuffers(1, &fbText);
    glBindFramebuffer(GL_FRAMEBUFFER, fbText);
    glBindTexture(GL_TEXTURE_2D, texText);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kCanvasWidth, kCanvasHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texText, 0);
    GLenum textDrawBuffer[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, textDrawBuffer);

    // Create the font to be used by opengl
    HFONT font = CreateFont(32,
                            0, 0, 0, FW_MEDIUM,
                            FALSE, FALSE, FALSE, ANSI_CHARSET,
                            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY,
                            FF_DONTCARE | DEFAULT_PITCH,
                            "Impact");
    SelectObject(hDC, font);
    unsigned int list = glGenLists(GLYPHS);
    wglUseFontBitmaps(hDC, FIRST_GLYPH, GLYPHS, list);

    glBindFramebuffer(GL_FRAMEBUFFER, fbText);
    const unsigned int white = 0xffffffff;
    glColor3ubv((const GLubyte *)&white);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_TEXTURE_2D);
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0); // Q1: Why is this necessary for the text to be displayed ?

    float unitX32 = 32.0 / kCanvasWidth * 2.0;
    float unitY32 = 32.0 / kCanvasHeight;
    int maxUnitWidth = kCanvasWidth / 32.0;

    for (int i = 0; i < 43; ++i)
    {
        glPushAttrib(GL_LIST_BIT);
        glListBase(0);
        glPushMatrix();
        float xPos = (float)(i % 15);
        float yPos = (float)(i / 15);
        glRasterPos2f(-1.0 + unitX32 * (i % maxUnitWidth), -1.0 + (FONT_BOTTOM_COMPENSATION) + unitY32 * (i / maxUnitWidth));
        glCallLists(1, GL_UNSIGNED_BYTE, &dummyText[i]);
        glPopMatrix();
        glPopAttrib();
    }

    //glBindTexture(GL_TEXTURE_2D, texText);
    //glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kCanvasWidth, kCanvasHeight);

    // Display a full-screen rectangle and use the shader to display the {texText} texture content.
    /*
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gShaderTex);
    glUniform4f(
        0,
        (float)kCanvasWidth,
        (float)kCanvasHeight,
        (float)kCanvasWidth / (float)kCanvasHeight,
        (float)kCanvasHeight / (float)kCanvasWidth);
    glUniform1i(kTextSampler, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texText);
    glRecti(-1, -1, 1, 1);
    */

    SwapBuffers(hDC);

    // just some busy waiting to show that it is indeed diaplying the first time.
    int to = timeGetTime() + 1500;
    while (to > timeGetTime())
    {
        // busy waiting.
    }

    // Now make a rendering loop where we change the position of the text
    // The problem is, from this point on, we seem to not be able to draw the text anymore.
    int time = timeGetTime();
    int dt;
    int l = 0;
    do
    {
        dt = timeGetTime() - time;
        time = timeGetTime();

        glBindFramebuffer(GL_FRAMEBUFFER, fbText);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor3ubv((const GLubyte *)&white);
        glBindTexture(GL_TEXTURE_2D, 0);

        for (int i = 0; i < 11; ++i)
        {
            glPushAttrib(GL_LIST_BIT);
            glListBase(0);
            glPushMatrix();
            float xPos = (float)(i % 15);
            float yPos = (float)(i / 15);
            glRasterPos2f(-1.0 + unitX32 * (i % maxUnitWidth), -1.0 + (FONT_BOTTOM_COMPENSATION) + unitY32 * (i / maxUnitWidth));
            glCallLists(1, GL_UNSIGNED_BYTE, &other[i + l]);
            glPopMatrix();
            glPopAttrib();
        }

        glBindTexture(GL_TEXTURE_2D, texText);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kCanvasWidth, kCanvasHeight);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Display a full-screen rectangle and use the shader to display the {texText} texture content.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(gShaderTex);
        glUniform4f(
            0,
            (float)kCanvasWidth,
            (float)kCanvasHeight,
            (float)kCanvasWidth / (float)kCanvasHeight,
            (float)kCanvasHeight / (float)kCanvasWidth);
        glUniform1i(kTextSampler, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texText);
        glRecti(-1, -1, 1, 1);

        SwapBuffers(hDC);
        processMessages();
        ++l;
    } while (!GetAsyncKeyState(VK_ESCAPE));

    ExitProcess(0);
    return 0;
}