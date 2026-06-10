# 10 - Texture Sampling

## Goal

Create a procedural 2D texture entirely in JavaScript (no image files), upload it to the GPU, configure sampler parameters, and sample it in a GLSL fragment shader using a `sampler2D` uniform.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.createTexture()` | Allocates a new texture object on the GPU |
| `gl.texImage2D(target, level, internalFormat, w, h, border, format, type, pixels)` | Uploads CPU-side pixel data to the currently bound texture |
| `gl.texParameteri(target, pname, param)` | Sets sampler parameters: filter mode (`NEAREST`/`LINEAR`) and wrap mode (`REPEAT`/`CLAMP_TO_EDGE`) |
| `gl.activeTexture(gl.TEXTURE0)` | Selects which texture unit subsequent bind calls target |
| `texture(u_tex, v_uv)` in GLSL | Samples the bound 2D texture at the given UV coordinate |

## Implementation Steps

1. Build an `N×N` `Uint8Array` (RGBA) on the CPU using a checkerboard pattern: `checker = ((x>>3) ^ (y>>3)) & 1`.
2. Create the texture, bind it, call `texImage2D` to upload, then set `MIN_FILTER`, `MAG_FILTER`, and `WRAP_S/T` parameters.
3. Create a quad VAO with UV coordinates `[0,1]`, bind the texture to unit 0, set the `uniform sampler2D u_tex` to `0`, and draw.

## Expected Output

A fullscreen quad displaying a crisp 8×8 pixel checkerboard pattern (sharp because `MAG_FILTER = NEAREST`), demonstrating the full CPU-to-GPU texture upload and GLSL sampling pipeline.
