# Shaders

This folder contains GLSL source and compiled SPIR-V for `05. Push Constants And Uniforms`.

Compile commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\Push Constants And Uniforms.vert.glsl" -o ".\shaders\Push Constants And Uniforms.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\Push Constants And Uniforms.frag.glsl" -o ".\shaders\Push Constants And Uniforms.frag.spv"
```

The vertex shader reads a uniform buffer at binding 0. The fragment shader reads a push constant block.
