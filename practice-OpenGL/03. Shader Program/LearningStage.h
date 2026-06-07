#pragma once

// Learning goal: compile GLSL shaders, link a program, and render with it.
// Implementation status: Implemented.
//
// This sample still submits vertices through compatibility immediate mode.
// Buffer objects begin in the next sample, so this lesson can focus on shader
// object creation, compile/link validation, binding, and cleanup.

#include <windows.h>
#include <gl/GL.h>

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

using GLchar = char;
using PFNGLCREATESHADERPROC = GLuint(APIENTRY*)(GLenum type);
using PFNGLSHADERSOURCEPROC = void(APIENTRY*)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
using PFNGLCOMPILESHADERPROC = void(APIENTRY*)(GLuint shader);
using PFNGLGETSHADERIVPROC = void(APIENTRY*)(GLuint shader, GLenum pname, GLint* params);
using PFNGLGETSHADERINFOLOGPROC = void(APIENTRY*)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
using PFNGLDELETESHADERPROC = void(APIENTRY*)(GLuint shader);
using PFNGLCREATEPROGRAMPROC = GLuint(APIENTRY*)();
using PFNGLATTACHSHADERPROC = void(APIENTRY*)(GLuint program, GLuint shader);
using PFNGLLINKPROGRAMPROC = void(APIENTRY*)(GLuint program);
using PFNGLGETPROGRAMIVPROC = void(APIENTRY*)(GLuint program, GLenum pname, GLint* params);
using PFNGLGETPROGRAMINFOLOGPROC = void(APIENTRY*)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
using PFNGLUSEPROGRAMPROC = void(APIENTRY*)(GLuint program);
using PFNGLDELETEPROGRAMPROC = void(APIENTRY*)(GLuint program);

struct LearningStageSetupContext
{
    int Width = 0;
    int Height = 0;
};

struct LearningStageRenderContext
{
    int Width = 0;
    int Height = 0;
};

struct ShaderProgramApi
{
    PFNGLCREATESHADERPROC CreateShader = nullptr;
    PFNGLSHADERSOURCEPROC ShaderSource = nullptr;
    PFNGLCOMPILESHADERPROC CompileShader = nullptr;
    PFNGLGETSHADERIVPROC GetShaderiv = nullptr;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog = nullptr;
    PFNGLDELETESHADERPROC DeleteShader = nullptr;
    PFNGLCREATEPROGRAMPROC CreateProgram = nullptr;
    PFNGLATTACHSHADERPROC AttachShader = nullptr;
    PFNGLLINKPROGRAMPROC LinkProgram = nullptr;
    PFNGLGETPROGRAMIVPROC GetProgramiv = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog = nullptr;
    PFNGLUSEPROGRAMPROC UseProgram = nullptr;
    PFNGLDELETEPROGRAMPROC DeleteProgram = nullptr;
};

struct LearningStageState
{
    ShaderProgramApi Gl = {};
    GLuint Program = 0;
    float ClearColor[4] = { 0.035f, 0.050f, 0.070f, 1.0f };
    float Pulse = 0.0f;
};

inline PROC LoadRequiredOpenGLProc(const char* name)
{
    PROC proc = wglGetProcAddress(name);
    if (!proc)
    {
        throw std::runtime_error(std::string("wglGetProcAddress failed for ") + name + ".");
    }

    return proc;
}

template <typename T>
inline T LoadRequiredOpenGLProcAs(const char* name)
{
    return reinterpret_cast<T>(LoadRequiredOpenGLProc(name));
}

inline void LoadShaderProgramApi(ShaderProgramApi& gl)
{
    gl.CreateShader = LoadRequiredOpenGLProcAs<PFNGLCREATESHADERPROC>("glCreateShader");
    gl.ShaderSource = LoadRequiredOpenGLProcAs<PFNGLSHADERSOURCEPROC>("glShaderSource");
    gl.CompileShader = LoadRequiredOpenGLProcAs<PFNGLCOMPILESHADERPROC>("glCompileShader");
    gl.GetShaderiv = LoadRequiredOpenGLProcAs<PFNGLGETSHADERIVPROC>("glGetShaderiv");
    gl.GetShaderInfoLog = LoadRequiredOpenGLProcAs<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog");
    gl.DeleteShader = LoadRequiredOpenGLProcAs<PFNGLDELETESHADERPROC>("glDeleteShader");
    gl.CreateProgram = LoadRequiredOpenGLProcAs<PFNGLCREATEPROGRAMPROC>("glCreateProgram");
    gl.AttachShader = LoadRequiredOpenGLProcAs<PFNGLATTACHSHADERPROC>("glAttachShader");
    gl.LinkProgram = LoadRequiredOpenGLProcAs<PFNGLLINKPROGRAMPROC>("glLinkProgram");
    gl.GetProgramiv = LoadRequiredOpenGLProcAs<PFNGLGETPROGRAMIVPROC>("glGetProgramiv");
    gl.GetProgramInfoLog = LoadRequiredOpenGLProcAs<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog");
    gl.UseProgram = LoadRequiredOpenGLProcAs<PFNGLUSEPROGRAMPROC>("glUseProgram");
    gl.DeleteProgram = LoadRequiredOpenGLProcAs<PFNGLDELETEPROGRAMPROC>("glDeleteProgram");
}

