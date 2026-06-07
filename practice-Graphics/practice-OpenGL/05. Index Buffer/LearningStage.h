#pragma once

// Learning goal: reuse vertices through an element/index buffer.
// Implementation status: Implemented.
//
// This sample keeps the shader and VBO path from 04, then adds one
// GL_ELEMENT_ARRAY_BUFFER and switches the draw call to glDrawElements.

#include <windows.h>
#include <gl/GL.h>

#include <cstddef>
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
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

using GLchar = char;
using GLsizeiptr = ptrdiff_t;

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
using PFNGLGENBUFFERSPROC = void(APIENTRY*)(GLsizei n, GLuint* buffers);
using PFNGLBINDBUFFERPROC = void(APIENTRY*)(GLenum target, GLuint buffer);
using PFNGLBUFFERDATAPROC = void(APIENTRY*)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
using PFNGLDELETEBUFFERSPROC = void(APIENTRY*)(GLsizei n, const GLuint* buffers);

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

struct Vertex
{
    float Position[2];
    float Color[3];
};

struct OpenGLApi
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
    PFNGLGENBUFFERSPROC GenBuffers = nullptr;
    PFNGLBINDBUFFERPROC BindBuffer = nullptr;
    PFNGLBUFFERDATAPROC BufferData = nullptr;
    PFNGLDELETEBUFFERSPROC DeleteBuffers = nullptr;
};

struct LearningStageState
{
    OpenGLApi Gl = {};
    GLuint Program = 0;
    GLuint VertexBuffer = 0;
    GLuint IndexBuffer = 0;
    GLsizei IndexCount = 0;
    float ClearColor[4] = { 0.025f, 0.040f, 0.060f, 1.0f };
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

inline void LoadOpenGLApi(OpenGLApi& gl)
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
    gl.GenBuffers = LoadRequiredOpenGLProcAs<PFNGLGENBUFFERSPROC>("glGenBuffers");
    gl.BindBuffer = LoadRequiredOpenGLProcAs<PFNGLBINDBUFFERPROC>("glBindBuffer");
    gl.BufferData = LoadRequiredOpenGLProcAs<PFNGLBUFFERDATAPROC>("glBufferData");
    gl.DeleteBuffers = LoadRequiredOpenGLProcAs<PFNGLDELETEBUFFERSPROC>("glDeleteBuffers");
}

inline std::string ReadShaderInfoLog(const OpenGLApi& gl, GLuint shader)
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

inline std::string ReadProgramInfoLog(const OpenGLApi& gl, GLuint program)
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

inline GLuint CompileShader(const OpenGLApi& gl, GLenum type, const char* source, const char* debugName)
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

inline GLuint CreateQuadProgram(const OpenGLApi& gl)
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

inline GLuint CreateBuffer(const OpenGLApi& gl, GLenum target, GLsizeiptr byteSize, const void* data)
{
    GLuint buffer = 0;
    gl.GenBuffers(1, &buffer);
    if (buffer == 0)
    {
        throw std::runtime_error("glGenBuffers failed.");
    }

    gl.BindBuffer(target, buffer);
    gl.BufferData(target, byteSize, data, GL_STATIC_DRAW);
    gl.BindBuffer(target, 0);
    return buffer;
}

inline void CreateIndexedQuadBuffers(LearningStageState& stage)
{
    const Vertex vertices[] =
    {
        { { -0.62f, 0.52f }, { 0.94f, 0.26f, 0.18f } },
        { { -0.62f, -0.52f }, { 0.22f, 0.70f, 1.0f } },
        { { 0.62f, -0.52f }, { 0.28f, 0.92f, 0.42f } },
        { { 0.62f, 0.52f }, { 1.0f, 0.82f, 0.24f } },
    };

    const unsigned int indices[] =
    {
        0, 1, 2,
        0, 2, 3,
    };

    stage.VertexBuffer = CreateBuffer(stage.Gl, GL_ARRAY_BUFFER, sizeof(vertices), vertices);
    stage.IndexBuffer = CreateBuffer(stage.Gl, GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices);
    stage.IndexCount = static_cast<GLsizei>(sizeof(indices) / sizeof(indices[0]));
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    LoadOpenGLApi(stage.Gl);
    stage.Program = CreateQuadProgram(stage.Gl);
    CreateIndexedQuadBuffers(stage);
    glViewport(0, 0, context.Width, context.Height);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)stage;
    (void)timeSeconds;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    glViewport(0, 0, context.Width, context.Height);
    glClearColor(stage.ClearColor[0], stage.ClearColor[1], stage.ClearColor[2], stage.ClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    stage.Gl.UseProgram(stage.Program);
    stage.Gl.BindBuffer(GL_ARRAY_BUFFER, stage.VertexBuffer);
    stage.Gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, stage.IndexBuffer);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, Position)));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, Color)));
    glDrawElements(GL_TRIANGLES, stage.IndexCount, GL_UNSIGNED_INT, nullptr);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    stage.Gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    stage.Gl.BindBuffer(GL_ARRAY_BUFFER, 0);
    stage.Gl.UseProgram(0);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.IndexBuffer != 0)
    {
        stage.Gl.DeleteBuffers(1, &stage.IndexBuffer);
        stage.IndexBuffer = 0;
    }

    if (stage.VertexBuffer != 0)
    {
        stage.Gl.DeleteBuffers(1, &stage.VertexBuffer);
        stage.VertexBuffer = 0;
    }

    if (stage.Program != 0)
    {
        stage.Gl.DeleteProgram(stage.Program);
        stage.Program = 0;
    }
}
