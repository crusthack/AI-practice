# Vulkan Practice Workspace Context

## Purpose

This workspace is a Vulkan learning repository built around small, repeatable Visual C++ sample projects. It reflects the existing DirectX 11 and DirectX 12 practice style: numbered samples, one focused concept per sample, independent project files, and explicit markdown context.

## Current State

- Root solution: `VulkanLearning.sln`
- Current API focus: Vulkan on Win32
- Current platform: Windows, x64
- Current Visual Studio toolset: `v145`
- Required SDK: Vulkan SDK available through `VULKAN_SDK`
- Project count: 27
- Implemented sample count: 6
- Scaffolded sample count: 21

## Samples

- `01. VulkanBasic`: Implemented
- `02. HelloTriangle`: Implemented
- `03. Vertex Buffer Upload`: Implemented
- `04. Index Buffer`: Implemented
- `05. Push Constants And Uniforms`: Implemented
- `06. Descriptor Sets`: Implemented
- `07. Texture Upload`: Scaffold
- `08. Depth Buffer`: Scaffold
- `09. Transform Matrices`: Scaffold
- `10. Camera`: Scaffold
- `11. Frames In Flight`: Scaffold
- `12. Image Layout Transitions`: Scaffold
- `13. Staging And Device Local`: Scaffold
- `14. Pipeline Variants`: Scaffold
- `15. Rasterizer State`: Scaffold
- `16. Blend State`: Scaffold
- `17. Render To Texture`: Scaffold
- `18. Compute Shader`: Scaffold
- `19. Storage Buffer And Readback`: Scaffold
- `20. Model Loading`: Scaffold
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
    SampleName.vert.glsl
    SampleName.frag.glsl
  assets/
    README.md
  README.md
```

## Sample Contract

`main.cpp` owns the application shell:

- Win32 window creation
- Vulkan instance, surface, device, swapchain, image views, render pass, and framebuffers
- command pool and command buffer lifecycle
- acquire, submit, present, and synchronization
- calls to the stage hooks

`LearningStage.h` owns only sample-specific code:

- stage resources
- setup/update/render/cleanup hooks
- comments around the one new concept introduced by the sample

## Build Notes

Build the whole workspace:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Generated build outputs are placed under each sample's `bin/` and `obj/` directories and are ignored by git. Each project expects `$(VULKAN_SDK)\Include` and `$(VULKAN_SDK)\Lib`.

## Expansion Rules

- Add each new sample project to `VulkanLearning.sln`.
- Keep each sample independently buildable from its `.vcxproj`.
- Keep one primary learning goal per sample.
- Prefer procedural data for early samples.
- Use GLSL plus SPIR-V compilation when shaders become the primary lesson.
- Document placeholders explicitly in both code and README files.
