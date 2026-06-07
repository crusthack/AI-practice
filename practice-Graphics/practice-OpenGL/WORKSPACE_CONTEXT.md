# OpenGL Practice Workspace Context

## Purpose

This workspace is an OpenGL learning repository built around small, repeatable Visual C++ sample projects. It mirrors the DirectX 11 and DirectX 12 practice repositories by keeping one concept per sample and a consistent file/documentation contract.

## Current State

- Root solution: `OpenGLLearning.sln`
- Current API focus: OpenGL through Win32 WGL
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Project count: 27
- Implemented sample count: 5
- Scaffolded sample count: 22

## Samples

- `01. WglBasic`: Implemented
- `02. Hello Triangle`: Implemented
- `03. Shader Program`: Implemented
- `04. Vertex Buffer`: Implemented
- `05. Index Buffer`: Implemented
- `06. Vertex Array Object`: Scaffold
- `07. Uniforms And Time`: Scaffold
- `08. Transform Matrices`: Scaffold
- `09. Camera`: Scaffold
- `10. Texture Sampling`: Scaffold
- `11. Depth Buffer`: Scaffold
- `12. Face Culling`: Scaffold
- `13. Blending`: Scaffold
- `14. Framebuffer Object`: Scaffold
- `15. Post Processing`: Scaffold
- `16. Uniform Buffer Object`: Scaffold
- `17. Texture Array`: Scaffold
- `18. Cubemap Skybox`: Scaffold
- `19. Model Loading`: Scaffold
- `20. Scene Graph`: Scaffold
- `21. Lighting And Materials`: Scaffold
- `22. Instancing`: Scaffold
- `23. Shadow Mapping`: Scaffold
- `24. Geometry Shader`: Scaffold
- `25. Compute Shader`: Scaffold
- `26. ImGui Integration`: Scaffold
- `27. Mini Renderer`: Scaffold

## Repository Layout Contract

```text
NN. SampleName/
  NN. SampleName.vcxproj
  NN. SampleName.vcxproj.filters
  main.cpp
  LearningStage.h
  shaders/
    README.md
    SampleName.glsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- WGL pixel format and OpenGL context setup
- frame timing and message loop
- buffer swap and shutdown
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\OpenGLLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and should stay out of source control.

## Expansion Rules

- Add each new sample project to `OpenGLLearning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Keep OpenGL extension loading explicit through `common/OpenGLRuntime.h`.
- Keep stage-specific behavior controlled by the sample number and documented in each README.
