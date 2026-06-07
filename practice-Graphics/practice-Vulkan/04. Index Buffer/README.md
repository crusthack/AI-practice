# 04. Index Buffer

## Intent
Draw shared-vertex geometry through an index buffer.

## Implementation Status
Implemented. This project uploads shared vertices and 16-bit indices through staging buffers, binds both buffers, and draws a quad with `vkCmdDrawIndexed`.

## Prerequisite Sample
03. Vertex Buffer Upload

## New Concepts
- index buffer
- index type
- shared vertices
- indexed draw call

## Expected Result
A 1280x720 Win32 window titled `04. Index Buffer` opens, clears to a dark teal background, and draws one colored quad made from four shared vertices and six indices.

## Important API Objects / Calls
- vkCreateBuffer
- vkAllocateMemory
- vkMapMemory
- vkCmdCopyBuffer
- vkCmdBindIndexBuffer
- vkCmdDrawIndexed

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL source and generated SPIR-V that read indexed vertex attributes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\04. Index Buffer\04. Index Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
