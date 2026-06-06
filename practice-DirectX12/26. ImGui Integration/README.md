# 26. ImGui Integration

## Intent
Integrate Dear ImGui as a debug UI overlay for DX12 samples.

## Implementation Status
Scaffold. This project currently provides the required folder/project structure and a DX12 clear/present smoke test; the lesson-specific rendering work is intentionally left for the focused implementation pass.

## Prerequisite Sample
25. Post Processing

## New Concepts
- ImGui context
- descriptor heap for UI
- frame UI build
- overlay rendering

## Expected Result
Scaffold visual: a smoke-test window clears to a dark blue-gray background. Final target: DX12 scene with ImGui overlay.

## Important API Objects / Calls
- ImGui::CreateContext
- ImGui_ImplWin32_Init
- ImGui_ImplDX12_Init
- ImGui_ImplDX12_RenderDrawData

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `ImGui Integration.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\26. ImGui Integration\26. ImGui Integration.vcxproj" /p:Configuration=Debug /p:Platform=x64
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
