# Project Intent Audit

This audit checks whether each DirectX 11 learning sample's stated intent matches the current implementation.

Verification date: 2026-06-06

## Runtime Verification

- `PracticeD3D11.slnx` builds successfully with `Debug|x64`.
- All 27 generated executables start successfully in the smoke test.
- Verification command:

```powershell
.\tools\Verify-Samples.ps1
```

## Intent Match Summary

| # | Project | Intended learning target | Implementation match | Notes |
|---|---------|--------------------------|----------------------|-------|
| 01 | Dx11Basic | Device, swap chain, render target, present | Partial | Now modernized as the reference structure. It still demonstrates the render loop, but also uses external HLSL and a fullscreen triangle. |
| 02 | HelloTriangle | Vertex buffer, input layout, shader, `Draw(3,0)` | Match | Minimal triangle pipeline is implemented. |
| 03 | Shaders | Vertex color interpolation and dynamic buffer update | Match | Adds color attribute and per-frame `Map`/`Unmap`. |
| 04 | Index Buffer | Indexed quad rendering | Match | Uses index buffer and `DrawIndexed(6,0,0)`. |
| 05 | Constant Buffer | CPU-to-GPU constants for VS/PS | Match | Uses constant buffers and `UpdateSubresource`. |
| 06 | Render Pipeline | IA/VS/RS/PS/OM visualization | Match | `LearningStage.h` records and visualizes pipeline state. |
| 07 | Coordinate Spaces | Local/world/view/projection comparison | Match | Uses multiple viewports and WVP transforms. |
| 08 | Matrix | SRT composition order | Match | Renders cubes using different matrix composition patterns. |
| 09 | Camera | Orbit camera controls | Match | Uses arrow keys and W/S for orbit/zoom. |
| 10 | 3D Cube | 3D cube plus edge visualization | Match | Renders indexed cube and line-list edges. |
| 11 | Depth Buffer | Depth-stencil state toggle | Match | Creates depth states and toggles depth behavior. |
| 12 | Texture | Texture2D, SRV, sampler, UV | Match | Builds a procedural texture and samples it. |
| 13 | Lighting | Basic directional lighting | Match | Uses normals, light vectors, and lit cube rendering. |
| 14 | Material | Texture plus material constants and lighting | Match | Integrates material color/specular-style constants with texture and lighting. |
| 15 | Rasterizer State | Fill/cull state switching | Match | Creates solid, wireframe, and no-cull rasterizer states. |
| 16 | Blend State | Alpha blending and depth write control | Match | Uses blend state and no-depth-write state for transparent rendering. |
| 17 | Model Loading | Inline OBJ parse into VB/IB | Match | Supports `v`, `vn`, and `f pos//normal`; UV/dedup intentionally omitted. |
| 18 | Scene | Entity/transform/mesh-renderer pattern | Match | Adds scene entity rendering via `LearningStage.h`. |
| 19 | Instancing | Per-instance buffer and instanced draw | Match | Uses `D3D11_INPUT_PER_INSTANCE_DATA` and `DrawIndexedInstanced`. |
| 20 | Render To Texture | Offscreen render target sampled later | Match | Creates offscreen RTV/SRV and uses write/sample passes. |
| 21 | Post Processing | Fullscreen post-process pass | Match | Copies back buffer to texture and renders grayscale/passthrough passes. |
| 22 | Shadow Mapping | Shadow depth pass and sampling | Match | Creates shadow map DSV/SRV and uses a light-space pass. |
| 23 | Skybox | TextureCube skybox pass | Match | Builds procedural cubemap and draws fullscreen skybox pass. |
| 24 | Deferred Rendering | G-buffer MRT and lighting pass | Match | Uses 3 render targets, G-buffer shader, lighting pass, and preview strip. |
| 25 | ImGui Integration | Runtime debug UI integration point | Partial | Buildable stub only. It does not vendor or run actual ImGui backend code yet. |
| 26 | Mini Engine | Resource handles, scene graph, render queue | Match | Demonstrates resource records, scene nodes, queue sorting, and culling stats. |
| 27 | Final Project | Combined viewer-style final sample | Match | Integrates camera movement, object list, material colors, lighting, and debug parameters. |

## Structural Findings

1. The project set is consistent as a learning ladder: most samples introduce exactly one new rendering concept while preserving earlier context.
2. `LearningStage.h` is effective for stage-specific overlays and extra render passes, but it is currently applied only to `06` and `17`-`27`.
3. `01. Dx11Basic` is now intentionally different from older samples because it is the reference for the modernized reusable structure.
4. `25. ImGui Integration` should be treated as an ImGui integration hook/stub until real ImGui source/backend files are added.
5. Existing README statements that say all shaders are inline are no longer globally true because `01. Dx11Basic` now uses `ClearTriangle.hlsl`.

## Recommended Corrections

- Document `01. Dx11Basic` as the modernized reference sample.
- Document `25. ImGui Integration` as a buildable debug-UI placeholder, not full ImGui integration.
- Use the new single-project template instruction when adding future DX12/OpenGL/Vulkan samples so each project has the same file structure and learning contract.
