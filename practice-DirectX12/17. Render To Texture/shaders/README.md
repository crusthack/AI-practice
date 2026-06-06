# Shaders

Policy: external HLSL source copied to the output directory.

`Render To Texture.hlsl` contains two small passes:

- `OffscreenVS` / `OffscreenPS`: draw a procedural colored triangle into the offscreen render target.
- `ScreenVS` / `ScreenPS`: sample the offscreen texture SRV and draw it to the swap-chain back buffer.
