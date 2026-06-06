# 05. Root Signature And Constants

## Intent
Bind small per-draw values through root constants and compare them with constant-buffer binding.

## Implementation Status
Implemented. This project creates a root signature with 32-bit root constants, updates those values each frame, binds them with `SetGraphicsRoot32BitConstants`, and uses them in HLSL.

## Prerequisite Sample
04. Index Buffer

## New Concepts
- root parameters
- root constants
- constant buffer view
- 256-byte alignment
- root argument cost

## Expected Result
A 1280x720 window titled `05. Root Signature And Constants` opens, clears to a dark green background, and renders a colored triangle that shifts and pulses using root constants.

## Important API Objects / Calls
- D3D12SerializeRootSignature
- ID3D12Device::CreateRootSignature
- ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants
- ID3D12Device::CreateGraphicsPipelineState
- ID3D12Device::CreateCommittedResource
- ID3D12Resource::Map
- ID3D12GraphicsCommandList::DrawInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Root Signature And Constants.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\05. Root Signature And Constants\05. Root Signature And Constants.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders an animated triangle controlled by root constants, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
