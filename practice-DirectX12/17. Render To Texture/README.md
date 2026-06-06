# 17. Render To Texture

## Intent
Render into an offscreen texture and then sample it while presenting to the swap chain.

## Implementation Status
Implemented. This project creates an offscreen render target texture, renders a procedural triangle into it, transitions it to an SRV, and samples it while drawing to the swap-chain back buffer.

## Prerequisite Sample
16. Blend State

## New Concepts
- offscreen render target
- RTV and SRV for one texture
- render pass separation
- state transition between render target and SRV

## Expected Result
A 1280x720 window titled `17. Render To Texture` opens and displays a colored triangle that was first rendered into an offscreen texture and then sampled onto the back buffer.

## Important API Objects / Calls
- ID3D12Device::CreateRenderTargetView
- ID3D12Device::CreateShaderResourceView
- ID3D12GraphicsCommandList::ResourceBarrier
- ID3D12GraphicsCommandList::OMSetRenderTargets
- ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Render To Texture.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\17. Render To Texture\17. Render To Texture.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, shows the offscreen-rendered colored triangle through a sampled texture, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
