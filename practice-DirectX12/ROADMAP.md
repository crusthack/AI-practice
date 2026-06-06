# DirectX 12 Learning Roadmap

This roadmap is organized around DirectX 12's explicit device, resource, descriptor, command-list, synchronization, and render-pass model. It keeps the 27-step learning scale of the DirectX 11 practice set, but the order and topic boundaries are DX12-specific.

## Learning Principles

- One sample, one primary concept.
- Keep the first important DX12 objects explicit in `main.cpp`.
- Put only sample-specific resources and hooks in `LearningStage.h`.
- Prefer deterministic procedural data before asset-loading samples.
- Every sample must have a visible result that can be checked quickly.
- Every sample must build independently and also belong to `DirectX12Learning.sln`.

## Full Sequence

| No. | Sample | Goal | Status |
|---|---|---|---|
| 01 | `Dx12Basic` | Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present. | Implemented |
| 02 | `HelloTriangle` | Build the first root signature and graphics pipeline state object, then draw a triangle. | Implemented |
| 03 | `Vertex Buffer Upload` | Upload vertex data into GPU resources and bind a vertex buffer view. | Implemented |
| 04 | `Index Buffer` | Draw shared-vertex geometry through an index buffer. | Implemented |
| 05 | `Root Signature And Constants` | Bind small per-draw values through root constants and compare them with constant-buffer binding. | Implemented |
| 06 | `Descriptor Heap` | Create shader-visible descriptor heaps and bind CBV descriptors through descriptor tables. | Implemented |
| 07 | `Texture Upload` | Upload a procedural texture and sample it through an SRV and sampler. | Implemented |
| 08 | `Depth Buffer` | Create a depth resource and DSV heap so nearer fragments occlude farther fragments. | Implemented |
| 09 | `Transform Matrices` | Pass world, view, and projection matrices to shaders and transform geometry. | Implemented |
| 10 | `Camera` | Move a view camera with keyboard and mouse input. | Implemented |
| 11 | `Frame Resources` | Use per-frame command allocators, constant buffers, and fence values for multiple frames in flight. | Implemented |
| 12 | `Resource Barriers` | Practice explicit resource state transitions for render targets, copy destinations, and shader resources. | Implemented |
| 13 | `Upload And Default Heaps` | Separate CPU-visible upload resources from GPU-local default resources and copy between them. | Implemented |
| 14 | `Pipeline State Variants` | Create and switch between multiple PSOs for different shaders or render states. | Implemented |
| 15 | `Rasterizer State` | Change fill and culling behavior through rasterizer state in the PSO. | Implemented |
| 16 | `Blend State` | Enable alpha blending and render transparent geometry in a controlled order. | Implemented |
| 17 | `Render To Texture` | Render into an offscreen texture and then sample it while presenting to the swap chain. | Implemented |
| 18 | `Compute Shader` | Dispatch a compute shader and synchronize its output for rendering or readback. | Implemented |
| 19 | `UAV And Readback` | Write GPU data through a UAV and read selected results back to the CPU. | Implemented |
| 20 | `Model Loading` | Load mesh data from an external file and upload it into GPU buffers. | Implemented |
| 21 | `Scene Graph` | Render multiple objects with separate transforms, materials, and draw calls. | Scaffold |
| 22 | `Lighting And Materials` | Combine material parameters, normals, and light constants for basic shaded objects. | Scaffold |
| 23 | `Instancing` | Draw many copies of the same mesh with per-instance data. | Scaffold |
| 24 | `Shadow Mapping` | Render depth from a light view and sample it to shade shadows. | Scaffold |
| 25 | `Post Processing` | Apply a full-screen post-processing pass to an offscreen render target. | Scaffold |
| 26 | `ImGui Integration` | Integrate Dear ImGui as a debug UI overlay for DX12 samples. | Scaffold |
| 27 | `Mini Renderer` | Combine the learning samples into a small DX12 renderer with frame resources, descriptors, passes, and debug UI. | Scaffold |

## Implementation Strategy

`01. Dx12Basic` is implemented as the foundation. The remaining projects are scaffolded as independent DX12 projects with the required folder contract, README contract, shader/assets folders, and a clear/present smoke test. Each scaffold should be promoted from `Scaffold` to `Implemented` one at a time, following its README intent and checklist.

## Next Implementation Pass

Start with `21. Scene Graph` and implement its focused DX12 lesson:

- promote the sample README from `Scaffold` to `Implemented`
- replace the clear-only render hook with the sample-specific resources and draw/dispatch work
- keep the project independently buildable
- update `ROADMAP.md` and `WORKSPACE_CONTEXT.md`

