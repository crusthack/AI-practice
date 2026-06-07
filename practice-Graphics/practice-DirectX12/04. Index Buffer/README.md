# 04. Index Buffer

## Intent
Draw shared-vertex geometry through an index buffer.

## Implementation Status
Implemented. This project creates shared rectangle vertices, uploads a 16-bit index buffer, binds both views, and records an indexed draw.

## Prerequisite Sample
03. Vertex Buffer Upload

## New Concepts
- index buffer
- index buffer view
- shared vertices
- indexed draw call

## Expected Result
A 1280x720 window titled `04. Index Buffer` opens, clears to a dark teal background, and renders a colored rectangle using six indices and four shared vertices.

## Important API Objects / Calls
- ID3D12Device::CreateCommittedResource
- ID3D12Resource::Map
- ID3D12GraphicsCommandList::IASetVertexBuffers
- ID3D12GraphicsCommandList::IASetIndexBuffer
- ID3D12GraphicsCommandList::DrawIndexedInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Index Buffer.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\04. Index Buffer\04. Index Buffer.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders an indexed colored rectangle, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
