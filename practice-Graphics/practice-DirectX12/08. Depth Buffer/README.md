# 08. Depth Buffer

## Intent
Create a depth resource and DSV heap so nearer fragments occlude farther fragments.

## Implementation Status
Implemented. The sample creates a depth texture and DSV heap, clears depth each frame, and renders overlapping geometry with depth testing enabled.

## Prerequisite Sample
07. Texture Upload

## New Concepts
- depth resource
- DSV heap
- depth clear
- depth comparison
- depth/stencil state

## Expected Result
A 1280x720 window titled `08. Depth Buffer` opens, clears to a dark blue-black background, and renders overlapping triangles with depth testing enabled.

## Important API Objects / Calls
- ID3D12Device::CreateDescriptorHeap
- ID3D12Device::CreateDepthStencilView
- ID3D12GraphicsCommandList::ClearDepthStencilView
- ID3D12GraphicsCommandList::OMSetRenderTargets
- D3D12_DEPTH_STENCIL_DESC

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Depth Buffer.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\08. Depth Buffer\08. Depth Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders overlapping triangles with depth occlusion, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
