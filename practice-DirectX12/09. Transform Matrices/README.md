# 09. Transform Matrices

## Intent
Pass world, view, and projection matrices to shaders and transform geometry.

## Implementation Status
Implemented. The sample builds a world-view-projection matrix on the CPU, uploads it through a CBV descriptor table, and transforms geometry in HLSL.

## Prerequisite Sample
08. Depth Buffer

## New Concepts
- world matrix
- view matrix
- projection matrix
- matrix upload
- HLSL constant layout

## Expected Result
A 1280x720 window titled `09. Transform Matrices` opens, clears to a dark violet-gray background, and renders a rotating transformed quad using a WVP constant buffer.

## Important API Objects / Calls
- DirectX::XMMatrixRotationZ
- DirectX::XMMatrixLookAtLH
- DirectX::XMMatrixPerspectiveFovLH
- DirectX::XMStoreFloat4x4
- ID3D12Device::CreateConstantBufferView

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Transform Matrices.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\09. Transform Matrices\09. Transform Matrices.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the rotating transformed quad, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
