# 11. Frame Resources

## Intent
Use per-frame command allocators, constant buffers, and fence values for multiple frames in flight.

## Implementation Status
Implemented. The sample keeps one mapped constant buffer and one CBV descriptor per swap-chain frame slot, then binds the current frame's descriptor during rendering.

## Prerequisite Sample
10. Camera

## New Concepts
- frames in flight
- per-frame allocator
- per-frame constants
- fence value tracking

## Expected Result
A 1280x720 window titled `11. Frame Resources` opens, clears to a charcoal background, and renders a triangle whose tint and offset are updated through the current frame resource.

## Important API Objects / Calls
- ID3D12Device::CreateCommandAllocator
- ID3D12Device::CreateConstantBufferView
- ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
- ID3D12CommandQueue::Signal
- ID3D12Fence::SetEventOnCompletion
- IDXGISwapChain3::GetCurrentBackBufferIndex

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Frame Resources.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\11. Frame Resources\11. Frame Resources.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders a triangle using the current per-frame constant buffer, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
