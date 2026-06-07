# Shaders

This folder contains GLSL source and compiled SPIR-V for `06. Descriptor Sets`.

Compile commands:

```powershell
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=vert ".\shaders\Descriptor Sets.vert.glsl" -o ".\shaders\Descriptor Sets.vert.spv"
& "$env:VULKAN_SDK\Bin\glslc.exe" -fshader-stage=frag ".\shaders\Descriptor Sets.frag.glsl" -o ".\shaders\Descriptor Sets.frag.spv"
```

Binding 0 is a vertex-stage uniform buffer for transform offset. Binding 1 is a fragment-stage uniform buffer for color tint.
