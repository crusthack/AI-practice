# Shaders

This folder contains GLSL source and compiled SPIR-V for `03. Vertex Buffer Upload`.

Compile commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\Vertex Buffer Upload.vert.glsl" -o ".\shaders\Vertex Buffer Upload.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\Vertex Buffer Upload.frag.glsl" -o ".\shaders\Vertex Buffer Upload.frag.spv"
```

The vertex shader reads position and color from vertex buffer attributes at locations 0 and 1.
