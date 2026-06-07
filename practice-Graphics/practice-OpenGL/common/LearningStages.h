#pragma once

#include "OpenGLRuntime.h"

struct LearningStageSetupContext
{
    int Width = 0;
    int Height = 0;
    HWND Window = nullptr;
};

struct LearningStageRenderContext
{
    int Width = 0;
    int Height = 0;
};

struct LearningStageState
{
    GLuint Program = 0;
    GLuint PostProgram = 0;
    GLuint GeometryProgram = 0;
    GLuint ComputeProgram = 0;
    GLuint Vao = 0;
    GLuint Vbo = 0;
    GLuint Ebo = 0;
    GLuint InstanceVbo = 0;
    GLuint Texture = 0;
    GLuint TextureArray = 0;
    GLuint CubeMap = 0;
    GLuint Fbo = 0;
    GLuint FboColor = 0;
    GLuint FboDepth = 0;
    GLuint ShadowFbo = 0;
    GLuint ShadowDepth = 0;
    GLuint Ubo = 0;
    GLsizei IndexCount = 0;
    double Time = 0.0;
    bool UseIndexBuffer = false;
    bool FboReady = false;
};

namespace OpenGLPractice
{
struct Vertex
{
    float px, py, pz;
    float nx, ny, nz;
    float r, g, b;
    float u, v;
};

inline const char* MainVertexShader()
{
    return R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aUv;
layout(location = 4) in vec3 aInstanceOffset;
layout(location = 5) in vec3 aInstanceColor;

layout(std140) uniform FrameBlock
{
    mat4 uBlockMvp;
    vec4 uBlockLight;
};

uniform mat4 uMvp;
uniform float uTime;
uniform int uUseUbo;
uniform int uUseInstancing;

out vec3 vNormal;
out vec3 vColor;
out vec2 vUv;
out vec3 vWorld;

void main()
{
    vec3 offset = (uUseInstancing != 0) ? aInstanceOffset : vec3(0.0);
    vec3 position = aPosition + offset;
    mat4 finalMvp = (uUseUbo != 0) ? uBlockMvp : uMvp;
    gl_Position = finalMvp * vec4(position, 1.0);
    vNormal = normalize(aNormal);
    vColor = (uUseInstancing != 0) ? aInstanceColor : aColor;
    vUv = aUv;
    vWorld = position;
}
)GLSL";
}

inline const char* MainFragmentShader()
{
    return R"GLSL(
#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec2 vUv;
in vec3 vWorld;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform sampler2DArray uTextureArray;
uniform samplerCube uCubeMap;
uniform sampler2DShadow uShadowMap;
uniform float uTime;
uniform int uMode;

vec3 palette(float x)
{
    return 0.55 + 0.45 * cos(6.28318 * (vec3(0.00, 0.33, 0.67) + x));
}

void main()
{
    vec3 n = normalize(vNormal);
    vec3 lightDir = normalize(vec3(-0.35, 0.8, 0.45));
    float diffuse = max(dot(n, lightDir), 0.0);
    vec3 color = vColor;

    if (uMode >= 10)
        color *= texture(uTexture, vUv).rgb;
    if (uMode >= 17)
        color = mix(color, texture(uTextureArray, vec3(vUv, mod(floor(vWorld.x * 2.0 + 4.0), 3.0))).rgb, 0.45);
    if (uMode >= 18)
        color = mix(color, texture(uCubeMap, normalize(vec3(vUv * 2.0 - 1.0, 1.0))).rgb, 0.25);
    if (uMode >= 21)
    {
        vec3 viewDir = normalize(vec3(0.0, 0.0, 4.0) - vWorld);
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(n, halfDir), 0.0), 48.0);
        color = color * (0.18 + diffuse * 0.82) + vec3(spec);
    }
    if (uMode >= 23)
    {
        float stripe = step(0.35, fract((vWorld.x + vWorld.z + uTime * 0.15) * 2.0));
        color *= mix(0.65, 1.0, stripe);
    }
    if (uMode >= 26)
    {
        vec3 uiTint = palette(uTime * 0.08);
        color = mix(color, uiTint, 0.12);
    }
    FragColor = vec4(color, (uMode >= 13 && vWorld.x > 0.0) ? 0.62 : 1.0);
}
)GLSL";
}

