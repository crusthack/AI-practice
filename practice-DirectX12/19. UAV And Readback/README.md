# 19. UAV And Readback

## Intent
Write GPU data through a UAV and read selected results back to the CPU.

## Implementation Status
Implemented. This project dispatches a compute shader that writes one color into a UAV buffer, copies that buffer into a readback heap, maps the readback resource on the CPU, and uses the read value to drive the next frame's clear color.

## Prerequisite Sample
18. Compute Shader

## New Concepts
- unordered access view
- readback heap
- UAV barrier
- GPU-to-CPU copy

## Expected Result
A 1280x720 window titled `19. UAV And Readback` opens and slowly changes its clear color based on a value written by the GPU and read back by the CPU.

## Important API Objects / Calls
- ID3D12Device::CreateUnorderedAccessView
- ID3D12GraphicsCommandList::CopyResource
- D3D12_RESOURCE_BARRIER_TYPE_UAV
- D3D12_HEAP_TYPE_READBACK
- ID3D12Resource::Map

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `UAV And Readback.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\19. UAV And Readback\19. UAV And Readback.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, updates the clear color from a GPU-written readback value, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
