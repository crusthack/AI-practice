# 10. Camera

## Intent
Move a view camera with keyboard and mouse input.

## Implementation Status
Implemented. The sample renders a small 3D cube scene and updates the view matrix from keyboard movement and right-mouse camera look.

## Prerequisite Sample
09. Transform Matrices

## New Concepts
- camera position
- camera basis vectors
- view matrix
- keyboard input
- mouse look

## Expected Result
A 1280x720 window titled `10. Camera` opens, clears to a dark navy background, and renders three colored cubes that can be inspected by moving the camera.

## Important API Objects / Calls
- GetAsyncKeyState
- GetCursorPos
- DirectX::XMMatrixLookToLH
- DirectX::XMVector3Normalize
- ID3D12Device::CreateConstantBufferView

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Camera.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
WASD moves horizontally, Q/E moves down/up, Shift increases movement speed, hold the right mouse button and move the mouse to look around, and Esc exits.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\10. Camera\10. Camera.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders a camera-controlled cube scene, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
