# 25. Post Processing

## Intent
Apply a full-screen post-processing pass to an offscreen render target.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass.

## Prerequisite Sample
24. Shadow Mapping

## New Concepts
- full-screen triangle
- post-process pixel shader
- SRV input texture
- two-pass rendering

## Expected Result
Scaffold visual: a smoke-test window clears to a dark neutral background. Final target: grayscale or color-shift post effect.

## Important API Objects / Calls
- ID3D12Device::CreateShaderResourceView
- ID3D12GraphicsCommandList::SetDescriptorHeaps
- ID3D12GraphicsCommandList::DrawInstanced
- ID3D12GraphicsCommandList::ResourceBarrier

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Post Processing.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\25. Post Processing\25. Post Processing.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