inline const char* PostVertexShader()
{
    return R"GLSL(
#version 330 core
const vec2 kPos[3] = vec2[3](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
out vec2 vUv;
void main()
{
    gl_Position = vec4(kPos[gl_VertexID], 0.0, 1.0);
    vUv = gl_Position.xy * 0.5 + 0.5;
}
)GLSL";
}

inline const char* PostFragmentShader()
{
    return R"GLSL(
#version 330 core
in vec2 vUv;
out vec4 FragColor;
uniform sampler2D uScene;
uniform float uTime;
uniform int uMode;
void main()
{
    vec3 color = texture(uScene, vUv).rgb;
    if (uMode >= 15)
    {
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        vec2 d = vUv - vec2(0.5);
        float vignette = smoothstep(0.8, 0.15, dot(d, d));
        color = mix(vec3(gray), color * vec3(1.08, 1.02, 0.95), 0.55) * vignette;
    }
    if (uMode >= 27)
        color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
)GLSL";
}

inline const char* GeometryShader()
{
    return R"GLSL(
#version 330 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;
void main()
{
    for (int i = 0; i < 3; ++i)
    {
        vec4 p = gl_in[i].gl_Position;
        gl_Position = p;
        EmitVertex();
        gl_Position = p + vec4(0.0, 0.08, 0.0, 0.0);
        EmitVertex();
        EndPrimitive();
    }
}
)GLSL";
}

inline const char* GeometryFragmentShader()
{
    return R"GLSL(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.95, 0.95, 0.30, 1.0);
}
)GLSL";
}

inline const char* ComputeShader()
{
    return R"GLSL(
#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba8, binding = 0) uniform writeonly image2D uImage;
uniform float uTime;
void main()
{
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(p) / vec2(imageSize(uImage));
    vec3 color = 0.5 + 0.5 * cos(6.28318 * vec3(uv.x, uv.y, uv.x + uv.y + uTime * 0.1));
    imageStore(uImage, p, vec4(color, 1.0));
}
)GLSL";
}

inline std::vector<Vertex> TriangleVertices()
{
    return {
        {  0.0f,  0.7f, 0.0f, 0, 0, 1, 0.95f, 0.20f, 0.15f, 0.5f, 1.0f },
        { -0.7f, -0.6f, 0.0f, 0, 0, 1, 0.20f, 0.75f, 0.95f, 0.0f, 0.0f },
        {  0.7f, -0.6f, 0.0f, 0, 0, 1, 0.35f, 0.90f, 0.35f, 1.0f, 0.0f },
    };
}

inline std::vector<Vertex> QuadVertices()
{
    return {
        { -0.65f,  0.55f, 0.0f, 0, 0, 1, 0.90f, 0.25f, 0.20f, 0.0f, 1.0f },
        {  0.65f,  0.55f, 0.0f, 0, 0, 1, 0.20f, 0.70f, 0.95f, 1.0f, 1.0f },
        {  0.65f, -0.55f, 0.0f, 0, 0, 1, 0.30f, 0.85f, 0.35f, 1.0f, 0.0f },
        { -0.65f, -0.55f, 0.0f, 0, 0, 1, 0.85f, 0.70f, 0.20f, 0.0f, 0.0f },
    };
}

