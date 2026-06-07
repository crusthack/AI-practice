# 27. Mini Renderer

## Intent
Combine the lessons into a small Vulkan renderer with passes, descriptors, synchronization, and debug UI.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a Vulkan clear/present smoke test; lesson-specific rendering is left for the focused implementation pass.

## Prerequisite Sample
26. ImGui Integration

## New Concepts
- renderer modules
- resource lifetime
- render graph mindset
- debug UI
- frame orchestration

## Expected Result
A 1280x720 Win32 window titled `27. Mini Renderer` opens, clears the swapchain image to this sample's documented color, and presents until closed.

## Important API Objects / Calls
- vkQueueSubmit
- vkCmdBeginRenderPass
- vkCmdBindDescriptorSets

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
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\27. Mini Renderer\27. Mini Renderer.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
