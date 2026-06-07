# 02. HelloTriangle

## Intent
Create shader modules, pipeline layout, graphics pipeline, and draw the first triangle.

## Implementation Status
Implemented. This project creates shader modules, a pipeline layout, a graphics pipeline, and draws a colored triangle without vertex buffers.

## Prerequisite Sample
01. VulkanBasic

## New Concepts
- SPIR-V shader modules
- pipeline layout
- graphics pipeline
- viewport and scissor
- non-indexed draw call

## Expected Result
A 1280x720 Win32 window titled `02. HelloTriangle` opens, clears the swapchain image to a dark blue background, and draws one colored triangle.

## Important API Objects / Calls
- vkCreateShaderModule
- vkCreatePipelineLayout
- vkCreateGraphicsPipelines
- vkCmdBindPipeline
- vkCmdDraw

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL source and generated SPIR-V used by the graphics pipeline.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\02. HelloTriangle\02. HelloTriangle.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Requires `VULKAN_SDK` to point at an installed Vulkan SDK.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
