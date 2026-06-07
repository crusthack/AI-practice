# 18. Compute Shader

## Intent
Dispatch a compute shader and synchronize its output for rendering or readback.

## Implementation Status
Implemented. This project dispatches a compute shader into a UAV texture, transitions that texture for shader sampling, and displays the compute-generated result on the swap-chain back buffer.

## Prerequisite Sample
17. Render To Texture

## New Concepts
- compute root signature
- compute PSO
- dispatch dimensions
- UAV output

## Expected Result
A 1280x720 window titled `18. Compute Shader` opens and displays an animated compute-generated color pattern sampled from a UAV-written texture.

## Important API Objects / Calls
- ID3D12Device::CreateComputePipelineState
- ID3D12GraphicsCommandList::SetComputeRootSignature
- ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
- ID3D12GraphicsCommandList::Dispatch
- ID3D12Device::CreateUnorderedAccessView
- ID3D12GraphicsCommandList::ResourceBarrier

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Compute Shader.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\18. Compute Shader\18. Compute Shader.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, shows an animated compute-generated texture, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
