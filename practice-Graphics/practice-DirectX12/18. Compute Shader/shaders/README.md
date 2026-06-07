# Shaders

Policy: external HLSL source copied to the output directory.

`Compute Shader.hlsl` contains:

- `ComputeMain`: writes an animated procedural pattern into a UAV texture.
- `ScreenVS` / `ScreenPS`: samples the compute output through an SRV and draws it to the swap-chain back buffer.
