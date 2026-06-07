# Shaders

This folder contains the GLSL source and compiled SPIR-V for `02. HelloTriangle`.

Compile commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\HelloTriangle.vert.glsl" -o ".\shaders\HelloTriangle.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\HelloTriangle.frag.glsl" -o ".\shaders\HelloTriangle.frag.spv"
```

The `.vcxproj` copies the `.spv` files to the executable output directory under `shaders/`.
