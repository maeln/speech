#pragma once

#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define ERROR_INVALID_VERSION_ARB 0x2095
typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
#define WGL_ARB_create_context_profile 1
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define ERROR_INVALID_PROFILE_ARB 0x2096
#define wglCreateContextAttribs ((PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB"))

#define glAttachShader ((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))
#define glBindFramebuffer ((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))
#define glCompileShader ((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))
#define glCreateProgram ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))
#define glCreateShader ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))
#define glCreateShaderProgramv ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))
#define glDebugMessageCallback ((PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback"))
#define glDeleteProgram ((PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram"))
#define glDeleteShader ((PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader"))
#define glDrawBuffers ((PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers"))
#define glFramebufferTexture ((PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture"))
#define glGenFramebuffers ((PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers"))
#define glGetProgramInfoLog ((PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog"))
#define glGetProgramiv ((PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv"))
#define glGetShaderInfoLog ((PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog"))
#define glGetShaderiv ((PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv"))
#define glGetUniformLocation ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))
#define glLinkProgram ((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))
#define glShaderSource ((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))
#define glUniform1i ((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))
#define glUniform2i ((PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i"))
#define glUniform3i ((PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i"))
#define glUniform4i ((PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i"))
#define glUniform1f ((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))
#define glUniform2f ((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))
#define glUniform3f ((PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f"))
#define glUniform4f ((PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f"))
#define glUseProgram ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))
#define glActiveTexture ((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))
