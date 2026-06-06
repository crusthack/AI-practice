# 21. Scene Graph

## Intent
Render multiple objects with separate transforms, materials, and draw calls.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass.

## Prerequisite Sample
20. Model Loading

## New Concepts
- scene object list
- per-object constants
- material table
- multiple draw calls

## Expected Result
Scaffold visual: a smoke-test window clears to a dark magenta-gray background. Final target: multi-object scene.

## Important API Objects / Calls
- ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
- ID3D12GraphicsCommandList::IASetVertexBuffers
- ID3D12GraphicsCommandList::IASetIndexBuffer
- ID3D12GraphicsCommandList::DrawIndexedInstanced

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Scene Graph.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\21. Scene Graph\21. Scene Graph.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