inline std::string ReadShaderInfoLog(const ShaderProgramApi& gl, GLuint shader)
{
    GLint length = 0;
    gl.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1)
    {
        return "No shader compiler log was returned.";
    }

    std::vector<GLchar> log(static_cast<size_t>(length), '\0');
    gl.GetShaderInfoLog(shader, length, nullptr, log.data());
    return std::string(log.data());
}

inline std::string ReadProgramInfoLog(const ShaderProgramApi& gl, GLuint program)
{
    GLint length = 0;
    gl.GetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1)
    {
        return "No program linker log was returned.";
    }

    std::vector<GLchar> log(static_cast<size_t>(length), '\0');
    gl.GetProgramInfoLog(program, length, nullptr, log.data());
    return std::string(log.data());
}

inline GLuint CompileShader(const ShaderProgramApi& gl, GLenum type, const char* source, const char* debugName)
{
    const GLuint shader = gl.CreateShader(type);
    if (shader == 0)
    {
        throw std::runtime_error(std::string("glCreateShader failed for ") + debugName + ".");
    }

    gl.ShaderSource(shader, 1, &source, nullptr);
    gl.CompileShader(shader);

    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        const std::string log = ReadShaderInfoLog(gl, shader);
        gl.DeleteShader(shader);
        throw std::runtime_error(std::string(debugName) + " compilation failed:\n" + log);
    }

    return shader;
}

inline GLuint CreateTriangleProgram(const ShaderProgramApi& gl)
{
    const char* vertexShaderSource =
        "#version 120\n"
        "varying vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = gl_Vertex;\n"
        "    vColor = gl_Color.rgb;\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 120\n"
        "varying vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n";

    const GLuint vertexShader = CompileShader(gl, GL_VERTEX_SHADER, vertexShaderSource, "vertex shader");
    const GLuint fragmentShader = CompileShader(gl, GL_FRAGMENT_SHADER, fragmentShaderSource, "fragment shader");
    const GLuint program = gl.CreateProgram();
    if (program == 0)
    {
        gl.DeleteShader(fragmentShader);
        gl.DeleteShader(vertexShader);
        throw std::runtime_error("glCreateProgram failed.");
    }

    gl.AttachShader(program, vertexShader);
    gl.AttachShader(program, fragmentShader);
    gl.LinkProgram(program);

    gl.DeleteShader(fragmentShader);
    gl.DeleteShader(vertexShader);

    GLint linked = GL_FALSE;
    gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE)
    {
        const std::string log = ReadProgramInfoLog(gl, program);
        gl.DeleteProgram(program);
        throw std::runtime_error("shader program link failed:\n" + log);
    }

    return program;
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    LoadShaderProgramApi(stage.Gl);
    stage.Program = CreateTriangleProgram(stage.Gl);
    glViewport(0, 0, context.Width, context.Height);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.Pulse = static_cast<float>(0.5 + 0.5 * std::sin(timeSeconds * 1.25));
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    glViewport(0, 0, context.Width, context.Height);
    glClearColor(stage.ClearColor[0], stage.ClearColor[1], stage.ClearColor[2], stage.ClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    stage.Gl.UseProgram(stage.Program);
    glBegin(GL_TRIANGLES);
        glColor3f(0.90f, 0.25f + stage.Pulse * 0.25f, 0.20f);
        glVertex2f(0.0f, 0.68f);

        glColor3f(0.20f, 0.66f, 1.0f);
        glVertex2f(-0.72f, -0.56f);

        glColor3f(0.28f, 0.92f, 0.42f + stage.Pulse * 0.12f);
        glVertex2f(0.72f, -0.56f);
    glEnd();
    stage.Gl.UseProgram(0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.Program != 0)
    {
        stage.Gl.DeleteProgram(stage.Program);
        stage.Program = 0;
    }
}
