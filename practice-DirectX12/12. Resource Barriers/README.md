# 12. Resource Barriers

## Intent
Practice explicit resource state transitions for render targets, copy destinations, and shader resources.

## Implementation Status
Implemented. The sample uploads a procedural texture while it is in `COPY_DEST`, transitions it to `PIXEL_SHADER_RESOURCE`, and then samples it during rendering.

## Prerequisite Sample
11. Frame Resources

## New Concepts
- transition barrier
- render target state
- copy state
- shader resource state
- texture upload synchronization

## Expected Result
A 1280x720 window titled `12. Resource Barriers` opens, clears to a dark forest background, and renders a textured quad whose texture was transitioned from copy destination to pixel shader resource.

## Important API Objects / Calls
- ID3D12GraphicsCommandList::ResourceBarrier
- D3D12_RESOURCE_BARRIER
- D3D12_RESOURCE_STATE_COPY_DEST
- D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
- D3D12_RESOURCE_STATE_RENDER_TARGET

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Resource Barriers.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\12. Resource Barriers\12. Resource Barriers.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the barrier-uploaded texture, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