inline std::vector<Vertex> CubeVertices()
{
    const float s = 0.45f;
    return {
        { -s,-s, s,  0, 0, 1, 0.95f,0.25f,0.18f, 0,0 }, {  s,-s, s,  0, 0, 1, 0.95f,0.55f,0.18f, 1,0 },
        {  s, s, s,  0, 0, 1, 0.95f,0.85f,0.18f, 1,1 }, { -s, s, s,  0, 0, 1, 0.95f,0.25f,0.45f, 0,1 },
        {  s,-s,-s,  0, 0,-1, 0.20f,0.55f,0.95f, 0,0 }, { -s,-s,-s,  0, 0,-1, 0.20f,0.80f,0.95f, 1,0 },
        { -s, s,-s,  0, 0,-1, 0.50f,0.80f,0.95f, 1,1 }, {  s, s,-s,  0, 0,-1, 0.30f,0.45f,0.95f, 0,1 },
        { -s,-s,-s, -1, 0, 0, 0.25f,0.85f,0.40f, 0,0 }, { -s,-s, s, -1, 0, 0, 0.25f,0.85f,0.65f, 1,0 },
        { -s, s, s, -1, 0, 0, 0.55f,0.85f,0.40f, 1,1 }, { -s, s,-s, -1, 0, 0, 0.25f,0.65f,0.35f, 0,1 },
        {  s,-s, s,  1, 0, 0, 0.85f,0.35f,0.85f, 0,0 }, {  s,-s,-s,  1, 0, 0, 0.65f,0.35f,0.85f, 1,0 },
        {  s, s,-s,  1, 0, 0, 0.85f,0.55f,0.95f, 1,1 }, {  s, s, s,  1, 0, 0, 0.85f,0.35f,0.70f, 0,1 },
        { -s, s, s,  0, 1, 0, 0.95f,0.95f,0.25f, 0,0 }, {  s, s, s,  0, 1, 0, 0.95f,0.85f,0.35f, 1,0 },
        {  s, s,-s,  0, 1, 0, 0.70f,0.95f,0.25f, 1,1 }, { -s, s,-s,  0, 1, 0, 0.75f,0.85f,0.20f, 0,1 },
        { -s,-s,-s,  0,-1, 0, 0.45f,0.45f,0.48f, 0,0 }, {  s,-s,-s,  0,-1, 0, 0.55f,0.50f,0.48f, 1,0 },
        {  s,-s, s,  0,-1, 0, 0.65f,0.55f,0.48f, 1,1 }, { -s,-s, s,  0,-1, 0, 0.45f,0.40f,0.42f, 0,1 },
    };
}

inline std::vector<std::uint16_t> QuadIndices()
{
    return { 0, 1, 2, 0, 2, 3 };
}

inline std::vector<std::uint16_t> CubeIndices()
{
    std::vector<std::uint16_t> result;
    for (std::uint16_t f = 0; f < 6; ++f)
    {
        const std::uint16_t b = static_cast<std::uint16_t>(f * 4);
        result.insert(result.end(), { b, static_cast<std::uint16_t>(b + 1), static_cast<std::uint16_t>(b + 2), b, static_cast<std::uint16_t>(b + 2), static_cast<std::uint16_t>(b + 3) });
    }
    return result;
}

inline void CreateMesh(LearningStageState& stage, int number)
{
    const bool cube = number >= 8;
    const bool quad = number >= 5 && !cube;
    const std::vector<Vertex> vertices = cube ? CubeVertices() : (quad ? QuadVertices() : TriangleVertices());
    const std::vector<std::uint16_t> indices = cube ? CubeIndices() : QuadIndices();

    stage.UseIndexBuffer = number >= 5;
    stage.IndexCount = stage.UseIndexBuffer ? static_cast<GLsizei>(indices.size()) : static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &stage.Vao);
    glBindVertexArray(stage.Vao);
    glGenBuffers(1, &stage.Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, stage.Vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<ptrdiff_t>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);

    if (stage.UseIndexBuffer)
    {
        glGenBuffers(1, &stage.Ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stage.Ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<ptrdiff_t>(indices.size() * sizeof(std::uint16_t)), indices.data(), GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, px)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, nx)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, u)));

    if (number >= 22)
    {
        struct Instance { float ox, oy, oz; float r, g, b; };
        std::array<Instance, 25> instances = {};
        int k = 0;
        for (int y = -2; y <= 2; ++y)
        {
            for (int x = -2; x <= 2; ++x)
            {
                instances[static_cast<size_t>(k)] = {
                    static_cast<float>(x) * 0.45f,
                    static_cast<float>(y) * 0.32f,
                    0.0f,
                    0.35f + static_cast<float>(x + 2) * 0.12f,
                    0.35f + static_cast<float>(y + 2) * 0.12f,
                    0.85f
                };
                ++k;
            }
        }
        glGenBuffers(1, &stage.InstanceVbo);
        glBindBuffer(GL_ARRAY_BUFFER, stage.InstanceVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<ptrdiff_t>(instances.size() * sizeof(Instance)), instances.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), reinterpret_cast<void*>(offsetof(Instance, ox)));
        glVertexAttribDivisor(4, 1);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), reinterpret_cast<void*>(offsetof(Instance, r)));
        glVertexAttribDivisor(5, 1);
    }

    glBindVertexArray(0);
}

