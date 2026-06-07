# 05. Push Constants And Uniforms

## Intent
Compare small push constants with uniform buffer binding.

## Implementation Status
Implemented. This project uses a uniform buffer descriptor for per-frame vertex offset data and push constants for a small fragment tint value.

## Prerequisite Sample
04. Index Buffer

## New Concepts
- push constants
- uniform buffer
- dynamic per-frame data
- alignment rules

## Expected Result
A 1280x720 Win32 window titled `05. Push Constants And Uniforms` opens, clears to a dark green background, and draws an indexed quad that slides horizontally while its fragment tint changes.

## Important API Objects / Calls
- vkCmdPushConstants
- vkCreateBuffer
- vkCreateDescriptorSetLayout
- vkCreateDescriptorPool
- vkAllocateDescriptorSets
- vkUpdateDescriptorSets

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL source and generated SPIR-V using a uniform block and push constant block.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\05. Push Constants And Uniforms\05. Push Constants And Uniforms.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
