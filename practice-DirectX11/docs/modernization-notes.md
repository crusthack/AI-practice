# DirectX 11 Samples Modernization Notes

## Current verification status

- `PracticeD3D11.slnx` builds successfully for `Debug|x64`.
- All 27 sample executables start successfully in a short smoke test.
- `01. Dx11Basic` is now the reference sample for the modernized structure.
- Intent/implementation alignment is tracked in `docs/project-intent-audit.md`.
- New projects for other APIs should follow `docs/single-project-template-instructions.md`.

Run the verification script from the repository root:

```powershell
.\tools\Verify-Samples.ps1
```

The script finds MSBuild, builds the solution, starts every generated executable briefly, and reports whether each sample reached its message loop.

## Modernized reference structure

The shared runtime lives under `common/`:

- `D3DApp`: Win32 window creation, message loop, and `WM_SIZE` dispatch.
- `DeviceResources`: D3D11 device, immediate context, swap chain, render target view, viewport, and resize handling.
- `ShaderUtils`: runtime HLSL compilation from external files.

`01. Dx11Basic` demonstrates the target direction:

- no global raw Direct3D interface pointers in sample code;
- `Microsoft::WRL::ComPtr` for COM lifetime management;
- swap-chain resize through `DeviceResources::Resize`;
- external `ClearTriangle.hlsl` copied to the output directory and compiled with `D3DCompileFromFile`.

## Recommended next migrations

Migrate samples in concept groups instead of rewriting all 27 at once:

1. `02`-`05`: move basic shader, vertex/index buffer, and constant-buffer samples onto `D3DApp`.
2. `06`-`10`: add optional depth-buffer support to `DeviceResources`, then migrate matrix/camera/cube samples.
3. `11`-`16`: introduce reusable state-object helpers for depth, rasterizer, blend, sampler, and texture resources.
4. `17`-`24`: formalize render-pass helpers for shadow, render-to-texture, post-process, skybox, and deferred rendering.
5. `25`-`27`: replace the current debug-panel stub with real ImGui integration and move mini-engine concepts into reusable engine modules.

Keep the original single-file style for early lessons only when it helps teach the first appearance of an API.
