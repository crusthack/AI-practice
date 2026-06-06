# 24. Shadow Mapping

## Intent
Render depth from a light view and sample it to shade shadows.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass.

## Prerequisite Sample
23. Instancing

## New Concepts
- shadow map
- light view projection
- depth-only pass
- shadow comparison

## Expected Result
Scaffold visual: a smoke-test window clears to a dark graphite background. Final target: object casting a shadow.

## Important API Objects / Calls
- ID3D12Device::CreateDepthStencilView
- ID3D12Device::CreateShaderResourceView
- ID3D12GraphicsCommandList::ClearDepthStencilView
- ID3D12GraphicsCommandList::DrawIndexedInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Shadow Mapping.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\24. Shadow Mapping\24. Shadow Mapping.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
