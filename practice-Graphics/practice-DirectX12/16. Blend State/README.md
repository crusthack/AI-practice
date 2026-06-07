# 16. Blend State

## Intent
Enable alpha blending and render transparent geometry in a controlled order.

## Implementation Status
Implemented. The sample enables source-alpha blending in the PSO and draws overlapping transparent triangles in a fixed order.

## Prerequisite Sample
15. Rasterizer State

## New Concepts
- blend state
- alpha blending
- render ordering
- source/destination factors

## Expected Result
A 1280x720 window titled `16. Blend State` opens, clears to a dark wine background, and renders two overlapping transparent triangles with visible alpha blending.

## Important API Objects / Calls
- D3D12_BLEND_DESC
- D3D12_RENDER_TARGET_BLEND_DESC
- ID3D12Device::CreateGraphicsPipelineState
- ID3D12GraphicsCommandList::DrawInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Blend State.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\16. Blend State\16. Blend State.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the blended transparent geometry, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
