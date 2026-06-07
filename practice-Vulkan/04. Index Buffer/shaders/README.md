# Shaders

This folder contains GLSL source and compiled SPIR-V for `04. Index Buffer`.

Compile commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\Index Buffer.vert.glsl" -o ".\shaders\Index Buffer.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\Index Buffer.frag.glsl" -o ".\shaders\Index Buffer.frag.spv"
```

The vertex shader still reads position and color attributes; the indexed draw decides which shared vertices are reused.
