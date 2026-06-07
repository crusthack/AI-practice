# 07. Texture Upload

## Intent
Upload a procedural texture and sample it through an SRV and sampler.

## Implementation Status
Implemented. The sample uploads a procedural checker texture into a default texture resource, creates an SRV, and samples it from a pixel shader.

## Prerequisite Sample
06. Descriptor Heap

## New Concepts
- texture resource
- subresource footprint
- SRV descriptor
- sampler
- texture state transition

## Expected Result
A 1280x720 window titled `07. Texture Upload` opens, clears to a desaturated blue-green background, and renders a quad textured with a procedural checker pattern.

## Important API Objects / Calls
- ID3D12Device::GetCopyableFootprints
- ID3D12GraphicsCommandList::CopyTextureRegion
- ID3D12Device::CreateShaderResourceView
- ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
- ID3D12GraphicsCommandList::ResourceBarrier

## File Map
- `main.cpp`: owns the Win32 window, DX12 device, swap chain, command queue, frame loop, resource barriers, present, and stage hook calls.
- `LearningStage.h`: owns the sample-specific stage state and render hook.
- `shaders/`: contains `Texture Upload.hlsl` and shader policy notes.
- `assets/`: documents the sample asset policy.

## Controls
Press `Esc` or close the window to exit.

## Verification
Build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\07. Texture Upload\07. Texture Upload.vcxproj" /p:Configuration=Debug /p:Platform=x64
```

Solution build command:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\DirectX12Learning.sln" /p:Configuration=Debug /p:Platform=x64
```

Run expectation: the application starts, opens one window, renders the procedural checker-textured quad, and continues presenting until closed.

## Intent Match Checklist
- [x] The project builds independently.
- [x] The visual result demonstrates the stated intent.
- [x] Every new API call listed above appears in code.
- [x] No unrelated concept is introduced as a required dependency.
