# 14. Pipeline State Variants

## Intent
Create and switch between multiple PSOs for different shaders or render states.

## Implementation Status
Implemented. The sample creates two graphics PSOs with different rasterizer fill modes and switches between them while drawing two indexed rectangles.

## Prerequisite Sample
13. Upload And Default Heaps

## New Concepts
- PSO cache concept
- rasterizer state variants
- solid fill
- wireframe fill
- state switching

## Expected Result
A 1280x720 window titled `14. Pipeline State Variants` opens, clears to a subdued purple-gray background, and renders a solid rectangle beside a wireframe rectangle.

## Important API Objects / Calls
- ID3D12Device::CreateGraphicsPipelineState
- ID3D12GraphicsCommandList::SetPipelineState
- D3D12_GRAPHICS_PIPELINE_STATE_DESC
- D3D12_RASTERIZER_DESC

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Pipeline State Variants.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\14. Pipeline State Variants\14. Pipeline State Variants.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders solid and wireframe PSO variants side by side, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
