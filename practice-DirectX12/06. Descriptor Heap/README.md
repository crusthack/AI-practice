# 06. Descriptor Heap

## Intent
Create shader-visible descriptor heaps and bind CBV descriptors through descriptor tables.

## Implementation Status
Implemented. The sample creates a shader-visible CBV descriptor heap, binds it through a descriptor table, and renders geometry using data read from the constant buffer.

## Prerequisite Sample
05. Root Signature And Constants

## New Concepts
- CPU descriptor handle
- GPU descriptor handle
- shader-visible heap
- descriptor table
- descriptor increment size

## Expected Result
A 1280x720 window titled `06. Descriptor Heap` opens, clears to a dark slate background, and renders a triangle whose offset and tint come from a CBV descriptor table.

## Important API Objects / Calls
- ID3D12Device::CreateDescriptorHeap
- ID3D12Device::CreateConstantBufferView
- ID3D12GraphicsCommandList::SetDescriptorHeaps
- ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
- ID3D12GraphicsCommandList::DrawInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Descriptor Heap.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\06. Descriptor Heap\06. Descriptor Heap.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the descriptor-table-driven triangle, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
