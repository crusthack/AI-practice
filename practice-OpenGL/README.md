# OpenGL Learning Samples

OpenGL을 단계적으로 익히기 위한 Visual Studio C++ 샘플 모음이다. DirectX11/12 학습 프로젝트와 같은 27단계 구조를 유지하되, WGL 컨텍스트 생성부터 버퍼, 셰이더, 텍스처, FBO, 고급 렌더링까지 OpenGL식 개념 순서로 정리한다.

## Quick Start

Visual Studio에서 `OpenGLLearning.sln`을 열고 원하는 번호의 프로젝트를 시작 프로젝트로 설정한 뒤 `F5`로 실행한다.

Command-line build:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

## Structure

```text
practice-OpenGL/
  OpenGLLearning.sln
  WORKSPACE_CONTEXT.md
  ROADMAP.md
  docs/
  tools/
  01. WglBasic/
    main.cpp
    LearningStage.h
    shaders/
    assets/
  ...
  27. Mini Renderer/
```

Each project is independently buildable and uses the same hook pattern:

```text
wWinMain
  InitWindow
  InitOpenGL
  ApplyStageSpecificSetup
  message loop
    UpdateStageSpecificDemo
    ApplyStageSpecificRender
    SwapBuffers
  ApplyStageSpecificCleanup
```

## Curriculum

| # | Project | Core Goal | Status |
|---|---|---|---|
| 01 | WglBasic | Create a Win32 window, initialize a WGL OpenGL context, clear the default framebuffer, and swap buffers. | Implemented |
| 02 | Hello Triangle | Draw the first triangle and identify how OpenGL turns submitted vertices into fragments. | Implemented |
| 03 | Shader Program | Compile GLSL vertex and fragment shaders, link a program, and inspect shader errors. | Implemented |
| 04 | Vertex Buffer | Upload vertex data into a VBO and describe the vertex attribute layout. | Implemented |
| 05 | Index Buffer | Reuse vertices through an element buffer and indexed drawing. | Implemented |
| 06 | Vertex Array Object | Capture vertex input state with a VAO and separate object setup from draw calls. | Scaffold |
| 07 | Uniforms And Time | Update shader uniforms every frame for animation and per-draw parameters. | Scaffold |
| 08 | Transform Matrices | Send model, view, and projection matrices to GLSL and transform geometry. | Scaffold |
| 09 | Camera | Build an orbit camera and update the view matrix from keyboard input. | Scaffold |
| 10 | Texture Sampling | Create a procedural texture, configure sampler state, and sample it in GLSL. | Scaffold |
| 11 | Depth Buffer | Enable depth testing so nearer fragments occlude farther fragments. | Scaffold |
| 12 | Face Culling | Control front-face winding and cull back or front faces. | Scaffold |
| 13 | Blending | Enable alpha blending and render transparent geometry in controlled order. | Scaffold |
| 14 | Framebuffer Object | Render into an offscreen framebuffer object and sample its color attachment. | Scaffold |
| 15 | Post Processing | Draw a fullscreen pass from an FBO texture and apply a simple effect. | Scaffold |
| 16 | Uniform Buffer Object | Group frame constants in a UBO and bind it across shader programs. | Scaffold |
| 17 | Texture Array | Store related textures in a texture array and choose layers in the shader. | Scaffold |
| 18 | Cubemap Skybox | Create a cubemap texture and render a skybox with depth-state changes. | Scaffold |
| 19 | Model Loading | Load or procedurally parse mesh data and upload it into GL buffers. | Scaffold |
| 20 | Scene Graph | Render multiple objects with transforms, mesh references, and material data. | Scaffold |
| 21 | Lighting And Materials | Combine normals, material parameters, and lights for Blinn-Phong shading. | Scaffold |
| 22 | Instancing | Draw many copies of a mesh with per-instance transform or color data. | Scaffold |
| 23 | Shadow Mapping | Render depth from a light view and sample it when shading the main scene. | Scaffold |
| 24 | Geometry Shader | Use a geometry shader for controlled primitive expansion or visualization. | Scaffold |
| 25 | Compute Shader | Dispatch a compute shader and synchronize its result for rendering or readback. | Scaffold |
| 26 | ImGui Integration | Attach Dear ImGui as a debug UI overlay for OpenGL runtime parameters. | Scaffold |
| 27 | Mini Renderer | Combine context setup, shader management, scene data, render passes, and debug UI into a small renderer. | Scaffold |

## Notes

- Implement samples in order; `01. WglBasic` through `05. Index Buffer` are complete right now.
- The project intentionally avoids external dependencies such as GLFW, GLAD, or GLEW.
- Modern OpenGL entry points will be introduced deliberately when the relevant sample requires them.
