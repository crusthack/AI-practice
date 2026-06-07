# 19. Storage Buffer And Readback

## Intent
Write GPU data through a storage buffer and read selected results on the CPU.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a Vulkan clear/present smoke test; lesson-specific rendering is left for the focused implementation pass.

## Prerequisite Sample
18. Compute Shader

## New Concepts
- storage buffer
- host readback memory
- buffer barriers
- CPU verification

## Expected Result
A 1280x720 Win32 window titled `19. Storage Buffer And Readback` opens, clears the swapchain image to this sample's documented color, and presents until closed.

## Important API Objects / Calls
- vkCmdBindDescriptorSets
- vkCmdDispatch
- vkMapMemory

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: contains GLSL placeholders for the lesson-specific shader pass.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\19. Storage Buffer And Readback\19. Storage Buffer And Readback.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Requires `VULKAN_SDK` to point at an installed Vulkan SDK.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
