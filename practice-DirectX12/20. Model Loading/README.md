# 20. Model Loading

## Intent
Load mesh data from an external file and upload it into GPU buffers.

## Implementation Status
Implemented. This project loads a small external OBJ mesh, converts supported `v` and triangular `f` records into vertex/index data, uploads them into DX12 buffers, and draws the imported mesh.

## Prerequisite Sample
19. UAV And Readback

## New Concepts
- mesh file parsing
- external asset copy
- vertex/index upload
- model bounds

## Expected Result
A 1280x720 window titled `20. Model Loading` opens, clears to a neutral dark gray background, and renders a small colored diamond mesh loaded from `assets/sample_model.obj`.

## Important API Objects / Calls
- ID3D12Device::CreateCommittedResource
- ID3D12Resource::Map
- ID3D12GraphicsCommandList::IASetVertexBuffers
- ID3D12GraphicsCommandList::IASetIndexBuffer
- ID3D12GraphicsCommandList::DrawIndexedInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Model Loading.hlsl` and shader policy notes.
- `assets/`: contains `sample_model.obj` and documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\20. Model Loading\20. Model Loading.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, loads `assets/sample_model.obj`, renders the imported indexed mesh, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
