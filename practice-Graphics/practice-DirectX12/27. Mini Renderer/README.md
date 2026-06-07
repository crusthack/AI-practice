# 27. Mini Renderer

## Intent
Combine the learning samples into a small DX12 renderer with frame resources, descriptors, passes, and debug UI.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass.

## Prerequisite Sample
26. ImGui Integration

## New Concepts
- renderer structure
- frame resources
- descriptor management
- render passes
- debug UI

## Expected Result
Scaffold visual: a smoke-test window clears to a final-project dark background. Final target: interactive rendered scene.

## Important API Objects / Calls
- ID3D12GraphicsCommandList::DrawIndexedInstanced
- ID3D12GraphicsCommandList::SetDescriptorHeaps
- ID3D12GraphicsCommandList::ResourceBarrier
- ID3D12CommandQueue::ExecuteCommandLists
- IDXGISwapChain::Present

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Mini Renderer.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\27. Mini Renderer\27. Mini Renderer.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
