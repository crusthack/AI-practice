# 13. Upload And Default Heaps

## Intent
Separate CPU-visible upload resources from GPU-local default resources and copy between them.

## Implementation Status
Implemented. The sample writes vertex and index data into upload heap buffers, copies them into GPU-local default heap buffers, and renders only from the default heap resources.

## Prerequisite Sample
12. Resource Barriers

## New Concepts
- upload heap
- default heap
- intermediate resource
- copy queue concept
- GPU-local resource

## Expected Result
A 1280x720 window titled `13. Upload And Default Heaps` opens, clears to a dim olive background, and renders a colored indexed rectangle from default heap vertex and index buffers.

## Important API Objects / Calls
- ID3D12Device::CreateCommittedResource
- D3D12_HEAP_TYPE_UPLOAD
- D3D12_HEAP_TYPE_DEFAULT
- ID3D12GraphicsCommandList::CopyResource
- D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
- D3D12_RESOURCE_STATE_INDEX_BUFFER

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Upload And Default Heaps.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\13. Upload And Default Heaps\13. Upload And Default Heaps.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the copied default-heap rectangle, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
