# Shaders

Policy: external HLSL compute shader copied to the output directory.

`UAV And Readback.hlsl` contains `ComputeMain`, which writes one animated color value into an `RWStructuredBuffer`. C++ copies that UAV buffer into a readback resource and maps it on the CPU.
