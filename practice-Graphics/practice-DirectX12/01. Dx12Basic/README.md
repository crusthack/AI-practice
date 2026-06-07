# 01. Dx12Basic

## Intent
Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present.

## Implementation Status
Implemented. This project implements its stated first-step goal.

## Prerequisite Sample
none

## New Concepts
- Win32 window creation for a graphics sample
- DXGI factory and hardware adapter selection
- DirectX 12 device and direct command queue
- Flip-model swap chain with two back buffers
- RTV descriptor heap and render target views
- Command allocator, command list, resource barriers, fence, and present

## Expected Result
A 1280x720 window titled `01. Dx12Basic` opens and shows a solid dark blue-gray background.

## Important API Objects / Calls
- CreateDXGIFactory2
- IDXGIFactory4::EnumAdapters1
- D3D12CreateDevice
- ID3D12Device::CreateCommandQueue
- IDXGIFactory4::CreateSwapChainForHwnd
- ID3D12Device::CreateDescriptorHeap
- ID3D12Device::CreateRenderTargetView
- ID3D12Device::CreateCommandAllocator
- ID3D12Device::CreateCommandList
- ID3D12GraphicsCommandList::ResourceBarrier
- IDXGISwapChain::Present
- ID3D12Device::CreateFence

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Dx12Basic.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\01. Dx12Basic\01. Dx12Basic.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, clears the back buffer to the documented scaffold color, and continues presenting until closed.

## Intent Match Checklist
- [ ] The project builds independently.
- [ ] The visual result demonstrates the stated intent.
- [ ] Every new API call listed above appears in code.
- [ ] No unrelated concept is introduced as a required dependency.
