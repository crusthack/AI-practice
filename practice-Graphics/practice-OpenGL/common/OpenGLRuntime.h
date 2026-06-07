#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <gl/GL.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_COMPUTE_SHADER 0x91B9
#define GL_SHADER_STORAGE_BARRIER_BIT 0x2000
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#define GL_RGBA8 0x8058
#define GL_WRITE_ONLY 0x88B9
#define GL_DEPTH_COMPONENT 0x1902
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_LEQUAL 0x0203
#define GL_MULTISAMPLE 0x809D
#endif

#ifndef GLchar
using GLchar = char;
#endif

namespace OpenGLPractice
{
using WglCreateContextAttribsARB = HGLRC(APIENTRY*)(HDC, HGLRC, const int*);
using WglSwapIntervalEXT = BOOL(APIENTRY*)(int);

inline WglCreateContextAttribsARB wglCreateContextAttribsARB_ = nullptr;
inline WglSwapIntervalEXT wglSwapIntervalEXT_ = nullptr;

#define OPENGL_PROC_LIST(X) \
    X(void, glGenVertexArrays, (GLsizei, GLuint*)) \
    X(void, glBindVertexArray, (GLuint)) \
    X(void, glDeleteVertexArrays, (GLsizei, const GLuint*)) \
    X(void, glGenBuffers, (GLsizei, GLuint*)) \
    X(void, glBindBuffer, (GLenum, GLuint)) \
    X(void, glBufferData, (GLenum, ptrdiff_t, const void*, GLenum)) \
    X(void, glBufferSubData, (GLenum, ptrdiff_t, ptrdiff_t, const void*)) \
    X(void, glDeleteBuffers, (GLsizei, const GLuint*)) \
    X(void, glEnableVertexAttribArray, (GLuint)) \
    X(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)) \
    X(void, glVertexAttribDivisor, (GLuint, GLuint)) \
    X(GLuint, glCreateShader, (GLenum)) \
    X(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint*)) \
    X(void, glCompileShader, (GLuint)) \
    X(void, glGetShaderiv, (GLuint, GLenum, GLint*)) \
    X(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
    X(void, glDeleteShader, (GLuint)) \
    X(GLuint, glCreateProgram, ()) \
    X(void, glAttachShader, (GLuint, GLuint)) \
    X(void, glLinkProgram, (GLuint)) \
    X(void, glGetProgramiv, (GLuint, GLenum, GLint*)) \
    X(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
    X(void, glUseProgram, (GLuint)) \
    X(void, glDeleteProgram, (GLuint)) \
    X(GLint, glGetUniformLocation, (GLuint, const GLchar*)) \
    X(void, glUniform1f, (GLint, GLfloat)) \
    X(void, glUniform1i, (GLint, GLint)) \
    X(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat)) \
    X(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*)) \
    X(void, glActiveTexture, (GLenum)) \
    X(void, glGenFramebuffers, (GLsizei, GLuint*)) \
    X(void, glBindFramebuffer, (GLenum, GLuint)) \
    X(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint)) \
    X(GLenum, glCheckFramebufferStatus, (GLenum)) \
    X(void, glDeleteFramebuffers, (GLsizei, const GLuint*)) \
    X(void, glGenRenderbuffers, (GLsizei, GLuint*)) \
    X(void, glBindRenderbuffer, (GLenum, GLuint)) \
    X(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei, GLsizei)) \
    X(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint)) \
    X(void, glDeleteRenderbuffers, (GLsizei, const GLuint*)) \
    X(void, glDrawElementsInstanced, (GLenum, GLsizei, GLenum, const void*, GLsizei)) \
    X(void, glBindBufferBase, (GLenum, GLuint, GLuint)) \
    X(GLuint, glGetUniformBlockIndex, (GLuint, const GLchar*)) \
    X(void, glUniformBlockBinding, (GLuint, GLuint, GLuint)) \
    X(void, glTexImage3D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)) \
    X(void, glDispatchCompute, (GLuint, GLuint, GLuint)) \
    X(void, glMemoryBarrier, (GLbitfield)) \
    X(void, glBindImageTexture, (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum)) \
    X(void, glTexStorage2D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei))

#define DECLARE_GL_PROC(ret, name, args) using name##Proc = ret(APIENTRY*) args; inline name##Proc name = nullptr;
OPENGL_PROC_LIST(DECLARE_GL_PROC)
#undef DECLARE_GL_PROC

inline void* LoadAnyGLProc(const char* name)
{
    void* p = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (p == nullptr || p == reinterpret_cast<void*>(0x1) || p == reinterpret_cast<void*>(0x2) ||
        p == reinterpret_cast<void*>(0x3) || p == reinterpret_cast<void*>(-1))
    {
        HMODULE module = GetModuleHandleW(L"opengl32.dll");
        p = reinterpret_cast<void*>(GetProcAddress(module, name));
    }
    return p;
}

