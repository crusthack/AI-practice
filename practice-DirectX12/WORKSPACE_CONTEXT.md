# DirectX 12 Practice Workspace Context

## Purpose

This workspace is a DirectX 12 learning repository built around small, repeatable Visual C++ sample projects. Each sample isolates one graphics concept and keeps a consistent file/documentation contract so later samples are easy to compare.

## Current State

- Root solution: `DirectX12Learning.sln`
- Current API focus: DirectX 12 on Win32
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Project count: 27
- Implemented sample count: 20
- Scaffolded sample count: 7

## Samples

- `01. Dx12Basic`: Implemented
- `02. HelloTriangle`: Implemented
- `03. Vertex Buffer Upload`: Implemented
- `04. Index Buffer`: Implemented
- `05. Root Signature And Constants`: Implemented
- `06. Descriptor Heap`: Implemented
- `07. Texture Upload`: Implemented
- `08. Depth Buffer`: Implemented
- `09. Transform Matrices`: Implemented
- `10. Camera`: Implemented
- `11. Frame Resources`: Implemented
- `12. Resource Barriers`: Implemented
- `13. Upload And Default Heaps`: Implemented
- `14. Pipeline State Variants`: Implemented
- `15. Rasterizer State`: Implemented
- `16. Blend State`: Implemented
- `17. Render To Texture`: Implemented
- `18. Compute Shader`: Implemented
- `19. UAV And Readback`: Implemented
- `20. Model Loading`: Implemented
- `21. Scene Graph`: Scaffold
- `22. Lighting And Materials`: Scaffold
- `23. Instancing`: Scaffold
- `24. Shadow Mapping`: Scaffold
- `25. Post Processing`: Scaffold
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
    SampleName.hlsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- DX12 device and swap chain setup
- frame timing and message loop
- command submission, present, and synchronization
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and are ignored by git.

## Expansion Rules

- Add each new sample project to `DirectX12Learning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Use external HLSL files once pipeline state and shaders become the primary lesson.
- Document placeholders explicitly in both code and README files.
