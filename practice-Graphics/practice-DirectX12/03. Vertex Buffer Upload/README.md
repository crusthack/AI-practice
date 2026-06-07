# 03. Vertex Buffer Upload

## Intent
Upload vertex data into GPU resources and bind a vertex buffer view.

## Implementation Status
Implemented. This project uploads triangle vertices through an upload heap, copies them into a default-heap vertex buffer, transitions that buffer for vertex input, and draws from the resulting vertex buffer view.

## Prerequisite Sample
02. HelloTriangle

## New Concepts
- upload heap
- default heap
- vertex buffer resource
- vertex buffer view
- copy command

## Expected Result
A 1280x720 window titled `03. Vertex Buffer Upload` opens, clears to a muted indigo background, and renders one colored triangle from a default-heap vertex buffer.

## Important API Objects / Calls
- ID3D12Device::CreateCommittedResource
- ID3D12Resource::Map
- ID3D12GraphicsCommandList::CopyBufferRegion
- ID3D12GraphicsCommandList::ResourceBarrier
- ID3D12GraphicsCommandList::IASetVertexBuffers
- ID3D12GraphicsCommandList::DrawInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Vertex Buffer Upload.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\03. Vertex Buffer Upload\03. Vertex Buffer Upload.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders a colored triangle from a GPU-local vertex buffer, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