inline void LoadOpenGLFunctions()
{
#define LOAD_GL_PROC(ret, name, args) \
    name = reinterpret_cast<name##Proc>(LoadAnyGLProc(#name)); \
    if (!name && std::strcmp(#name, "glDispatchCompute") != 0 && std::strcmp(#name, "glMemoryBarrier") != 0 && std::strcmp(#name, "glBindImageTexture") != 0 && std::strcmp(#name, "glTexStorage2D") != 0) throw std::runtime_error("Missing OpenGL function: " #name);
OPENGL_PROC_LIST(LOAD_GL_PROC)
#undef LOAD_GL_PROC
}

inline void LoadWglExtensions()
{
    wglCreateContextAttribsARB_ = reinterpret_cast<WglCreateContextAttribsARB>(LoadAnyGLProc("wglCreateContextAttribsARB"));
    wglSwapIntervalEXT_ = reinterpret_cast<WglSwapIntervalEXT>(LoadAnyGLProc("wglSwapIntervalEXT"));
}

inline std::string WideToUtf8(const wchar_t* text)
{
    if (!text)
        return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<size_t>(size > 0 ? size - 1 : 0), '\0');
    if (size > 1)
        WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
    return result;
}

struct Mat4
{
    float m[16] = {};
};

inline Mat4 Identity()
{
    Mat4 r = {};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

inline Mat4 Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 r = {};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            for (int k = 0; k < 4; ++k)
                r.m[c * 4 + row] += a.m[k * 4 + row] * b.m[c * 4 + k];
    return r;
}

inline Mat4 Translate(float x, float y, float z)
{
    Mat4 r = Identity();
    r.m[12] = x;
    r.m[13] = y;
    r.m[14] = z;
    return r;
}

inline Mat4 Scale(float x, float y, float z)
{
    Mat4 r = Identity();
    r.m[0] = x;
    r.m[5] = y;
    r.m[10] = z;
    return r;
}

inline Mat4 RotateY(float a)
{
    Mat4 r = Identity();
    const float c = std::cos(a);
    const float s = std::sin(a);
    r.m[0] = c;
    r.m[2] = -s;
    r.m[8] = s;
    r.m[10] = c;
    return r;
}

inline Mat4 Perspective(float fovy, float aspect, float nearZ, float farZ)
{
    Mat4 r = {};
    const float f = 1.0f / std::tan(fovy * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (farZ + nearZ) / (nearZ - farZ);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return r;
}

inline Mat4 LookAt(float ex, float ey, float ez, float tx, float ty, float tz)
{
    float fx = tx - ex, fy = ty - ey, fz = tz - ez;
    float fl = std::sqrt(fx * fx + fy * fy + fz * fz);
    fx /= fl; fy /= fl; fz /= fl;

    float sx = fy * 0.0f - fz * 1.0f;
    float sy = fz * 0.0f - fx * 0.0f;
    float sz = fx * 1.0f - fy * 0.0f;
    float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    sx /= sl; sy /= sl; sz /= sl;

    const float ux = sy * fz - sz * fy;
    const float uy = sz * fx - sx * fz;
    const float uz = sx * fy - sy * fx;

    Mat4 r = Identity();
    r.m[0] = sx;  r.m[4] = sy;  r.m[8] = sz;
    r.m[1] = ux;  r.m[5] = uy;  r.m[9] = uz;
    r.m[2] = -fx; r.m[6] = -fy; r.m[10] = -fz;
    r.m[12] = -(sx * ex + sy * ey + sz * ez);
    r.m[13] = -(ux * ex + uy * ey + uz * ez);
    r.m[14] = fx * ex + fy * ey + fz * ez;
    return r;
}

inline GLuint CompileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error(log);
    }
    return shader;
}

inline GLuint LinkProgram(const char* vs, const char* fs, const char* gs = nullptr, const char* cs = nullptr)
{
    GLuint program = glCreateProgram();
    std::vector<GLuint> shaders;
    if (vs) shaders.push_back(CompileShader(GL_VERTEX_SHADER, vs));
    if (fs) shaders.push_back(CompileShader(GL_FRAGMENT_SHADER, fs));
    if (gs) shaders.push_back(CompileShader(GL_GEOMETRY_SHADER, gs));
    if (cs) shaders.push_back(CompileShader(GL_COMPUTE_SHADER, cs));
    for (GLuint shader : shaders)
        glAttachShader(program, shader);
    glLinkProgram(program);
    for (GLuint shader : shaders)
        glDeleteShader(shader);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetProgramInfoLog(program, len, nullptr, log.data());
        glDeleteProgram(program);
        throw std::runtime_error(log);
    }
    return program;
}

inline void SetObjectName(HWND hwnd, const wchar_t* title)
{
    SetWindowTextW(hwnd, title);
}

} // namespace OpenGLPractice
