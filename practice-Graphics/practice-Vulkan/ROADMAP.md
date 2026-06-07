# Vulkan Learning Roadmap

This roadmap mirrors the scale of the DirectX 11 and DirectX 12 practice sets while using Vulkan-specific ordering: instance and surface first, then swapchain and render passes, followed by buffers, descriptors, images, synchronization, render passes, compute, and renderer structure.

## Learning Principles

- One sample, one primary concept.
- Keep Vulkan ownership explicit in `main.cpp` until a concept needs to move into `LearningStage.h`.
- Put sample-specific resources and render hooks in `LearningStage.h`.
- Prefer deterministic procedural data before asset-loading samples.
- Every sample must have a visible result that can be checked quickly.
- Every sample must build independently and also belong to `VulkanLearning.sln`.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
| 01 | `VulkanBasic` | Create a Win32 window, Vulkan instance, surface, device, swapchain, command buffers, and clear/present loop. | Implemented |
| 02 | `HelloTriangle` | Create shader modules, pipeline layout, graphics pipeline, and draw the first triangle. | Implemented |
| 03 | `Vertex Buffer Upload` | Upload vertex data through a staging buffer and bind a vertex buffer. | Implemented |
| 04 | `Index Buffer` | Draw shared-vertex geometry through an index buffer. | Implemented |
| 05 | `Push Constants And Uniforms` | Compare small push constants with uniform buffer binding. | Implemented |
| 06 | `Descriptor Sets` | Create descriptor set layouts, descriptor pools, and bind uniform descriptors. | Implemented |
| 07 | `Texture Upload` | Upload a procedural texture, transition image layouts, and sample it. | Scaffold |
| 08 | `Depth Buffer` | Create a depth image and use depth testing for 3D geometry. | Scaffold |
| 09 | `Transform Matrices` | Pass world, view, and projection matrices to shaders. | Scaffold |
| 10 | `Camera` | Move a view camera with keyboard and mouse input. | Scaffold |
| 11 | `Frames In Flight` | Use per-frame command buffers, semaphores, fences, and uniform buffers. | Scaffold |
| 12 | `Image Layout Transitions` | Practice explicit image layout transitions and pipeline barriers. | Scaffold |
| 13 | `Staging And Device Local` | Separate CPU-visible staging resources from GPU-local buffers and images. | Scaffold |
| 14 | `Pipeline Variants` | Create and switch between multiple graphics pipelines. | Scaffold |
| 15 | `Rasterizer State` | Change polygon mode, culling, and front-face settings in pipeline state. | Scaffold |
| 16 | `Blend State` | Enable alpha blending and render transparent geometry. | Scaffold |
| 17 | `Render To Texture` | Render into an offscreen image and sample it on the swapchain pass. | Scaffold |
| 18 | `Compute Shader` | Dispatch a compute shader and synchronize its output. | Scaffold |
| 19 | `Storage Buffer And Readback` | Write GPU data through a storage buffer and read selected results on the CPU. | Scaffold |
| 20 | `Model Loading` | Load mesh data from an external file and upload it into GPU buffers. | Scaffold |
| 21 | `Scene Graph` | Render multiple objects with separate transforms, materials, and draw calls. | Scaffold |
| 22 | `Lighting And Materials` | Combine normals, material parameters, and light constants for shaded objects. | Scaffold |
| 23 | `Instancing` | Draw many copies of the same mesh with per-instance data. | Scaffold |
| 24 | `Shadow Mapping` | Render depth from a light view and sample it to shade shadows. | Scaffold |
| 25 | `Post Processing` | Apply a full-screen post-processing pass to an offscreen image. | Scaffold |
| 26 | `ImGui Integration` | Integrate Dear ImGui as a debug UI overlay for Vulkan samples. | Scaffold |
| 27 | `Mini Renderer` | Combine the lessons into a small Vulkan renderer with passes, descriptors, synchronization, and debug UI. | Scaffold |

## Implementation Strategy

`01. VulkanBasic` is implemented as the foundation. Later projects start as independent Vulkan clear/present smoke tests with the full folder and documentation contract. Promote each scaffold to `Implemented` by adding only the lesson-specific Vulkan resources and draw/dispatch work described in its README.

## Next Implementation Pass

Start with `07. Texture Upload`:

- create a sampled image, image view, and sampler
- upload procedural texture data through staging resources
- add image layout transitions and sampled-image descriptors
- update `ROADMAP.md` and `WORKSPACE_CONTEXT.md`
