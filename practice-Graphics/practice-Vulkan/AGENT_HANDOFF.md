# Vulkan Practice Agent Handoff

## Current Objective

Continue implementing `practice-Vulkan` samples in order, preserving the structure and documentation style derived from `practice-DirectX11` and `practice-DirectX12`.

The user asked to complete each project sequentially. Work started from sample 1 and has completed samples 1 through 6.

## Environment

- Workspace root: `C:\Users\crust\Documents\AI-practice`
- Vulkan SDK installed through winget:
  - `C:\VulkanSDK\1.4.350.0`
  - `glslc.exe`: `C:\VulkanSDK\1.4.350.0\Bin\glslc.exe`
- Current shell may not have `VULKAN_SDK` automatically populated. Use this before builds if needed:

```powershell
$env:VULKAN_SDK='C:\VulkanSDK\1.4.350.0'
```

- MSBuild path used successfully:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
```

## Completed Samples

### 01. VulkanBasic

Status: Implemented and built successfully.

Implemented:
- Win32 window
- Vulkan instance
- optional validation layer/debug messenger
- Win32 surface
- physical/logical device
- swapchain
- image views
- render pass
- framebuffers
- command pool and command buffers
- semaphores/fence
- clear/present loop

Validation:
- Build succeeded with 0 warnings and 0 errors.

Important note:
- `NOMINMAX` was added to avoid Windows `min/max` macro conflicts.
- `ThrowIfFailed` treats only negative `VkResult` values as failure.

### 02. HelloTriangle

Status: Implemented and built successfully.

Implemented:
- GLSL vertex/fragment shaders
- compiled SPIR-V checked into `shaders/`
- shader modules
- pipeline layout
- graphics pipeline
- `vkCmdBindPipeline`
- `vkCmdDraw`

Shader commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\HelloTriangle.vert.glsl" -o ".\shaders\HelloTriangle.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\HelloTriangle.frag.glsl" -o ".\shaders\HelloTriangle.frag.spv"
```

Validation:
- Build succeeded with 0 warnings and 0 errors.

### 03. Vertex Buffer Upload

Status: Implemented and built successfully.

Implemented:
- `Vertex` struct with position/color
- host-visible staging buffer
- device-local vertex buffer
- `vkMapMemory`
- `vkCmdCopyBuffer`
- `vkCmdBindVertexBuffers`
- shader attribute input at locations 0 and 1

Validation:
- Build succeeded with 0 warnings and 0 errors.

### 04. Index Buffer

Status: Implemented and built successfully.

Implemented:
- shared-vertex quad
- `uint16_t` index buffer
- staging upload for vertex and index data
- `vkCmdBindIndexBuffer`
- `vkCmdDrawIndexed`

Validation:
- Build succeeded with 0 warnings and 0 errors.

### 05. Push Constants And Uniforms

Status: Implemented and built successfully.

Implemented:
- uniform buffer for per-frame offset
- descriptor set layout/pool/allocation/write for uniform buffer
- push constant range for fragment tint
- animated offset/tint from `std::chrono` time
- `vkCmdBindDescriptorSets`
- `vkCmdPushConstants`

Validation:
- Build succeeded with 0 warnings and 0 errors.

### 06. Descriptor Sets

Status: Implemented and built successfully.

Implemented:
- descriptor set layout with two uniform-buffer bindings
- binding 0: vertex-stage transform offset uniform
- binding 1: fragment-stage tint uniform
- descriptor pool allocation
- descriptor writes for both bindings
- descriptor-bound animated transform and tint

Validation:
- Build succeeded with 0 warnings and 0 errors.

## Repository State Updates Already Done

Updated:
- `practice-Vulkan/WORKSPACE_CONTEXT.md`
  - Implemented sample count: 6
  - Scaffolded sample count: 21
  - samples 01 through 06 marked Implemented
- `practice-Vulkan/ROADMAP.md`
  - samples 01 through 06 marked Implemented
  - next implementation pass points to `07. Texture Upload`
- sample README files for 01 through 06
- shader README files for 02 through 06
- `practice-Vulkan/.gitignore`
  - ignores build outputs such as `bin/`, `obj/`, `.pdb`, `.ilk`, `.exe`, `.lib`

## Next Sample

Start with:

```text
07. Texture Upload
```

Roadmap intent:

> Upload a procedural texture, transition image layouts, and sample it.

Expected implementation:
- keep the existing Win32/Vulkan shell pattern
- use prior geometry/pipeline/descriptor patterns
- add procedural CPU texture data, likely a checkerboard
- create staging buffer for pixel data
- create device-local `VkImage`
- allocate/bind image memory
- transition image layouts with `vkCmdPipelineBarrier`
- copy buffer to image with `vkCmdCopyBufferToImage`
- create `VkImageView`
- create `VkSampler`
- add descriptor binding for combined image sampler
- shader should read UV coordinates and sample the texture
- update `07. Texture Upload/README.md`, `shaders/README.md`, `ROADMAP.md`, and `WORKSPACE_CONTEXT.md`
- compile GLSL to SPIR-V and include `.spv` files in project copy items
- build sample 07 and verify 0 warnings, 0 errors

## Implementation Pattern To Preserve

Each implemented sample currently follows this pattern:
- `main.cpp` owns shared shell:
  - window
  - instance/surface/device/swapchain/render pass/framebuffers
  - command buffers
  - acquire/submit/present synchronization
  - calls into stage hooks
- `LearningStage.h` owns sample-specific resources and render commands.
- `ApplyStageSpecificSetup(...)` accepts only the Vulkan handles the stage needs.
- `ApplyStageSpecificRender(...)` starts/ends the render pass and records sample draw work.
- `ApplyStageSpecificCleanup(...)` destroys stage-owned resources.
- shader source lives in `shaders/*.glsl`
- compiled SPIR-V lives in `shaders/*.spv`
- `.vcxproj` copies `.spv` files into `bin\x64\Debug\shaders\...`

## Build Commands

Use these from workspace root.

Single sample example:

```powershell
$env:VULKAN_SDK='C:\VulkanSDK\1.4.350.0'
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\practice-Vulkan\07. Texture Upload\07. Texture Upload.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

GLSL compile example:

```powershell
$env:VULKAN_SDK='C:\VulkanSDK\1.4.350.0'
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\practice-Vulkan\07. Texture Upload\shaders\Texture Upload.vert.glsl" -o ".\practice-Vulkan\07. Texture Upload\shaders\Texture Upload.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\practice-Vulkan\07. Texture Upload\shaders\Texture Upload.frag.glsl" -o ".\practice-Vulkan\07. Texture Upload\shaders\Texture Upload.frag.spv"
```

## Cautions

- Do not regenerate the whole project with `tools/generate_vulkan_samples.py` unless it has first been updated to preserve implemented samples. The script still reflects older scaffold templates in places and can overwrite implemented work.
- Build outputs are intentionally ignored by `practice-Vulkan/.gitignore`.
- The repository also has an untracked `practice-OpenGL/` directory. It was not touched during this Vulkan work.
- The generated `main.cpp` shell is duplicated across samples. Keep changes focused to the active sample unless intentionally promoting a shell improvement to multiple samples.
