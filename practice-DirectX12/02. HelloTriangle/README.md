# 02. HelloTriangle

## Intent
Build the first root signature and graphics pipeline state object, then draw a triangle.

## Implementation Status
Implemented. This project creates an empty root signature, compiles external HLSL, creates a graphics pipeline state object, uploads triangle vertices, and records a draw call.

## Prerequisite Sample
01. Dx12Basic

## New Concepts
- root signature
- graphics pipeline state object
- viewport and scissor
- external HLSL shaders
- non-indexed draw call

## Expected Result
A 1280x720 window titled `02. HelloTriangle` opens, clears to a deep blue background, and renders one colored triangle.

## Important API Objects / Calls
- D3D12SerializeRootSignature
- ID3D12Device::CreateRootSignature
- D3DCompileFromFile
- ID3D12Device::CreateGraphicsPipelineState
- ID3D12Device::CreateCommittedResource
- ID3D12Resource::Map
- ID3D12GraphicsCommandList::SetGraphicsRootSignature
- ID3D12GraphicsCommandList::SetPipelineState
- ID3D12GraphicsCommandList::RSSetViewports
- ID3D12GraphicsCommandList::RSSetScissorRects
- ID3D12GraphicsCommandList::IASetPrimitiveTopology
- ID3D12GraphicsCommandList::IASetVertexBuffers
- ID3D12GraphicsCommandList::DrawInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `HelloTriangle.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\02. HelloTriangle\02. HelloTriangle.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders a colored triangle over the clear color, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