inline void CreateTexture(LearningStageState& stage)
{
    std::array<std::uint32_t, 64 * 64> pixels = {};
    for (int y = 0; y < 64; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            const bool checker = ((x / 8 + y / 8) % 2) == 0;
            const std::uint8_t r = checker ? 235 : 40;
            const std::uint8_t g = checker ? 210 : 120;
            const std::uint8_t b = checker ? 80 : 210;
            pixels[static_cast<size_t>(y * 64 + x)] = 0xff000000u | (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(g) << 8) | r;
        }
    }
    glGenTextures(1, &stage.Texture);
    glBindTexture(GL_TEXTURE_2D, stage.Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

inline void CreateTextureArray(LearningStageState& stage)
{
    std::array<std::uint32_t, 32 * 32 * 3> pixels = {};
    for (int layer = 0; layer < 3; ++layer)
    {
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 32; ++x)
            {
                const std::uint8_t r = static_cast<std::uint8_t>(layer == 0 ? 220 : 40 + x * 4);
                const std::uint8_t g = static_cast<std::uint8_t>(layer == 1 ? 220 : 40 + y * 4);
                const std::uint8_t b = static_cast<std::uint8_t>(layer == 2 ? 220 : 120);
                pixels[static_cast<size_t>(layer * 32 * 32 + y * 32 + x)] = 0xff000000u | (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(g) << 8) | r;
            }
        }
    }
    glGenTextures(1, &stage.TextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, stage.TextureArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 32, 32, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

inline void CreateCubeMap(LearningStageState& stage)
{
    const std::uint32_t colors[6] = { 0xff3344eeu, 0xff55cc55u, 0xffee5533u, 0xffdddd55u, 0xff55cceeu, 0xffbb66ddu };
    glGenTextures(1, &stage.CubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, stage.CubeMap);
    for (int face = 0; face < 6; ++face)
    {
        std::array<std::uint32_t, 16 * 16> pixels = {};
        pixels.fill(colors[face]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

inline void CreateFbo(LearningStageState& stage, int width, int height)
{
    glGenFramebuffers(1, &stage.Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, stage.Fbo);
    glGenTextures(1, &stage.FboColor);
    glBindTexture(GL_TEXTURE_2D, stage.FboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, stage.FboColor, 0);

    glGenRenderbuffers(1, &stage.FboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, stage.FboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stage.FboDepth);
    stage.FboReady = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void CreateUbo(LearningStageState& stage)
{
    glGenBuffers(1, &stage.Ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, stage.Ubo);
    std::array<float, 20> data = {};
    glBufferData(GL_UNIFORM_BUFFER, static_cast<ptrdiff_t>(data.size() * sizeof(float)), data.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, stage.Ubo);
    const GLuint block = glGetUniformBlockIndex(stage.Program, "FrameBlock");
    if (block != 0xffffffffu)
        glUniformBlockBinding(stage.Program, block, 0);
}

inline Mat4 BuildMvp(int number, double time, int width, int height)
{
    const float t = static_cast<float>(time);
    const float aspect = static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1);
    if (number < 8)
        return Identity();
    const float radius = number >= 9 ? 2.5f + 0.35f * std::sin(t * 0.5f) : 2.8f;
    const float camX = std::sin(t * 0.35f) * radius;
    const float camZ = std::cos(t * 0.35f) * radius;
    const Mat4 view = LookAt(camX, 1.35f, camZ, 0.0f, 0.0f, 0.0f);
    const Mat4 proj = Perspective(1.05f, aspect, 0.1f, 32.0f);
    const Mat4 model = Multiply(RotateY(t * 0.8f), Scale(1.0f, 1.0f, 1.0f));
    return Multiply(proj, Multiply(view, model));
}

inline void UpdateUbo(const LearningStageState& stage, const Mat4& mvp)
{
    if (!stage.Ubo)
        return;
    std::array<float, 20> data = {};
    std::memcpy(data.data(), mvp.m, sizeof(mvp.m));
    data[16] = -0.35f;
    data[17] = 0.80f;
    data[18] = 0.45f;
    data[19] = 0.0f;
    glBindBuffer(GL_UNIFORM_BUFFER, stage.Ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<ptrdiff_t>(data.size() * sizeof(float)), data.data());
}

inline void DrawMainScene(const LearningStageState& stage, int number, int width, int height)
{
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    if (number >= 12)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(number == 12 ? GL_FRONT : GL_BACK);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    if (number >= 13)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    const Mat4 mvp = BuildMvp(number, stage.Time, width, height);
    UpdateUbo(stage, mvp);

    glUseProgram(stage.Program);
    glUniformMatrix4fv(glGetUniformLocation(stage.Program, "uMvp"), 1, GL_FALSE, mvp.m);
    glUniform1f(glGetUniformLocation(stage.Program, "uTime"), static_cast<float>(stage.Time));
    glUniform1i(glGetUniformLocation(stage.Program, "uMode"), number);
    glUniform1i(glGetUniformLocation(stage.Program, "uUseUbo"), number >= 16 ? 1 : 0);
    glUniform1i(glGetUniformLocation(stage.Program, "uUseInstancing"), number >= 22 ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stage.Texture);
    glUniform1i(glGetUniformLocation(stage.Program, "uTexture"), 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, stage.TextureArray);
    glUniform1i(glGetUniformLocation(stage.Program, "uTextureArray"), 1);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, stage.CubeMap);
    glUniform1i(glGetUniformLocation(stage.Program, "uCubeMap"), 2);

    glBindVertexArray(stage.Vao);
    if (number >= 22)
    {
        glDrawElementsInstanced(GL_TRIANGLES, stage.IndexCount, GL_UNSIGNED_SHORT, nullptr, 25);
    }
    else if (stage.UseIndexBuffer)
    {
        if (number >= 20)
        {
            for (int i = 0; i < 4; ++i)
            {
                const float x = -0.9f + static_cast<float>(i) * 0.6f;
                const Mat4 sceneMvp = Multiply(mvp, Translate(x, 0.15f * std::sin(static_cast<float>(stage.Time) + static_cast<float>(i)), 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(stage.Program, "uMvp"), 1, GL_FALSE, sceneMvp.m);
                glDrawElements(GL_TRIANGLES, stage.IndexCount, GL_UNSIGNED_SHORT, nullptr);
            }
        }
        else
        {
            glDrawElements(GL_TRIANGLES, stage.IndexCount, GL_UNSIGNED_SHORT, nullptr);
        }
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, stage.IndexCount);
    }
    glBindVertexArray(0);
}

inline void DrawPost(const LearningStageState& stage, int number, int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glClearColor(0.03f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(stage.PostProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stage.FboColor);
    glUniform1i(glGetUniformLocation(stage.PostProgram, "uScene"), 0);
    glUniform1f(glGetUniformLocation(stage.PostProgram, "uTime"), static_cast<float>(stage.Time));
    glUniform1i(glGetUniformLocation(stage.PostProgram, "uMode"), number);
    glBindVertexArray(stage.Vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    SetObjectName(context.Window, OPENGL_SAMPLE_TITLE);
    glClearColor(0.055f, 0.075f, 0.095f, 1.0f);

    if (OPENGL_SAMPLE_NUMBER == 1)
        return;

    stage.Program = LinkProgram(MainVertexShader(), MainFragmentShader());
    if (OPENGL_SAMPLE_NUMBER >= 14)
        stage.PostProgram = LinkProgram(PostVertexShader(), PostFragmentShader());
    if (OPENGL_SAMPLE_NUMBER >= 24)
        stage.GeometryProgram = LinkProgram(MainVertexShader(), GeometryFragmentShader(), GeometryShader());
    if (OPENGL_SAMPLE_NUMBER >= 25 && glDispatchCompute)
        stage.ComputeProgram = LinkProgram(nullptr, nullptr, nullptr, ComputeShader());

    CreateMesh(stage, OPENGL_SAMPLE_NUMBER);
    if (OPENGL_SAMPLE_NUMBER >= 10)
        CreateTexture(stage);
    if (OPENGL_SAMPLE_NUMBER >= 14)
        CreateFbo(stage, context.Width, context.Height);
    if (OPENGL_SAMPLE_NUMBER >= 16)
        CreateUbo(stage);
    if (OPENGL_SAMPLE_NUMBER >= 17)
        CreateTextureArray(stage);
    if (OPENGL_SAMPLE_NUMBER >= 18)
        CreateCubeMap(stage);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.Time = timeSeconds;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    const int number = OPENGL_SAMPLE_NUMBER;
    if (number == 1)
    {
        glViewport(0, 0, context.Width, context.Height);
        glClearColor(0.055f, 0.075f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    if (number >= 25 && stage.ComputeProgram && glDispatchCompute && glBindImageTexture)
    {
        glUseProgram(stage.ComputeProgram);
        glUniform1f(glGetUniformLocation(stage.ComputeProgram, "uTime"), static_cast<float>(stage.Time));
        glBindImageTexture(0, stage.Texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glDispatchCompute(8, 8, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    if (number >= 14 && stage.FboReady)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, stage.Fbo);
        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawMainScene(stage, number, context.Width, context.Height);
        DrawPost(stage, number, context.Width, context.Height);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.055f, 0.075f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawMainScene(stage, number, context.Width, context.Height);
    }

    if (number >= 24 && stage.GeometryProgram)
    {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(stage.GeometryProgram);
        const Mat4 mvp = BuildMvp(number, stage.Time, context.Width, context.Height);
        glUniformMatrix4fv(glGetUniformLocation(stage.GeometryProgram, "uMvp"), 1, GL_FALSE, mvp.m);
        glUniform1f(glGetUniformLocation(stage.GeometryProgram, "uTime"), static_cast<float>(stage.Time));
        glUniform1i(glGetUniformLocation(stage.GeometryProgram, "uUseUbo"), 0);
        glUniform1i(glGetUniformLocation(stage.GeometryProgram, "uUseInstancing"), 0);
        glBindVertexArray(stage.Vao);
        glDrawElements(GL_TRIANGLES, stage.IndexCount, GL_UNSIGNED_SHORT, nullptr);
        glBindVertexArray(0);
    }
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    if (stage.Ubo) glDeleteBuffers(1, &stage.Ubo);
    if (stage.InstanceVbo) glDeleteBuffers(1, &stage.InstanceVbo);
    if (stage.Ebo) glDeleteBuffers(1, &stage.Ebo);
    if (stage.Vbo) glDeleteBuffers(1, &stage.Vbo);
    if (stage.Vao) glDeleteVertexArrays(1, &stage.Vao);
    if (stage.Texture) glDeleteTextures(1, &stage.Texture);
    if (stage.TextureArray) glDeleteTextures(1, &stage.TextureArray);
    if (stage.CubeMap) glDeleteTextures(1, &stage.CubeMap);
    if (stage.FboDepth) glDeleteRenderbuffers(1, &stage.FboDepth);
    if (stage.FboColor) glDeleteTextures(1, &stage.FboColor);
    if (stage.Fbo) glDeleteFramebuffers(1, &stage.Fbo);
    if (stage.Program) glDeleteProgram(stage.Program);
    if (stage.PostProgram) glDeleteProgram(stage.PostProgram);
    if (stage.GeometryProgram) glDeleteProgram(stage.GeometryProgram);
    if (stage.ComputeProgram) glDeleteProgram(stage.ComputeProgram);
}

} // namespace OpenGLPractice

using OpenGLPractice::ApplyStageSpecificCleanup;
using OpenGLPractice::ApplyStageSpecificRender;
using OpenGLPractice::ApplyStageSpecificSetup;
using OpenGLPractice::UpdateStageSpecificDemo;
