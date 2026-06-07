# 01. VulkanBasic

## Intent
Create a Win32 window, Vulkan instance, surface, device, swapchain, command buffers, and clear/present loop.

## Implementation Status
Implemented. This project implements the first Vulkan clear/present smoke test.

## Prerequisite Sample
none

## New Concepts
- Win32 window and Vulkan surface
- instance and validation layer selection
- physical/logical device selection
- swapchain images and image views
- render pass, framebuffers, command pool, command buffers
- semaphores, fences, acquire, submit, present

## Expected Result
A 1280x720 Win32 window titled `01. VulkanBasic` opens, clears the swapchain image to this sample's documented color, and presents until closed.

## Important API Objects / Calls
- vkCreateInstance
- vkEnumerateInstanceLayerProperties
- vkCreateDebugUtilsMessengerEXT
- vkCreateWin32SurfaceKHR
- vkEnumeratePhysicalDevices
- vkCreateDevice
- vkCreateSwapchainKHR
- vkCreateRenderPass
- vkAllocateCommandBuffers
- vkQueueSubmit
- vkQueuePresentKHR

## File Map
- `main.cpp`: owns the Win32 window, Vulkan instance/device/swapchain/render-pass/frame loop, synchronization, present, and stage hook calls.
- `LearningStage.h`: owns sample-specific state and render hook code.
- `shaders/`: present for repository consistency; this first clear/present sample does not compile or bind shaders.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\01. VulkanBasic\01. VulkanBasic.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

Requires `VULKAN_SDK` to point at an installed Vulkan SDK.

## Intent Match Checklist
- [x] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
