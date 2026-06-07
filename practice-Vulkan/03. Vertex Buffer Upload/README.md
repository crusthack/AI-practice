# 03. Vertex Buffer Upload

## Intent
Upload vertex data through a staging buffer and bind a vertex buffer.

## Implementation Status
Implemented. This project uploads CPU vertex data through a host-visible staging buffer, copies it into device-local memory, binds the vertex buffer, and draws a triangle.

## Prerequisite Sample
02. HelloTriangle

## New Concepts
- buffer creation
- host-visible staging memory
- device-local vertex buffer
- copy command
- vertex input binding

## Expected Result
A 1280x720 Win32 window titled `03. Vertex Buffer Upload` opens, clears the swapchain image to a dark violet background, and draws one colored triangle from a bound vertex buffer.

## Important API Objects / Calls
- vkCreateBuffer
- vkAllocateMemory
- vkMapMemory
- vkCmdCopyBuffer
- vkCmdBindVertexBuffers
- vkCmdDraw

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL source and generated SPIR-V that read vertex position/color attributes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\03. Vertex Buffer Upload\03. Vertex Buffer Upload.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
