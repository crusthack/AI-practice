# 15. Rasterizer State

## Intent
Change fill and culling behavior through rasterizer state in the PSO.

## Implementation Status
Implemented. The sample creates solid, wireframe, and back-face culling PSOs and renders comparison geometry with each rasterizer state.

## Prerequisite Sample
14. Pipeline State Variants

## New Concepts
- wireframe fill
- solid fill
- front/back culling
- rasterizer desc

## Expected Result
A 1280x720 window titled `15. Rasterizer State` opens, clears to a dark steel background, and renders solid, wireframe, and back-face-culling examples side by side.

## Important API Objects / Calls
- D3D12_RASTERIZER_DESC
- D3D12_CULL_MODE
- D3D12_FILL_MODE
- ID3D12Device::CreateGraphicsPipelineState

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Rasterizer State.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\15. Rasterizer State\15. Rasterizer State.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the three rasterizer-state examples, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
