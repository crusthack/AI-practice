# 06. Descriptor Sets

## Intent
Create descriptor set layouts, descriptor pools, and bind uniform descriptors.

## Implementation Status
Implemented. This project creates a descriptor set layout with two uniform-buffer bindings, allocates one descriptor set, writes both descriptors, and binds the set for rendering.

## Prerequisite Sample
05. Push Constants And Uniforms

## New Concepts
- descriptor set layout
- descriptor pool
- descriptor set allocation
- descriptor writes

## Expected Result
A 1280x720 Win32 window titled `06. Descriptor Sets` opens, clears to a dark slate background, and draws an indexed quad whose position and tint are driven by descriptor-bound uniform buffers.

## Important API Objects / Calls
- vkCreateDescriptorSetLayout
- vkCreateDescriptorPool
- vkAllocateDescriptorSets
- vkUpdateDescriptorSets
- vkCmdBindDescriptorSets

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL source and generated SPIR-V using descriptor bindings 0 and 1.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\06. Descriptor Sets\06. Descriptor Sets.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
